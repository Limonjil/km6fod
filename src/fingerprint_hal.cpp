// SPDX-License-Identifier: GPL-3.0-only

#include "fingerprint_hal.h"

#include "sensor_geometry.h"

#include <sys/stat.h>
#include <sys/types.h>

#include <cerrno>
#include <cstring>
#include <string>

#include "hbm.h"
#include "logging.h"
#include "stability.h"
#include "vendor_engine.h"

namespace km6fod {
namespace {

namespace fp = aidl::android::hardware::biometrics::fingerprint;
namespace bc = aidl::android::hardware::biometrics::common;

// Calibrated optical sensor window on the KM6 panel (1080x2400).

// How long engine notices from a superseded operation are discarded after a
// session swap. Long enough to absorb the vendor cancel path, short enough not
// to swallow the first events of a new capture.
constexpr int kSessionMuteMs = 300;

}  // namespace

std::atomic<FingerprintHal*> FingerprintHal::g_instance{nullptr};

FingerprintHal::~FingerprintHal() {
    stop();
}

void FingerprintHal::start() {
    if (started_.exchange(true)) {
        return;
    }
    g_instance.store(this, std::memory_order_release);

    timers_.start();
    dispatcher_.start();
    worker_.start();
    HbmController::instance().start();

    // The sensor itself stays closed: the spec forbids opening it at startup.
    // The callback is stored now and installed by VendorEngine::open(), so no
    // early engine event can be lost when the first operation opens the device.
    VendorEngine::instance().setNotify(&FingerprintHal::engineNotifyTrampoline);

    // Reading the digitizer feeds coordinates to the sensor and also satisfies
    // detectInteraction(); it is independent of any single session.
    if (!touch_.start([this](int x, int y, bool down) { onTouch(x, y, down); },
                      [this](bool down) { onFodKey(down); })) {
        LOGW("hal: touch reader unavailable; relying on framework pointer events");
    }

    LOGI("hal: started");
}

void FingerprintHal::stop() {
    if (!started_.exchange(false)) {
        return;
    }
    LOGI("hal: stopping");

    // Stop new engine notifications first, then the touch producer. Existing
    // dispatcher tasks hold shared_ptr<Session> and observe closed_.
    g_instance.store(nullptr, std::memory_order_release);
    touch_.stop();

    std::shared_ptr<Session> session;
    {
        std::lock_guard<std::mutex> lock(session_mutex_);
        session = session_;
        session_.reset();
    }
    if (session) {
        session->shutdown();
    }

    // Drain in order: the dispatcher first, because its queued callbacks may
    // still hand work to the worker; then the timer queue; then the worker,
    // which is what actually drains in-flight vendor calls. Closing the device
    // before the worker drained would let a queued remove/enumerate run on a
    // freed vendor object, and stopping the worker first dropped the callbacks
    // the framework was still waiting for (review H-31).
    dispatcher_.stop();
    timers_.stop();
    worker_.stop();

    HbmController::instance().requestOff();
    HbmController::instance().stop();
    VendorEngine::instance().close();

    LOGI("hal: stopped");
}

void FingerprintHal::engineNotifyTrampoline(const EngineMessage* message) {
    auto* instance = g_instance.load(std::memory_order_acquire);
    if (instance == nullptr || message == nullptr) {
        return;
    }
    instance->dispatchEngineMessage(*message);
}

void FingerprintHal::dispatchEngineMessage(const EngineMessage& message) {
    auto* instance = g_instance.load(std::memory_order_acquire);
    if (instance == nullptr) {
        return;
    }

    std::shared_ptr<Session> session;
    {
        std::lock_guard<std::mutex> lock(instance->session_mutex_);
        // Acquired notices are exempt: they carry no result, and the new operation's
        // first frame was being thrown away with them (review H-19).
        if (message.type != kEngineMsgAcquired &&
            std::chrono::steady_clock::now() < instance->engine_mute_until_) {
            LOGD("hal: engine message %d muted during session swap", message.type);
            return;
        }
        session = instance->session_;
    }
    if (!session) {
        LOGD("hal: engine message %d dropped, no session", message.type);
        return;
    }

    // Copy by value: the engine owns the original buffer and may reuse it as
    // soon as the callback returns.
    const EngineMessage copy = message;
    if (!instance->dispatcher_.post([session, copy] { session->onEngineMessage(copy); })) {
        LOGD("hal: dispatcher stopped, engine message %d dropped", message.type);
    }
}

void FingerprintHal::onFodKey(bool down) {
    std::shared_ptr<Session> session;
    {
        std::lock_guard<std::mutex> lock(session_mutex_);
        session = session_;
    }
    if (!session) {
        return;
    }
    session->onFodKey(down);
}

void FingerprintHal::onTouch(int x, int y, bool down) {
    std::shared_ptr<Session> session;
    {
        std::lock_guard<std::mutex> lock(session_mutex_);
        session = session_;
    }
    if (!session) {
        return;
    }
    // The touch reader is already our own thread; do not add 100+ dispatcher
    // tasks per second. Session::onTouchEvent is thread-safe and only posts the
    // rare interaction-detected callback.
    session->onTouchEvent(x, y, down);
}

::ndk::ScopedAStatus FingerprintHal::getSensorProps(std::vector<fp::SensorProps>* out) {
    if (out == nullptr) {
        return ::ndk::ScopedAStatus::ok();
    }

    fp::SensorProps props{};

    props.commonProps.sensorId = 0;
    props.commonProps.sensorStrength = bc::SensorStrength::STRONG;
    props.commonProps.maxEnrollmentsPerUser = 5;
    bc::ComponentInfo component{};
    component.componentId = "silead";
    component.hardwareVersion = "1";
    component.firmwareVersion = "1";
    component.serialNumber = "0";
    component.softwareVersion = "1";
    props.commonProps.componentInfo.push_back(std::move(component));

    props.sensorType = fp::FingerprintSensorType::UNDER_DISPLAY_OPTICAL;

    // An empty display name means "the built-in display". Using "local:0" here
    // makes the framework fail to resolve the display, collapse udfpsBounds to
    // an empty rect and drop every touch.
    fp::SensorLocation location{};
    location.display = "";
    // Shared with Session::inSensorZone, so the area the framework highlights and
    // the area the HAL accepts are the same one (review H-14, H-42).
    location.sensorLocationX = km6fod::kSensorCenterX;
    location.sensorLocationY = km6fod::kSensorCenterY;
    location.sensorRadius = km6fod::kSensorRadiusPx;
    location.sensorShape = fp::SensorShape::CIRCLE;
    props.sensorLocations.push_back(location);

    // The framework owns the visible illumination. An optical sensor needs light
    // EMITTED by the panel: HBM alone only scales backlight, and on a black
    // screen there is nothing to scale. The bright circle SystemUI paints is the
    // actual light source, so this must stay false - setting it true removed the
    // glow and with it the light the sensor needs.
    props.halControlsIllumination = false;
    props.halHandlesDisplayTouches = false;
    props.supportsDetectInteraction = true;
    props.supportsNavigationGestures = false;
    props.touchDetectionParameters = std::nullopt;

    out->clear();
    out->push_back(props);

    LOGI("hal: getSensorProps x=%d y=%d r=%d", km6fod::kSensorCenterX,
         km6fod::kSensorCenterY, km6fod::kSensorRadiusPx);
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus FingerprintHal::createSession(
    int32_t in_sensorId, int32_t in_userId, const std::shared_ptr<fp::ISessionCallback>& in_cb,
    std::shared_ptr<fp::ISession>* out) {
    if (out == nullptr) {
        return ::ndk::ScopedAStatus::ok();
    }
    LOGI("hal: createSession sensor=%d user=%d", in_sensorId, in_userId);
    if (in_sensorId != 0) {
        LOGW("hal: unexpected sensorId %d, using 0", in_sensorId);
    }

    auto session = ::ndk::SharedRefBase::make<Session>(in_cb, in_userId, dispatcher_, worker_,
                                                       timers_);
    session->attachSelf(session);
    markStable(session->asBinder().get(), "ISession");

    std::shared_ptr<Session> old;
    {
        std::lock_guard<std::mutex> lock(session_mutex_);
        old = session_;
        session_ = session;
        if (old && old->hasActiveOperation()) {
            engine_mute_until_ = std::chrono::steady_clock::now() +
                                 std::chrono::milliseconds(kSessionMuteMs);
        }
    }
    if (old) {
        // A new session supersedes the old one; make sure it cannot keep the
        // panel illuminated or leave an engine capture running.
        old->shutdown();
    }

    // The per-user template group is applied lazily in Session::armCapture(),
    // after the sensor is opened on the SerialWorker. createSession() never
    // touches the vendor device.

    *out = session;
    return ::ndk::ScopedAStatus::ok();
}

}  // namespace km6fod
