// SPDX-License-Identifier: GPL-2.0-only

#include "session.h"

#include "paths.h"
#include "sensor_geometry.h"

#include <android/binder_auto_utils.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <csignal>
#include <mutex>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <utility>

#include <aidl/android/hardware/biometrics/common/ICancellationSignal.h>
#include <aidl/android/hardware/biometrics/fingerprint/ISessionCallback.h>

#include "auth_token.h"
#include "hbm.h"
#include "logging.h"
#include "stability.h"
#include "vendor_engine.h"

extern char **environ;

namespace km6fod {
namespace {

namespace fp = aidl::android::hardware::biometrics::fingerprint;
namespace bc = aidl::android::hardware::biometrics::common;
namespace km = aidl::android::hardware::keymaster;

constexpr int32_t kSensorCenterX = 560;
constexpr int32_t kSensorCenterY = 2205;
constexpr int32_t kTouchTolerancePx = 250;

// Geometry lives in sensor_geometry.h, shared with the HAL's SensorProps.

int64_t monotonicMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

// Vendor engine error codes follow the 0-based order used by the AIDL Error
// enum. This was verified against libsl_fp_impl.so: silfp_common_cancel() sends
// 5, which is CANCELED in this order (the HIDL 2.1 enum would make it NO_SPACE).
// One path sends 1000, a vendor sentinel; it maps to VENDOR.
fp::Error mapEngineError(int32_t engine_code) {
    switch (engine_code) {
        case 0:
            return fp::Error::UNKNOWN;
        case 1:
            return fp::Error::HW_UNAVAILABLE;
        case 2:
            return fp::Error::UNABLE_TO_PROCESS;
        case 3:
            return fp::Error::TIMEOUT;
        case 4:
            return fp::Error::NO_SPACE;
        case 5:
            return fp::Error::CANCELED;
        case 6:
            return fp::Error::UNABLE_TO_REMOVE;
        default:
            return fp::Error::VENDOR;
    }
}

// AcquiredInfo was renumbered for AIDL: the vendor still speaks the HIDL
// enumeration (GOOD = 0) while the stable interface starts at 1.
fp::AcquiredInfo mapEngineAcquired(int32_t engine_code) {
    switch (engine_code) {
        case 0:
            return fp::AcquiredInfo::GOOD;
        case 1:
            return fp::AcquiredInfo::PARTIAL;
        case 2:
            return fp::AcquiredInfo::INSUFFICIENT;
        case 3:
            return fp::AcquiredInfo::SENSOR_DIRTY;
        case 4:
            return fp::AcquiredInfo::TOO_SLOW;
        case 5:
            return fp::AcquiredInfo::TOO_FAST;
        case 6:
            return fp::AcquiredInfo::IMMOBILE;
        default:
            return fp::AcquiredInfo::VENDOR;
    }
}

// The GSI ships no UdfpsHbmProvider, so SystemUI never calls
// UdfpsDisplayMode.enable() and the panel stays in doze when the icon is touched
// on the lock screen. The panel driver then silently drops the HBM write
// ("display state is slept or idle, skip set hbm node") and the sensor gets no
// light. Waking the display is the only way to let HBM take effect.
//
// The virtual input device is created once and kept for the process lifetime.
// Creating it on demand and writing 50 ms later works only on an idle system: while
// a touch stream is running the InputReader needs noticeably longer to register
// the new device, and the key event is silently dropped - which is exactly why the
// same code worked from a standalone tool and not from here.


// Duplicate gesture events arrive within a few milliseconds of each other; a real
// finger a second later must never be mistaken for one. A genuine touch on the
// sensor bypasses it entirely.
constexpr int64_t kWakeDebounceMs = 120;
std::atomic<int64_t> g_last_wake_ms{0};

// Puts the controller back into gesture mode. The touch driver keeps its
// per-gesture enable bit only while it is powered; after a suspend cycle that
// bit can end up cleared, and the gesture is then recognised and thrown away
// ("Sorry, this gesture has been disabled"). Re-applying it when the display
// goes to sleep is cheap and keeps every wake armed.
void rearmGestureMode() {
    // The working path is the procfs node: the vendor service maps its ITrantp
    // command to /proc/nvt_selftest, which the kernel rejects ("Invalid value!
    // count = 3"), while /proc/gesture_function really reaches the driver
    // (dmesg: gesture_data_write code1[be] enable[1]). A plain write, no process.
    const int fd = ::open("/proc/gesture_function", O_WRONLY | O_CLOEXEC);
    if (fd < 0) {
        return;
    }
    const ssize_t written = ::write(fd, "fp1\n", 4);
    ::close(fd);
    if (written == 4) {
        LOGD("wake: gesture re-armed via /proc/gesture_function");
    }
}

// The framework opens the authenticate operation about 1.3 s after the display
// comes up (measured: pre-light 25.124 -> authenticate 26.402). Illumination must
// therefore survive that gap instead of closing as soon as the operation is still
// missing, which is what left every dark-screen press with a 0.2 s flash and no
// unlock (DEBUG.md F27).
bool wakeInFlight() {
    const int64_t t = g_last_wake_ms.load(std::memory_order_acquire);
    return t != 0 && monotonicMs() - t < 2500;
}

// A real wake has to go through the platform's own wake path: only then does
// SystemUI draw the keyguard and its fingerprint overlay, and only then does the
// framework create the authenticate operation the capture needs. A display
// wakelock lights the panel but leaves SystemUI in its dozing bookkeeping, so the
// operation never arrives (measured: 12 wakes, "overlay did not appear", state=0).
void injectWakeKey() {
    static std::once_flag once;
    std::call_once(once, [] { ::signal(SIGCHLD, SIG_IGN); });
    const pid_t pid = ::fork();
    if (pid == 0) {
        ::execl("/system/bin/input", "input", "keyevent", "224", static_cast<char*>(nullptr));
        ::_exit(127);
    }
    if (pid < 0) {
        // The wake silently did not happen; without this the stall looked like a
        // dead sensor (review H-34).
        LOGE("session: cannot inject the wake key: fork failed (%s)", std::strerror(errno));
    }
}

// 100 ms cache: the wake decision does not need better resolution, and the reader
// calls it on every idle sample.
int cachedBacklightLevel() {
    static std::atomic<int64_t> cached_at{0};
    static std::atomic<int> cached{0};
    const int64_t now = monotonicMs();
    const int64_t at = cached_at.load(std::memory_order_acquire);
    if (at != 0 && now - at < 100) {
        return cached.load(std::memory_order_acquire);
    }
    const int level = panelBacklightLevel();
    cached.store(level, std::memory_order_release);
    cached_at.store(now, std::memory_order_release);
    return level;
}

void wakeDisplay(bool force = false) {
    const int64_t now = monotonicMs();
    const int64_t last = g_last_wake_ms.load(std::memory_order_acquire);
    // Once per press. This used to run on every touch sample: twelve kernel wakes
    // inside fifty milliseconds, all of them useless.
    if (!force && last != 0 && now - last < kWakeDebounceMs) {
        return;
    }
    // Two wake paths, both needed: the injected KEYCODE_WAKEUP is what makes
    // SystemUI treat the screen as interactive and create the authenticate
    // operation, and the vendor's kernel wakelock (tran_fp -> do_wakeup_set) brings
    // the panel up when the display was really asleep.
    // Cached: this used to open and read the sysfs node on every idle touch sample,
    // up to a hundred times per second on the reader (review H-15).
    if (cachedBacklightLevel() > 0) {
        return;
    }
    g_last_wake_ms.store(now, std::memory_order_release);
    injectWakeKey();
    VendorEngine::instance().wakeSystem();
}

}  // namespace

std::atomic<int32_t> Session::applied_user_id_{-1};

bool Session::postToWorker(std::function<void()> action) {
    if (worker_.post(std::move(action))) {
        return true;
    }
    LOGE("session: worker rejected a task, the vendor call is lost");
    return false;
}

bool Session::postToUi(std::function<void()> task) {
    // The dispatcher is what actually queues the task; postToUi only wraps it and
    // logs the failure (a global rename once turned this line into a self-call,
    // which crashed the HAL with a stack overflow).
    if (dispatcher_.post(std::move(task))) {
        return true;
    }
    LOGE("session: dispatcher rejected a callback, it will never be delivered");
    return false;
}

void Session::releaseFingerOnEngine() {
    // A vendor call, so it runs on the serial worker: the callers are the callback,
    // timer and touch threads, and none of them may block on the vendor
    // (review H-03/H-04). A finger flag left behind makes the engine wait for a
    // frame from a finger that is gone - its err 1030 (review H-07).
    if (!worker_.post([] { VendorEngine::instance().syncFingerStatusOptic(false); })) {
        LOGE("session: worker rejected the finger-release sync");
    }
}

bool Session::clearFingerState() {
    if (!finger_down_.exchange(false, std::memory_order_acq_rel)) {
        return false;
    }
    releaseFingerOnEngine();
    return true;
}

bool Session::uiReadyValid() const {
    return monotonicMs() < ui_ready_until_ms_.load(std::memory_order_acquire);
}

void Session::markUiReady() {
    ui_ready_until_ms_.store(monotonicMs() + kUiReadyWindowMs, std::memory_order_release);
}

void Session::clearUiReady() {
    ui_ready_until_ms_.store(0, std::memory_order_release);
}

// ------------------------------------------------------------- cancellation

class Session::Cancellation : public bc::BnCancellationSignal {
  public:
    Cancellation(std::weak_ptr<Session> self, uint64_t generation)
        : self_(std::move(self)), generation_(generation) {}

    ::ndk::ScopedAStatus cancel() override {
        bool expected = false;
        if (fired_.compare_exchange_strong(expected, true)) {
            if (auto self = self_.lock()) {
                self->cancelOperation(generation_);
            }
        }
        return ::ndk::ScopedAStatus::ok();
    }

  private:
    std::weak_ptr<Session> self_;
    const uint64_t generation_;
    std::atomic<bool> fired_{false};
};

// ------------------------------------------------------------------ session

Session::Session(std::shared_ptr<fp::ISessionCallback> callback, int32_t userId,
                 CallbackDispatcher& dispatcher, SerialWorker& worker,
                 DeadlineScheduler& timers)
    : callback_(std::move(callback)),
      dispatcher_(dispatcher),
      worker_(worker),
      timers_(timers),
      user_id_(userId) {
    LOGI("session: created for user %d", userId);
}

Session::~Session() {
    // The HAL calls shutdown() before dropping its reference. This is only a
    // last-resort safety net; it must not block.
    closed_.store(true, std::memory_order_release);
    HbmController::instance().requestOff();
    LOGD("session: destroyed user=%d", user_id_);
}

void Session::attachSelf(const std::shared_ptr<Session>& self) {
    self_ = self;
}

// --------------------------------------------------------------- internals

bool Session::isCurrent(uint64_t generation, State state) const {
    return !closed_.load(std::memory_order_acquire) &&
           generation_.load(std::memory_order_acquire) == generation &&
           state_.load(std::memory_order_acquire) == state;
}

uint64_t Session::beginOperation(State next_state) {
    uint64_t generation = 0;
    bool had_active = false;
    {
        std::lock_guard<std::mutex> lock(op_mutex_);
        if (closed_.load(std::memory_order_acquire)) {
            return 0;
        }
        had_active = state_.load(std::memory_order_acquire) != State::kIdle;
        generation = ++generation_;
        state_.store(next_state, std::memory_order_release);
        op_started_ms_.store(monotonicMs(), std::memory_order_release);
        ++cancel_epoch_;
        frame_good_.store(false, std::memory_order_release);
        if (timeout_id_ != 0) {
            timers_.cancel(timeout_id_);
            timeout_id_ = 0;
        }
        const std::weak_ptr<Session> weak = self_;
        timeout_id_ = timers_.scheduleAfter(
            std::chrono::milliseconds(kOperationTimeoutMs), [weak, generation] {
                if (auto self = weak.lock()) {
                    self->timeoutOperation(generation);
                }
            });
        if (timeout_id_ == 0) {
            // The deadline queue was full: the operation would wait forever and the
            // framework with it. Fail it now instead of leaking it (review H-47).
            LOGE("session: no deadline slot, failing the operation immediately");
            postTerminalError(fp::Error::UNABLE_TO_PROCESS, 0,
                              cancel_epoch_.load(std::memory_order_acquire));
        }
    }

    if (had_active) {
        LOGW("session: new operation supersedes an active one");
        // Binder thread: the vendor call belongs on the serial worker (review H-03).
        postToWorker([] { VendorEngine::instance().cancel(); });
        HbmController::instance().requestOff();
        clearUiReady();
    }
    return generation;
}

bool Session::finishOperation(uint64_t generation, bool close_hbm) {
    bool finger_was_down = false;
    {
        std::lock_guard<std::mutex> lock(op_mutex_);
        if (generation != generation_.load(std::memory_order_acquire) ||
            state_.load(std::memory_order_acquire) == State::kIdle) {
            return false;
        }
        state_.store(State::kIdle, std::memory_order_release);
        // The flag is taken here, the vendor is told after the mutex is released:
        // holding it across a vendor call stalls every Binder thread that wants an
        // operation (review H-13).
        finger_was_down = finger_down_.exchange(false, std::memory_order_acq_rel);
        if (finger_was_down) {
            finger_up_ms_.store(monotonicMs(), std::memory_order_release);
        }
        // The overlay is gone once the operation ends; the next one has to prove
        // itself again with a fresh onUiReady().
        clearUiReady();
        ++generation_;
        ++cancel_epoch_;
        if (timeout_id_ != 0) {
            timers_.cancel(timeout_id_);
            timeout_id_ = 0;
        }
    }
    if (finger_was_down) {
        releaseFingerOnEngine();
    }
    if (close_hbm) {
        HbmController::instance().requestOff();
    }
    return true;
}

void Session::failOperation(uint64_t generation, fp::Error error, int32_t vendor_code) {
    if (!finishOperation(generation, /*close_hbm=*/true)) {
        return;
    }
    VendorEngine::instance().cancel();
    // Deliver terminal errors asynchronously. failOperation can be reached from
    // the SerialWorker; calling Binder from there would block vendor work.
    const uint64_t epoch = cancel_epoch_.load(std::memory_order_acquire);
    postTerminalError(error, vendor_code, epoch);
}

void Session::postTerminalError(fp::Error error, int32_t vendor_code, uint64_t epoch) {
    const std::weak_ptr<Session> weak = self_;
    postToUi([weak, error, vendor_code, epoch] {
        auto self = weak.lock();
        if (!self || self->closed_.load(std::memory_order_acquire)) {
            return;
        }
        // A new operation or a newer terminal event invalidates this one.
        if (self->cancel_epoch_.load(std::memory_order_acquire) != epoch) {
            return;
        }
        self->emitError(error, vendor_code);
    });
}

void Session::timeoutOperation(uint64_t generation) {
    {
        std::lock_guard<std::mutex> lock(op_mutex_);
        if (generation != generation_.load(std::memory_order_acquire) ||
            state_.load(std::memory_order_acquire) == State::kIdle) {
            return;
        }
        state_.store(State::kIdle, std::memory_order_release);
        ++generation_;
        ++cancel_epoch_;
        timeout_id_ = 0;
    }
    // The operation is over: a finger flag left behind would swallow the first
    // touch of the next one, and a captured-frame flag would turn an engine
    // hiccup into a failed attempt later.
    clearFingerState();
    const bool captured = frame_good_.exchange(false, std::memory_order_acq_rel);
    LOGW("session: operation timed out (captured_frame=%d)", captured ? 1 : 0);
    // This runs on the timer thread: a vendor call here would stop every other
    // deadline behind it (review H-04).
    HbmController::instance().requestOff();
    postToWorker([] { VendorEngine::instance().cancel(); });
    const uint64_t epoch = cancel_epoch_.load(std::memory_order_acquire);
    // A GOOD frame without a comparison means the engine failed, not that the
    // user waited: say so instead of a generic timeout every 60 seconds.
    postTerminalError(captured ? fp::Error::UNABLE_TO_PROCESS : fp::Error::TIMEOUT, 0, epoch);
}

void Session::cancelOperation(uint64_t generation) {
    if (closed_.load(std::memory_order_acquire)) {
        return;
    }

    uint64_t cancel_epoch = 0;
    {
        std::lock_guard<std::mutex> lock(op_mutex_);
        // A cancellation from a superseded operation must not kill the current
        // one. Compare the operation identity, not just "there is an operation".
        if (generation != generation_.load(std::memory_order_acquire) ||
            state_.load(std::memory_order_acquire) == State::kIdle) {
            LOGD("session: stale cancel ignored");
            return;
        }
        state_.store(State::kIdle, std::memory_order_release);
        ++generation_;
        cancel_epoch = ++cancel_epoch_;
        if (timeout_id_ != 0) {
            timers_.cancel(timeout_id_);
            timeout_id_ = 0;
        }
    }

    clearFingerState();
    frame_good_.store(false, std::memory_order_release);
    LOGI("session: cancel requested");
    HbmController::instance().requestOff();
    postToWorker([] { VendorEngine::instance().cancel(); });
    // Async, not inline: this runs on the Binder thread that called
    // ICancellationSignal.cancel(). Delivering onError() synchronously from
    // inside that call can re-enter the framework while it holds its own lock.
    postTerminalError(fp::Error::CANCELED, 0, cancel_epoch);

    // Sensor watchdog from the spec: if the engine ignored the first cancel,
    // repeat it after ~3 s. A new operation invalidates this epoch.
    const std::weak_ptr<Session> weak = self_;
    timers_.scheduleAfter(std::chrono::milliseconds(kCancelEscalationMs), [weak, cancel_epoch] {
        auto self = weak.lock();
        if (!self || self->closed_.load(std::memory_order_acquire)) {
            return;
        }
        if (self->cancel_epoch_.load(std::memory_order_acquire) != cancel_epoch) {
            return;
        }
        LOGW("session: cancel escalation");
        // Timer thread: never call the vendor here (review H-04).
        self->postToWorker([] { VendorEngine::instance().cancel(); });
        HbmController::instance().requestOff();
    });
}

bool Session::ensureUserGroup() {
    // Once per session guard on top of the process-wide group below: re-applying
    // the group invalidates the challenge that pre_enroll() just handed out.
    bool expected = false;
    if (!group_applied_.compare_exchange_strong(expected, true)) {
        return true;
    }

    VendorEngine& engine = VendorEngine::instance();
    if (!engine.isOpen()) {
        group_applied_.store(false);
        return false;
    }

    // Already active for this process: nothing to do, and no calibration wait.
    if (applied_user_id_.load(std::memory_order_acquire) == user_id_) {
        return true;
    }

    const std::string path = VendorEngine::storePathForUser(user_id_);
    ::mkdir("/data/vendor_de", 0771);
    const std::string base = std::string("/data/vendor_de/") + std::to_string(user_id_);
    ::mkdir(base.c_str(), 0771);
    if (::mkdir(path.c_str(), 0700) != 0 && errno != EEXIST) {
        LOGW("session: mkdir(%s) failed: %s", path.c_str(), std::strerror(errno));
    }
    const int rc = engine.setActiveGroup(static_cast<uint32_t>(user_id_), path.c_str());
    LOGI("session: set_active_group(%d, %s) rc=%d", user_id_, path.c_str(), rc);
    if (rc == 0) {
        applied_user_id_.store(user_id_, std::memory_order_release);
        group_just_applied_.store(true, std::memory_order_release);
        return true;
    }
    if (user_id_ == 0) {
        // open() already applied the default group for user 0.
        LOGW("session: user 0 group switch failed, keeping the default group");
        applied_user_id_.store(user_id_, std::memory_order_release);
        group_just_applied_.store(true, std::memory_order_release);
        return true;
    }
    group_applied_.store(false);
    return false;
}

bool Session::armIllumination(bool allow_wake) {
    // The overlay can only be drawn on a live display, so a valid ready window is
    // proof that the panel is up. OperationContext::displayState is not usable as
    // that proof: it is frequently stale, and waiting on it made every dark-screen
    // press spend the whole settle budget for nothing.
    if (allow_wake && !uiReadyValid()) {
        wakeDisplay();
    }

    // No off/on "reset" here. It closed the window and reopened it on every
    // re-arm, which the user saw as flicker, and it trusted a software flag
    // instead of the panel. requestOn() now re-checks the node itself.
    bool woke_display = !allow_wake;
    for (int attempt = 0; attempt < kHbmArmAttempts; ++attempt) {
        // Abort the moment the reason to be bright is gone. Without this the
        // retry loop kept driving the panel for hundreds of milliseconds after
        // the finger had left: the screen lit up on its own, and the sensor
        // finished a capture on whatever finger happened to be there by then -
        // which is what looked like an unlock by a different finger.
        const State now_state = state_.load(std::memory_order_acquire);
        if (now_state != State::kAuthenticating && now_state != State::kEnrolling) {
            LOGD("session: illumination aborted, no operation");
            return false;
        }
        if (!finger_down_.load(std::memory_order_acquire)) {
            LOGD("session: illumination aborted, finger is up");
            return false;
        }
        if (HbmController::instance().requestOn()) {
            // The ONE place that declares the UI ready, and only after the panel
            // was verified bright.
            VendorEngine::instance().notifyUiReady(1);
            return true;
        }
        if (!woke_display && attempt >= 2) {
            woke_display = true;
            wakeDisplay();
        }
        if (attempt + 1 < kHbmArmAttempts) {
            ::usleep(kHbmArmRetryUs);
        }
    }
    LOGE("session: illumination unavailable after %d attempts", kHbmArmAttempts);
    return false;
}

bool Session::illuminationQuarantined() const {
    const int64_t last = last_success_ms_.load(std::memory_order_acquire);
    if (last == 0) {
        return false;
    }
    const int64_t age = monotonicMs() - last;
    return age >= 0 && age < 1500;
}

void Session::requestIllumination(bool allow_wake) {
    // Right after an unlock the framework opens a new operation and our pre-light
    // would light the sensor again, which looks exactly like a light that never
    // went out. Stay dark for a moment after a success.
    const int64_t since_success = monotonicMs() - last_success_ms_.load(std::memory_order_acquire);
    if (last_success_ms_.load(std::memory_order_acquire) != 0 && since_success >= 0 &&
        since_success < 1500) {
        LOGD("session: illumination suppressed %lld ms after unlock", (long long)since_success);
        return;
    }
    bool expected = false;
    if (!arm_in_flight_.compare_exchange_strong(expected, true,
                                                std::memory_order_acq_rel)) {
        return;
    }
    const std::weak_ptr<Session> weak = self_;
    // If the dispatcher is gone the task never runs, so the flag must be cleared
    // here or illumination stays disabled for the rest of the session (review H-06).
    if (!postToUi([weak, allow_wake] {
            std::shared_ptr<Session> self = weak.lock();
            if (!self) {
                return;
            }
            self->armIllumination(allow_wake);
            self->arm_in_flight_.store(false, std::memory_order_release);
        })) {
        arm_in_flight_.store(false, std::memory_order_release);
    }
}

bool Session::armCapture(uint64_t generation, State expected, bool open_window) {
    if (!isCurrent(generation, expected)) {
        return false;
    }
    VendorEngine& engine = VendorEngine::instance();
    if (!engine.open()) {
        LOGE("session: sensor unavailable");
        return false;
    }
    if (!isCurrent(generation, expected)) {
        return false;
    }
    if (!ensureUserGroup()) {
        LOGE("session: cannot activate the template group for user %d", user_id_);
        return false;
    }
    if (!isCurrent(generation, expected)) {
        return false;
    }

    // The engine gates capture on the screen being awake. Tell it before opening
    // the panel window; the engine's UI-ready wait also consults this state.
    engine.notifyScreenState(VendorEngine::kScreenStateOn);

    if (!open_window) {
        // Authentication is started by the framework as soon as the keyguard
        // appears, long before a finger is anywhere near the sensor. Opening the
        // illumination window here would flash the whole panel on every lock.
        // The window is opened on the first touch instead (see onTouchEvent),
        // while the engine simply waits for the FULL_HBM_SET gate.
        LOGI("session: illumination deferred until touch");
        illumination_pending_.store(true, std::memory_order_release);
        return true;
    }

    bool armed = false;
    bool woke_display = false;
    for (int attempt = 0; attempt < kHbmArmAttempts; ++attempt) {
        if (HbmController::instance().requestOn()) {
            armed = true;
            break;
        }
        if (!isCurrent(generation, expected)) {
            return false;
        }
        // Two failed attempts means the panel is almost certainly dozing. Wake
        // the display once, then keep retrying: the driver only accepts the HBM
        // write for an awake panel.
        if (!woke_display && attempt >= 2) {
            woke_display = true;
            wakeDisplay();
        }
        if (attempt + 1 < kHbmArmAttempts) {
            ::usleep(kHbmArmRetryUs);
        }
    }
    if (!armed) {
        LOGE("session: panel HBM unavailable after %d attempts, aborting capture",
             kHbmArmAttempts);
        return false;
    }
    if (!isCurrent(generation, expected)) {
        HbmController::instance().requestOff();
        return false;
    }

    // Only now, with the panel verified bright and the FULL_HBM_SET gate
    // satisfied, is it meaningful to declare the UI ready.
    engine.notifyUiReady(1);
    engine.notifyScreenState(VendorEngine::kScreenStateOn);
    return true;
}

void Session::runEnroll(uint64_t generation, AuthToken hat) {
    if (!isCurrent(generation, State::kEnrolling)) {
        return;
    }
    if (!armCapture(generation, State::kEnrolling, /*open_window=*/true)) {
        if (isCurrent(generation, State::kEnrolling)) {
            failOperation(generation, fp::Error::HW_UNAVAILABLE, 0);
        }
        return;
    }
    if (!isCurrent(generation, State::kEnrolling)) {
        HbmController::instance().requestOff();
        return;
    }

    uint8_t raw_token[kAuthTokenSize];
    packAuthToken(hat, raw_token);

    // The vendor enroll() takes the number of capture steps as its second
    // argument and uses it as the initial "remaining" counter. Passing the user
    // id there made the first progress event report remaining=0, so the
    // framework considered the fingerprint enrolled after a single touch.
    const int steps = VendorEngine::instance().enrollSteps();
    enroll_steps_total_.store(steps, std::memory_order_release);
    enroll_steps_done_.store(0, std::memory_order_release);

    const int rc = VendorEngine::instance().enroll(raw_token, static_cast<uint32_t>(steps),
                                                   kEnrollTimeoutSeconds);
    LOGI("session: enroll issued rc=%d", rc);

    if (!isCurrent(generation, State::kEnrolling)) {
        VendorEngine::instance().cancel();
        HbmController::instance().requestOff();
        return;
    }
    if (rc != 0) {
        failOperation(generation, fp::Error::UNABLE_TO_PROCESS, rc);
    }
}

void Session::runAuthenticate(uint64_t generation, int64_t operation_id) {
    if (!isCurrent(generation, State::kAuthenticating)) {
        return;
    }
    if (!armCapture(generation, State::kAuthenticating, /*open_window=*/false)) {
        if (isCurrent(generation, State::kAuthenticating)) {
            failOperation(generation, fp::Error::HW_UNAVAILABLE, 0);
        }
        return;
    }
    if (!isCurrent(generation, State::kAuthenticating)) {
        HbmController::instance().requestOff();
        return;
    }

    const int rc =
        VendorEngine::instance().authenticate(static_cast<uint64_t>(operation_id),
                                                      static_cast<uint32_t>(user_id_));
    LOGI("session: authenticate issued rc=%d", rc);

    if (!isCurrent(generation, State::kAuthenticating)) {
        VendorEngine::instance().cancel();
        HbmController::instance().requestOff();
        return;
    }
    if (rc != 0) {
        failOperation(generation, fp::Error::UNABLE_TO_PROCESS, rc);
    }
}

void Session::requestRearm(uint64_t generation) {
    if (closed_.load(std::memory_order_acquire)) {
        return;
    }
    const State state = state_.load(std::memory_order_acquire);
    if (state != State::kEnrolling && state != State::kAuthenticating) {
        return;
    }
    const int64_t now = monotonicMs();
    const int64_t last = last_rearm_ms_.load(std::memory_order_relaxed);
    if (now - last < kRearmThrottleMs) {
        return;
    }
    last_rearm_ms_.store(now, std::memory_order_relaxed);
    if (rearm_pending_.exchange(true)) {
        return;
    }

    const std::weak_ptr<Session> weak = self_;
    const bool posted = worker_.post([weak, generation] {
        auto self = weak.lock();
        if (!self) {
            return;
        }
        self->rearm_pending_.store(false, std::memory_order_release);
        const State state = self->state_.load(std::memory_order_acquire);
        const bool active = self->isCurrent(generation, State::kEnrolling) ||
                            self->isCurrent(generation, State::kAuthenticating);
        if (!active || state == State::kIdle) {
            return;
        }
        if (HbmController::instance().isOn()) {
            HbmController::instance().keepAlive();
            return;
        }
        if (!VendorEngine::instance().open()) {
            return;
        }
        if (!self->isCurrent(generation, state)) {
            return;
        }
        if (HbmController::instance().requestOn()) {
            VendorEngine::instance().notifyUiReady(1);
        }
    });
    if (!posted) {
        rearm_pending_.store(false, std::memory_order_release);
    }
}

void Session::onEngineMessage(const EngineMessage& message) {
    if (closed_.load(std::memory_order_acquire)) {
        return;
    }

    if (message.type == kEngineMsgEnumerate) {
        handleEnumerateMessage(message);
        return;
    }

    if (message.type == kEngineMsgError) {
        bool enumerate_active = false;
        {
            std::lock_guard<std::mutex> lock(enum_mutex_);
            enumerate_active = enum_active_;
            if (enumerate_active) {
                enum_error_ = true;
                enum_dirty_ = true;
                enum_cv_.notify_all();
            }
        }
        if (enumerate_active) {
            // The errors are not swallowed for an operation that is running at the
            // same time: it used to hang until its own 60 s timeout (review H-18).
            const State running = state_.load(std::memory_order_acquire);
            if (running != State::kAuthenticating && running != State::kEnrolling) {
                return;
            }
        }
    }

    const State state = state_.load(std::memory_order_acquire);
    if (state == State::kIdle) {
        LOGD("session: engine message %d dropped, no active operation", message.type);
        return;
    }
    const uint64_t generation = generation_.load(std::memory_order_acquire);

    switch (message.type) {
        case kEngineMsgError:
            LOGW("session: engine error code %d", message.a);
            failOperation(generation, mapEngineError(message.a), message.a);
            return;

        case kEngineMsgAcquired: {
            // A message from the previous operation of the same session arrives
            // after beginOperation() set the new state, and without this check it
            // marked the new operation as "captured" (review H-18b).
            const State now_state = state_.load(std::memory_order_acquire);
            const bool belongs = !closed_.load(std::memory_order_acquire) &&
                                 generation_.load(std::memory_order_acquire) == generation &&
                                 (now_state == State::kAuthenticating ||
                                  now_state == State::kEnrolling);
            if (!belongs) {
                LOGD("session: acquired dropped, it belongs to an older operation");
                return;
            }
            HbmController::instance().keepAlive();
            emitAcquired(message.a);
            return;
        }

        case kEngineMsgEnroll: {
            if (!isCurrent(generation, State::kEnrolling)) {
                return;
            }
            const int32_t enrollment_id = message.b;
            if (enrollment_id > 0) {
                // The id the engine reports on a match later (review H-08).
                single_template_id_.store(enrollment_id, std::memory_order_release);
            }
            HbmController::instance().keepAlive();

            const int32_t total = enroll_steps_total_.load(std::memory_order_acquire);
            const int32_t done = enroll_steps_done_.fetch_add(1, std::memory_order_acq_rel) + 1;

            // The engine reports its counter verbatim and never decrements it, so
            // completion is decided here: the template is complete after
            // GET_ENROLL_NUM accepted steps (20 for this part).
            int32_t remaining = total - done;
            if (remaining < 0) {
                remaining = 0;
            }

            LOGI("session: enrollment step %d/%d id=%d engine_remaining=%d -> report %d", done,
                 total, enrollment_id, message.a, remaining);

            if (remaining > 0) {
                emitEnrollmentProgress(enrollment_id, remaining);
                return;
            }

            // Finish the operation before the terminal progress callback: a
            // concurrent cancel then sees kIdle and cannot emit CANCELED after
            // the framework has already received the completion.
            if (!finishOperation(generation, /*close_hbm=*/true)) {
                return;
            }
            emitEnrollmentProgress(enrollment_id, 0);
            if (closed_.load(std::memory_order_acquire)) {
                return;
            }
            const std::weak_ptr<Session> weak = self_;
            worker_.post([weak] {
                if (auto self = weak.lock()) {
                    VendorEngine::instance().postEnroll();
                }
            });
            return;
        }

        case kEngineMsgRemoved:
            // Suppressed: removeEnrollments() reports the authoritative list
            // itself so the framework always gets a deterministic answer.
            LOGD("session: engine removed notice id=%d (suppressed)", message.b);
            return;

        case kEngineMsgAuthenticated: {
            if (!isCurrent(generation, State::kAuthenticating)) {
                return;
            }
            HbmController::instance().keepAlive();
            // The vendor engine puts its own notion of the template id here, and
            // it can be garbage (observed: -14811424). AOSP discards an
            // authentication result with a non-positive enrollment id, which
            // silently swallows a perfectly good match - the keyguard then simply
            // stays up. Normalise to a valid id.
            if (message.b == 0) {
                // A non-match is a real result only when the engine had already
                // captured a GOOD frame in this operation. Partial or badly placed
                // touches also surface as "no match"; forwarding those as failed
                // attempts is what once ended in the password prompt, so they stay
                // silent and the operation remains open.
                if (frame_good_.exchange(false, std::memory_order_acq_rel)) {
                    LOGI("session: no match after a captured frame; reporting a failed attempt");
                    // Report the miss but KEEP the operation open: AOSP expects the
                    // session to survive a non-match and accept the next frame.
                    // Closing it makes the framework wait for a new authenticate()
                    // call that never comes, and the icon then stays dark for every
                    // following touch.
                    finger_down_.store(false, std::memory_order_release);
                    HbmController::instance().requestOff();
                    emitAuthenticationFailed();
                    return;
                }
                LOGI("session: no match without a captured frame, keeping the operation open");
                return;
            }
            // The engine reports its own notion of the template id and it can be
            // garbage (observed: -14811424). AOSP discards a non-positive id and a
            // fabricated "1" does not match the enrollment the user registered, so
            // the id comes from enumerate/enroll or the match fails closed
            // (review H-08).
            const int32_t known_id = single_template_id_.load(std::memory_order_acquire);
            const int32_t enrollment_id = (message.b > 0) ? message.b : known_id;
            if (enrollment_id <= 0) {
                LOGE("session: match without a usable template id (engine %d, known %d)",
                     message.b, known_id);
                failOperation(generation, fp::Error::UNABLE_TO_PROCESS, 0);
                return;
            }
            if (!isAuthTokenPlausible(message.hat)) {
                // Fail closed. Forwarding an all-zero HAT as a success would
                // let the framework report an unlock without a cryptographic
                // proof. Treat it as a failed attempt, not as a success.
                LOGE("session: success reported with an invalid auth token; failing closed");
                emitAuthenticationFailed();
                HbmController::instance().requestOff();
                return;
            }
            // A result that surfaces long after the glass went empty belongs to
            // a press that is already over. Delivering it is what looked like the
            // device unlocking by itself - sometimes under a different finger.
            // Anything beyond this bound is dropped rather than applied.
            if (!finger_down_.load(std::memory_order_acquire)) {
                const int64_t up_at = finger_up_ms_.load(std::memory_order_acquire);
                const int64_t anchor =
                    (up_at > 0) ? up_at : op_started_ms_.load(std::memory_order_acquire);
                const int64_t age = (anchor > 0) ? monotonicMs() - anchor : kStaleResultMs;
                if (age > kStaleResultMs) {
                    // Dropped silently as well: a late result must not be
                    // converted into a failed attempt either.
                    LOGW("session: dropping a result that arrived %lld ms after the finger left",
                         static_cast<long long>(age));
                    HbmController::instance().requestOff();
                    return;
                }
            }

            // Make the terminal transition first: a concurrent cancel either
            // wins (and the success is not delivered) or sees kIdle and is
            // ignored. This prevents success followed by a stale CANCELED.
            if (!finishOperation(generation, /*close_hbm=*/true)) {
                return;
            }
            const AuthToken token = unpackAuthToken(message.hat);
            // SystemUI only dismisses the keyguard for a result that arrives while
            // the display is interactive. A result delivered from a dark screen is
            // logged as onAuthenticated(true) and then discarded - the panel comes
            // up still locked and the finger has to be applied a second time.
            // Waiting for the panel here is what turns a press into an unlock.
            for (int waited = 0; waited < kUiReadyWaitMs; waited += kUiReadyPollMs) {
                const int32_t display = display_state_.load(std::memory_order_acquire);
                if (display != static_cast<int32_t>(bc::DisplayState::AOD) &&
                    display != static_cast<int32_t>(bc::DisplayState::UNKNOWN)) {
                    break;
                }
                ::usleep(kUiReadyPollMs * 1000);
            }
            LOGI("session: authentication succeeded id=%d", enrollment_id);
            // The panel is about to be turned off by the framework; arm the gesture
            // now, while the controller is still powered, or the next press in sleep
            // finds no gesture path at all.
            rearmGestureMode();
            last_success_ms_.store(monotonicMs(), std::memory_order_release);
            emitAuthenticationSucceeded(enrollment_id, token);
            // The window may have been kept open for the result; once the framework
            // has it, the panel must go back to normal brightness.
            HbmController::instance().requestOff();
            // And the hold-mode helper must give the display back right away.
            {
                const int fd = ::open(km6fod::kHoldOffPath,
                                      O_WRONLY | O_CREAT | O_CLOEXEC, 0644);
                if (fd >= 0) {
                    ::close(fd);
                }
            }
            // The framework accepts the result but, with the panel asleep, does
            // not bring the display up by itself. KEY_WAKEUP is a no-op on an
            // awake screen, so it is safe to emit unconditionally; probing the
            // backlight node instead is unreliable (the `magisk` domain cannot
            // read it, and a failed read looks like an awake panel).
            LOGI("session: unlocked, ensuring the display is up");
            // Authoritative: an unlock must always leave the panel lit, and the
            // anti-duplicate window must not swallow it.
            wakeDisplay(/*force=*/true);
            return;
        }

        default:
            LOGW("session: unknown engine message type %d", message.type);
            return;
    }
}

void Session::handleEnumerateMessage(const EngineMessage& message) {
    std::lock_guard<std::mutex> lock(enum_mutex_);
    if (!enum_active_) {
        LOGD("session: enumerate notice id=%d outside enumeration", message.b);
        return;
    }
    if (message.b <= 0) {
        LOGD("session: enumerate notice with non-positive id %d dropped", message.b);
        return;
    }
    // The engine can report the same id twice; the framework keeps the list as-is
    // and a duplicate then breaks removeEnrollments (review H-20).
    for (int32_t known : enumerated_) {
        if (known == message.b) {
            return;
        }
    }
    enumerated_.push_back(message.b);
    if (enumerated_.size() == 1) {
        single_template_id_.store(message.b, std::memory_order_release);
    }
    enum_dirty_ = true;
    enum_cv_.notify_all();
}

void Session::onFodKey(bool down) {
    if (closed_.load(std::memory_order_acquire)) {
        return;
    }

    // The fingerprint gesture arrives from the touch controller, not from the
    // framework's pointer pipeline, so setIgnoreDisplayTouches() is irrelevant
    // here: that flag is about framework touch forwarding. Checking it made the
    // gesture dead exactly when it is needed, right after an unlock.
    if (!down) {
        // The controller reports the gesture as a momentary event: "down" is
        // followed by "up" within a few milliseconds, however long the finger
        // actually stays on the glass. Acting on that up immediately made a
        // single resting press impossible - the illumination window closed and
        // the capture owner was released before the display had even come back,
        // so the finger had to be lifted and placed again.
        //
        // The release is therefore deferred. A new gesture bumps the token and
        // cancels it; otherwise the window lives long enough for a finger that is
        // still on the glass to be captured.
        const uint64_t token = gesture_token_.load(std::memory_order_acquire);
        const std::weak_ptr<Session> weak = self_;
        postToUi([weak, token] {
            // Release as soon as the capture can possibly be finished - that is
            // what makes the result arrive quickly - but keep holding while the
            // framework is still bringing the display up, otherwise the sensor
            // is told the finger left before it ever had light to work with.
            // Polled through the timer queue instead of sleeping here: this runs on
            // the callback dispatcher, and a 1.5 s sleep held every queued callback
            // behind it (review H-01).
            auto started = std::make_shared<int64_t>(monotonicMs());
            auto poll = std::make_shared<std::function<void()>>();
            *poll = [weak, token, started, poll] {
                std::shared_ptr<Session> self = weak.lock();
                if (!self || self->gesture_token_.load(std::memory_order_acquire) != token) {
                    return;
                }
                const int64_t elapsed = monotonicMs() - *started;
                if (!((self->uiReadyValid() && elapsed >= 400) || elapsed >= kGestureHoldMs)) {
                    self->timers_.scheduleAfter(std::chrono::milliseconds(50), *poll);
                    return;
                }
                self->finishGestureRelease(token);
            };
            if (auto self = weak.lock()) {
                self->timers_.scheduleAfter(std::chrono::milliseconds(50), *poll);
            }
        });
        return;
    }

    gesture_token_.fetch_add(1, std::memory_order_acq_rel);
    LOGI("session: FOD wake gesture");

    // Every accepted gesture re-arms the next one. The controller clears the
    // per-gesture bit on suspend and this device does not always report the
    // LOCKSCREEN transition before the panel goes dark, so relying on that
    // transition alone left the chain dead until a reboot.
    rearmGestureMode();

    // While the framework is in detect mode it is waiting for exactly this: a
    // finger on the sensor. Reporting it is what makes the framework wake the
    // display and start an authentication on its own - the supported AOSP way to
    // unlock from a dark screen (see supportsDetectInteraction in SensorProps).
    // No illumination here: nothing is drawn yet, so there would be nothing to
    // amplify.
    if (state_.load(std::memory_order_acquire) == State::kDetecting) {
        LOGI("session: FOD gesture reported as interaction");
        const uint64_t generation = generation_.load(std::memory_order_acquire);
        if (finishOperation(generation, /*close_hbm=*/false)) {
            const std::weak_ptr<Session> weak = self_;
            postToUi([weak] {
                if (auto self = weak.lock()) {
                    self->emitInteractionDetected();
                }
            });
        }
        return;
    }

    // Everything below is vendor work: open(), notify, a bounded wait for the
    // overlay and the illumination arm. It runs on the serial worker, so the touch
    // reader keeps reading evdev and a cold dlopen never stalls it
    // (review H-02/H-03).
    const std::weak_ptr<Session> weak_press = self_;
    postToWorker([weak_press] {
        if (auto self = weak_press.lock()) {
            self->armFodPress();
        }
    });
}

void Session::armFodPress() {
    wakeDisplay();

    const bool engine_open = VendorEngine::instance().open();
    LOGD("trace: gesture open=%d", engine_open ? 1 : 0);
    if (!engine_open) {
        LOGW("session: FOD gesture with sensor unavailable");
        return;
    }
    VendorEngine::instance().notifyScreenState(VendorEngine::kScreenStateOn);

    // The overlay that provides the actual light is drawn by SystemUI after the
    // display comes up. Wait for it briefly instead of bailing out: this is what
    // makes a single press work instead of requiring a second touch.
    bool ready = false;
    for (int waited = 0; waited < kUiReadyWaitMs; waited += kUiReadyPollMs) {
        if (uiReadyValid()) {
            ready = true;
            break;
        }
        // The framework reports the wake-up through OperationContext; once the
        // display is no longer reported as asleep the overlay is on its way.
        const int32_t display = display_state_.load(std::memory_order_acquire);
        if (display != static_cast<int32_t>(bc::DisplayState::AOD) &&
            display != static_cast<int32_t>(bc::DisplayState::UNKNOWN)) {
            ready = true;
            break;
        }
        ::usleep(kUiReadyPollMs * 1000);
    }
    LOGD("trace: gesture ready=%d dark=%d isOn=%d pending=%d state=%d", ready ? 1 : 0,
         panelBacklightLevel() == 0 ? 1 : 0, HbmController::instance().isOn() ? 1 : 0,
         illumination_pending_.load(std::memory_order_acquire) ? 1 : 0,
         static_cast<int>(state_.load(std::memory_order_acquire)));
    if (!ready) {
        LOGW("session: no UI-ready after wake, illuminating anyway");
    }

    // The display was just woken above; waking again would only add latency.
    if (!finger_down_.exchange(true)) {
        VendorEngine::instance().syncFingerStatusOptic(true);
    }
    // isOn() can be stale: the panel driver resets the node to 0xFA when it
    // sleeps while our flag still says the window is open, and the press then
    // ends with a gesture, a wake key and no light at all. A dark panel is the
    // ground truth, so it forces the arm.
    const bool panel_dark = panelBacklightLevel() == 0;
    if (illumination_pending_.exchange(false) || !HbmController::instance().isOn() ||
        panel_dark) {
        requestIllumination(/*allow_wake=*/false);
    }
    // The framework creates the authenticate operation only after the keyguard is
    // up, and the overlay report comes ~1.4 s later - far outside the 0.6 s budget.
    // Light the panel straight after the wake instead, whatever the operation state:
    // the capture then starts with light already on the glass. A timer closes the
    // window if no capture arrives, and the normal path takes over when it does.
    // Light ahead only when someone can receive the result: a live operation, or a
    // UI that reported ready within the window. Without this the sensor lit up for
    // an operation that did not exist - "light on, no unlock" (DEBUG.md F27).
    const State st = state_.load(std::memory_order_acquire);
    const bool op_live =
        st == State::kAuthenticating || st == State::kEnrolling || uiReadyValid();
    if (op_live && !illuminationQuarantined() &&
        (panelBacklightLevel() == 0 || illumination_pending_.load(std::memory_order_acquire) ||
         !HbmController::instance().isOn())) {
        LOGI("session: lighting ahead of the capture");
        HbmController::instance().requestOn();
        const std::weak_ptr<Session> weak_pre = self_;
        timers_.scheduleAfter(std::chrono::milliseconds(2500), [weak_pre] {
            auto self = weak_pre.lock();
            if (!self) {
                return;
            }
            const State st = self->state_.load(std::memory_order_acquire);
            if (st == State::kIdle) {
                LOGI("session: no capture arrived, closing the pre-light");
                HbmController::instance().requestOff();
            }
        });
    }
}
void Session::finishGestureRelease(uint64_t token) {
    if (gesture_token_.load(std::memory_order_acquire) != token) {
        return;
    }
    const bool was_down = clearFingerState();
    if (was_down) {
        finger_up_ms_.store(monotonicMs(), std::memory_order_release);
    }
    const State op_state = state_.load(std::memory_order_acquire);
    const bool op_live = op_state == State::kAuthenticating || op_state == State::kEnrolling;
    if (!op_live && !wakeInFlight() && HbmController::instance().isOn()) {
        // Nobody is waiting for a result and no wake is in flight, so keeping the
        // sensor lit would only glow at a dead screen. While a wake is in flight the
        // light is held until the operation arrives.
        LOGI("session: finger up without an operation, light off");
        HbmController::instance().requestOff();
    } else if (was_down || HbmController::instance().isOn()) {
        // A GOOD frame means the engine is comparing right now. Killing the light a
        // few milliseconds later is what made the result arrive blind and get
        // dropped; the watchdog closes the window when the result lands.
        if (frame_good_.load(std::memory_order_acquire)) {
            LOGI("session: finger up with a captured frame, light kept for the result");
        } else {
            LOGI("session: finger up, closing illumination window");
            HbmController::instance().requestOff();
        }
    }
}

void Session::overlayTick(int waited) {
    if (!overlay_wait_active_.load(std::memory_order_acquire)) {
        return;
    }
    const State inner = state_.load(std::memory_order_acquire);
    if (uiReadyValid() && (inner == State::kAuthenticating || inner == State::kEnrolling)) {
        LOGI("session: overlay appeared, illuminating");
        requestIllumination(/*allow_wake=*/false);
        const int64_t touch_age = monotonicMs() - last_touch_ms_.load(std::memory_order_acquire);
        if (touch_age >= 0 && touch_age < 1500) {
            // The finger may still be resting on the glass even though the momentary
            // gesture already reported "up" - that is the held press from a dark
            // screen. Claim it so the engine captures this press. The vendor calls
            // stay on the worker (review H-04).
            finger_down_.store(true, std::memory_order_release);
            const int32_t x = last_touch_x_.load(std::memory_order_relaxed);
            const int32_t y = last_touch_y_.load(std::memory_order_relaxed);
            postToWorker([x, y] {
                VendorEngine::instance().syncFingerStatusOptic(true);
                VendorEngine::instance().sendTouchInfo(x, y);
            });
        }
        overlay_wait_active_.store(false, std::memory_order_release);
        return;
    }
    if (waited >= 1500) {
        // Observable instead of silent: this is the case where the icon stays dark
        // and the press looks dead.
        LOGW("session: overlay did not appear within 1.5 s; illumination skipped");
        overlay_wait_active_.store(false, std::memory_order_release);
        return;
    }
    const std::weak_ptr<Session> weak = self_;
    timers_.scheduleAfter(std::chrono::milliseconds(50), [weak, waited] {
        if (auto self = weak.lock()) {
            self->overlayTick(waited + 50);
        }
    });
}

void Session::onTouchEvent(int x, int y, bool down) {
    if (closed_.load(std::memory_order_acquire)) {
        return;
    }

    const State state = state_.load(std::memory_order_acquire);

    // Release: the engine is told explicitly, otherwise it keeps waiting for a
    // frame from a finger that is no longer on the glass and ends in err(1030).
    // The illumination window belongs to the finger, not to the operation: with
    // the finger gone there is nothing to capture, and leaving the panel bright
    // is what made it stay lit after an unlock until the watchdog expired.
    if (!down) {
        // The digitizer streams a release for every frame the finger wobbles on
        // the glass, and an immediate release per sample tore the capture apart:
        // HBM and the optic state were switched off and on dozens of times per
        // second, so the engine never got a stable run of frames. The release is
        // deferred instead and cancelled by the next down sample.
        const uint64_t token = release_token_.load(std::memory_order_acquire);
        const std::weak_ptr<Session> weak = self_;
        postToUi([weak, token, state] {
            // Only the most recent release sample deserves a wait; the older ones
            // are already superseded and must not occupy a thread.
            {
                const std::shared_ptr<Session> now = weak.lock();
                if (!now || now->release_token_.load(std::memory_order_acquire) != token) {
                    return;
                }
            }
            // Two timer stages instead of two sleeps: this runs on the callback
            // dispatcher, and holding it for 250 ms delayed every queued callback
            // (review H-01).
            auto self = weak.lock();
            if (!self) {
                return;
            }
            auto stage = std::make_shared<std::function<void()>>();
            *stage = [weak, token, state, stage] {
                auto now = weak.lock();
                if (!now || now->release_token_.load(std::memory_order_acquire) != token) {
                    return;
                }
                if (now->clearFingerState()) {
                    now->finger_up_ms_.store(monotonicMs(), std::memory_order_release);
                }
                // The panel stays bright for longer: a release followed by another
                // sample a few frames later must not blink the window.
                now->timers_.scheduleAfter(std::chrono::milliseconds(kWindowHoldMs - kOpticReleaseMs),
                                           [weak, token, state] {
                                               auto self = weak.lock();
                                               if (!self ||
                                                   self->release_token_.load(std::memory_order_acquire) !=
                                                       token) {
                                                   return;
                                               }
                                               if ((state == State::kEnrolling ||
                                                    state == State::kAuthenticating) &&
                                                   HbmController::instance().isOn()) {
                                                   if (self->frame_good_.load(
                                                           std::memory_order_acquire)) {
                                                       LOGI("session: finger up with a captured frame, light kept for the result");
                                                   } else {
                                                       LOGI("session: finger up, closing illumination window");
                                                       HbmController::instance().requestOff();
                                                   }
                                               }
                                           });
            };
            self->timers_.scheduleAfter(std::chrono::milliseconds(kOpticReleaseMs), *stage);
        });
        return;
    }

    release_token_.fetch_add(1, std::memory_order_acq_rel);

    const bool inside = inSensorZone(x, y);
    if (!inside) {
        // Any touch away from the sensor means the user is handling the phone:
        // tell the hold-mode helper to give the display back (it owns brightness).
        const int fd = ::open(km6fod::kHoldOffPath, O_WRONLY | O_CREAT | O_CLOEXEC, 0644);
        if (fd >= 0) {
            ::close(fd);
        }
    }
    if (inside) {
        // Record the press before any state check: the touch that wakes the panel
        // arrives while the HAL is still idle, and its coordinates are what let the
        // HAL claim a held finger once the framework starts the operation.
        last_touch_x_.store(x, std::memory_order_relaxed);
        last_touch_y_.store(y, std::memory_order_relaxed);
        last_touch_ms_.store(monotonicMs(), std::memory_order_release);
    }

    // Bring-up tracing, rate limited: shows whether coordinates reach the HAL
    // and whether the session state allows forwarding them to the sensor.
    {
        static thread_local int traced = 0;
        if ((traced++ % 10) == 0) {
            LOGD("session: touch (%d,%d) state=%d inside=%d ignore=%d", x, y,
                 static_cast<int>(state), inside ? 1 : 0,
                 ignore_touches_.load(std::memory_order_relaxed) ? 1 : 0);
        }
    }

    // A touch can arrive with nothing to capture yet - most often right after the
    // screen was locked, when the framework has not started a session, and on a
    // dark screen where it starts one only once the display is up. Such a touch
    // cannot arm anything (there is no capture owner), but it must wake the panel,
    // exactly like a physical key would.
    //
    // KEY_WAKEUP is a no-op on an already awake display, so this is safe to do
    // whenever the overlay is not running. displayState is deliberately not
    // consulted: it is only updated when the framework feels like it and is stale
    // often enough that gating on it left presses doing nothing at all.
    if (state == State::kIdle || (inside && !uiReadyValid())) {
        if (inside) {
            static thread_local int64_t last_ms = 0;
            const int64_t now = monotonicMs();
            if (now - last_ms > 900) {
                last_ms = now;
                LOGD("touch without a running overlay, waking the display");
            }
            // This press is a normal MT touch, not the FOD gesture (the driver keeps
            // the gesture in the kernel), so the pre-light has to happen here too:
            // otherwise the panel stays dark until the framework creates the
            // operation, and the press is spent before there is any light.
            // Waking costs a JVM start (~250 ms), so it happens only when there is
            // no operation yet: hold mode wakes the keyguard once on entry, and a
            // press then goes straight to the capture.
            const State touch_state = state_.load(std::memory_order_acquire);
            if (touch_state == State::kIdle) {
                wakeDisplay();
            }
            // Only light ahead of the capture when there IS a capture to light for.
            // A touch with no operation (state == kIdle) is just a touch: it used to
            // burn the sensor light for 2.5 s while the framework never started an
            // operation ("overlay did not appear").
            const bool capturing = touch_state == State::kEnrolling ||
                                   touch_state == State::kAuthenticating;
            if (capturing && !illuminationQuarantined() &&
                (panelBacklightLevel() == 0 || !HbmController::instance().isOn())) {
                LOGI("session: pre-lighting before the overlay");
                HbmController::instance().requestOn();
                const std::weak_ptr<Session> weak_pre = self_;
                timers_.scheduleAfter(std::chrono::milliseconds(2500), [weak_pre] {
                    auto self = weak_pre.lock();
                    if (!self) {
                        return;
                    }
                    if (self->state_.load(std::memory_order_acquire) == State::kIdle) {
                        LOGI("session: no operation arrived, closing the pre-light");
                        HbmController::instance().requestOff();
                    }
                });
            }

            // Right after a lock the framework has no session yet, so there is
            // nothing to illuminate and the press would only wake the panel. It
            // starts the session once the display is up, so wait for the overlay
            // instead of requiring a second press.
            finger_down_.store(true, std::memory_order_release);
            bool expected = false;
            if (overlay_wait_active_.compare_exchange_strong(expected, true)) {
                // Polled through the timer queue: the old loop slept 1.5 s on the
                // callback dispatcher, holding every queued callback behind it
                // (review H-01).
                overlayTick(0);
            }
        }
        return;
    }

    // Report the exact reason a press produced no illumination. Without this the
    // failure is invisible: the sample simply reaches none of the branches below
    // and the panel stays dark. Rate limited so a streaming finger cannot flood
    // the log.
    if (down) {
        const char* reason = nullptr;
        if (!(state == State::kEnrolling || state == State::kAuthenticating)) {
            reason = "session not capturing";
        } else if (!inside) {
            reason = "outside sensor zone";
        } else if (ignore_touches_.load(std::memory_order_relaxed)) {
            reason = "framework ignores touches";
        } else if (!uiReadyValid()) {
            reason = "overlay not ready";
        }
        if (reason != nullptr) {
            static thread_local int64_t last_ms = 0;
            const int64_t now = monotonicMs();
            if (now - last_ms > 900) {
                last_ms = now;
                LOGD("touch skipped: %s", reason);
            }
        }
    }

    if ((state == State::kEnrolling || state == State::kAuthenticating) && inside &&
        !ignore_touches_.load(std::memory_order_relaxed)) {
        // Remember the press even when there is nothing to capture yet: the touch
        // that wakes the panel arrives before the framework has a session, and
        // these coordinates are what let the HAL claim the finger (and start a
        // capture) once the operation begins. Without them a held finger is lost
        // and the user has to lift and touch again.
        last_touch_x_.store(x, std::memory_order_relaxed);
        last_touch_y_.store(y, std::memory_order_relaxed);
        last_touch_ms_.store(monotonicMs(), std::memory_order_release);

        // Touches are only meaningful while the framework is showing the
        // fingerprint overlay. On the PIN pad and the emergency dialer the
        // sensor zone is just part of the keypad: lighting the panel there (and
        // feeding coordinates to the sensor) is wrong.
        if (!uiReadyValid()) {
            return;
        }

        last_touch_x_.store(x, std::memory_order_relaxed);
        last_touch_y_.store(y, std::memory_order_relaxed);
        const bool fresh_down = !finger_down_.exchange(true);
        illumination_pending_.store(false, std::memory_order_release);

        // The window is (re)opened on EVERY sample the panel is not verified on,
        // not just the first one of a press. Arming only on the first sample made
        // every later one powerless to relight a window that had closed for any
        // reason - the "lights up only sometimes" the user kept hitting.
        // requestOn() is idempotent for an open window, so repeating costs almost
        // nothing, and the work happens off this thread.
        if (!HbmController::instance().isOn()) {
            requestIllumination(/*allow_wake=*/false);
        }

        if (fresh_down) {
            LOGI("session: finger down, illumination requested");
            VendorEngine::instance().syncFingerStatusOptic(true);
        }

        VendorEngine::instance().sendTouchInfo(x, y);
    }

    if (state == State::kDetecting) {
        if (!inside) {
            return;
        }
        const uint64_t generation = generation_.load(std::memory_order_acquire);
        if (finishOperation(generation, /*close_hbm=*/false)) {
            const std::weak_ptr<Session> weak = self_;
            postToUi([weak] {
                if (auto self = weak.lock()) {
                    self->emitInteractionDetected();
                }
            });
        }
        return;
    }

    if (inside && (state == State::kEnrolling || state == State::kAuthenticating)) {
        // Keep the same UI-ready gate as the arm path above. Without it this
        // branch re-lit the panel on the PIN pad and the emergency dialer, where
        // the zone is just part of the keypad and no finger-down was ever tracked
        // to pair it with a close.
        if (!uiReadyValid()) {
            return;
        }
        if (HbmController::instance().isOn()) {
            HbmController::instance().keepAlive();
        } else {
            requestRearm(generation_.load(std::memory_order_acquire));
        }
    }
}

// ------------------------------------------------------------- AIDL: setup

::ndk::ScopedAStatus Session::generateChallenge() {
    if (closed_.load(std::memory_order_acquire)) {
        emitChallengeGenerated(1);
        return ::ndk::ScopedAStatus::ok();
    }
    const std::weak_ptr<Session> weak = self_;
    if (!worker_.post([weak] {
            auto self = weak.lock();
            if (!self || self->closed_.load(std::memory_order_acquire)) {
                return;
            }
            int64_t challenge = 1;
            if (VendorEngine::instance().open() && self->ensureUserGroup()) {
                // The group switch kicks off optic calibration, which replaces
                // the TA's enroll challenge. Wait for it to finish, otherwise
                // the very next enroll() returns INVALID_CHALLENGE (-1004).
                if (self->group_just_applied_.exchange(false)) {
                    LOGI("session: waiting for optic calibration to settle");
                    ::usleep(kCalibrationSettleUs);
                }
                const uint64_t value = VendorEngine::instance().preEnroll();
                if (value != 0) {
                    challenge = static_cast<int64_t>(value);
                }
            } else {
                LOGW("session: generateChallenge without a sensor, returning placeholder");
            }
            LOGI("session: challenge %lld", static_cast<long long>(challenge));
            self->postToUi([weak, challenge] {
                auto inner = weak.lock();
                if (!inner || inner->closed_.load(std::memory_order_acquire)) {
                    return;
                }
                inner->emitChallengeGenerated(challenge);
            });
        })) {
        emitChallengeGenerated(1);
    }
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Session::revokeChallenge(int64_t in_challenge) {
    // Mandatory acknowledgement: without it FingerprintRevokeChallengeClient
    // stays pending forever and the enroll button never becomes usable.
    if (callback_) {
        checkCallback(callback_->onChallengeRevoked(in_challenge), "onChallengeRevoked");
    }
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Session::startEnroll(const AuthToken& hat,
                                          std::shared_ptr<CancellationSignal>* out) {
    if (out == nullptr) {
        return ::ndk::ScopedAStatus::ok();
    }
    if (closed_.load(std::memory_order_acquire)) {
        auto cancellation = ::ndk::SharedRefBase::make<Cancellation>(self_, 0);
        markStable(cancellation->asBinder().get(), "ICancellationSignal");
        *out = cancellation;
        return ::ndk::ScopedAStatus::ok();
    }

    const uint64_t generation = beginOperation(State::kEnrolling);
    auto cancellation = ::ndk::SharedRefBase::make<Cancellation>(self_, generation);
    markStable(cancellation->asBinder().get(), "ICancellationSignal");
    *out = cancellation;
    if (generation == 0) {
        return ::ndk::ScopedAStatus::ok();
    }

    const std::weak_ptr<Session> weak = self_;
    if (!worker_.post([weak, generation, hat] {
            auto self = weak.lock();
            if (!self) {
                return;
            }
            self->runEnroll(generation, hat);
        })) {
        failOperation(generation, fp::Error::UNABLE_TO_PROCESS, -1);
    }
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Session::enroll(
    const AuthToken& in_hat, std::shared_ptr<CancellationSignal>* _aidl_return) {
    return startEnroll(in_hat, _aidl_return);
}

::ndk::ScopedAStatus Session::startAuthenticate(
    int64_t operation_id, std::shared_ptr<CancellationSignal>* out) {
    if (out == nullptr) {
        return ::ndk::ScopedAStatus::ok();
    }
    if (closed_.load(std::memory_order_acquire)) {
        auto cancellation = ::ndk::SharedRefBase::make<Cancellation>(self_, 0);
        markStable(cancellation->asBinder().get(), "ICancellationSignal");
        *out = cancellation;
        return ::ndk::ScopedAStatus::ok();
    }

    const uint64_t generation = beginOperation(State::kAuthenticating);
    auto cancellation = ::ndk::SharedRefBase::make<Cancellation>(self_, generation);
    markStable(cancellation->asBinder().get(), "ICancellationSignal");
    *out = cancellation;
    if (generation == 0) {
        return ::ndk::ScopedAStatus::ok();
    }

    const std::weak_ptr<Session> weak = self_;
    if (!worker_.post([weak, generation, operation_id] {
            auto self = weak.lock();
            if (!self) {
                return;
            }
            self->runAuthenticate(generation, operation_id);
        })) {
        failOperation(generation, fp::Error::UNABLE_TO_PROCESS, -1);
    }
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Session::authenticate(
    int64_t in_operationId, std::shared_ptr<CancellationSignal>* _aidl_return) {
    return startAuthenticate(in_operationId, _aidl_return);
}

::ndk::ScopedAStatus Session::detectInteraction(
    std::shared_ptr<CancellationSignal>* _aidl_return) {
    if (_aidl_return == nullptr) {
        return ::ndk::ScopedAStatus::ok();
    }
    if (closed_.load(std::memory_order_acquire)) {
        auto cancellation = ::ndk::SharedRefBase::make<Cancellation>(self_, 0);
        markStable(cancellation->asBinder().get(), "ICancellationSignal");
        *_aidl_return = cancellation;
        return ::ndk::ScopedAStatus::ok();
    }

    // Touch detection is done by our evdev reader, not by the optical engine.
    // Deliberately do not open the sensor or turn on HBM here: detectInteraction
    // is called while the screen is on but no capture is requested, and a panel
    // flash for a mere proximity query is both pointless and panel wear.
    const uint64_t generation = beginOperation(State::kDetecting);
    auto cancellation = ::ndk::SharedRefBase::make<Cancellation>(self_, generation);
    markStable(cancellation->asBinder().get(), "ICancellationSignal");
    *_aidl_return = cancellation;
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Session::enumerateEnrollments() {
    startEnumerate();
    return ::ndk::ScopedAStatus::ok();
}

void Session::startEnumerate() {
    if (closed_.load(std::memory_order_acquire)) {
        emitEnrollmentsEnumerated({});
        return;
    }

    uint64_t generation = 0;
    {
        std::lock_guard<std::mutex> lock(enum_mutex_);
        generation = ++enum_generation_;
        enum_active_ = true;
        enum_error_ = false;
        enum_dirty_ = false;
        enumerated_.clear();
    }

    const std::weak_ptr<Session> weak = self_;
    if (!worker_.post([weak, generation] {
            auto self = weak.lock();
            if (!self) {
                return;
            }
            self->runEnumerate(generation);
        })) {
        finishEnumerate(generation, {}, /*authoritative=*/false);
    }
}

void Session::runEnumerate(uint64_t generation) {
    {
        std::lock_guard<std::mutex> lock(enum_mutex_);
        if (generation != enum_generation_ || !enum_active_) {
            return;
        }
    }

    VendorEngine& engine = VendorEngine::instance();
    if (!engine.open() || !ensureUserGroup()) {
        finishEnumerate(generation, {}, /*authoritative=*/false);
        return;
    }

    const int rc = engine.enumerate();
    if (rc != 0) {
        LOGW("session: enumerate rc=%d", rc);
    }

    const auto started = std::chrono::steady_clock::now();
    auto quiet_since = started;
    for (;;) {
        std::unique_lock<std::mutex> lock(enum_mutex_);
        if (generation != enum_generation_ || !enum_active_) {
            return;
        }
        if (enum_error_) {
            break;
        }
        const auto now = std::chrono::steady_clock::now();
        if (now - started >= std::chrono::milliseconds(kEnumerateMaxMs)) {
            break;
        }
        const auto quiet_deadline = quiet_since + std::chrono::milliseconds(kEnumerateQuietMs);
        const auto hard_deadline = started + std::chrono::milliseconds(kEnumerateMaxMs);
        const auto wake_at = std::min(quiet_deadline, hard_deadline);
        if (now >= wake_at) {
            break;
        }
        if (enum_cv_.wait_until(lock, wake_at, [&] { return enum_dirty_ || enum_error_; })) {
            if (enum_error_) {
                break;
            }
            enum_dirty_ = false;
            quiet_since = std::chrono::steady_clock::now();
        }
    }

    std::vector<int32_t> ids;
    bool error = false;
    {
        std::lock_guard<std::mutex> lock(enum_mutex_);
        if (generation != enum_generation_ || !enum_active_) {
            return;
        }
        ids = enumerated_;
        error = enum_error_;
        enum_active_ = false;
        enum_dirty_ = false;
    }
    // The marker survives a reboot: the first enumeration after boot is exactly
    // where a transient empty answer would wipe the enrollment for good.
    const char* kMarker = km6fod::kTemplatesMarkerPath;
    const bool seen_before = saw_templates_.load(std::memory_order_acquire) ||
                             ::access(kMarker, F_OK) == 0;
    if (!error && ids.empty() && seen_before) {
        // The engine already reported templates in this boot; an empty answer now
        // is a transient failure, not a wipe. Never destroy the user's enrollment.
        LOGW("session: empty enumeration after a non-empty one, reporting an error");
        error = true;
    }
    if (!ids.empty()) {
        saw_templates_.store(true, std::memory_order_release);
        const int fd = ::open(kMarker, O_WRONLY | O_CREAT | O_CLOEXEC, 0644);
        if (fd >= 0) {
            ::close(fd);
        }
    }
    finishEnumerate(generation, std::move(ids), /*authoritative=*/!error);
}

void Session::finishEnumerate(uint64_t generation, std::vector<int32_t> ids,
                               bool authoritative) {
    {
        std::lock_guard<std::mutex> lock(enum_mutex_);
        if (generation == enum_generation_) {
            enum_active_ = false;
        }
    }
    const std::weak_ptr<Session> weak = self_;
    postToUi([weak, generation, ids = std::move(ids), authoritative] {
        auto self = weak.lock();
        if (!self || self->closed_.load(std::memory_order_acquire)) {
            return;
        }
        {
            std::lock_guard<std::mutex> lock(self->enum_mutex_);
            if (generation != self->enum_generation_) {
                return;
            }
        }
        if (!authoritative) {
            // Never hand the framework an empty list after a failure: it treats the
            // list as authoritative and drops the enrollment, which is how a
            // transient engine error erased the user's fingerprint (review F-013).
            LOGE("session: enumeration failed, reporting an error instead of an empty list");
            self->emitError(fp::Error::UNABLE_TO_PROCESS, 0);
            return;
        }
        if (ids.empty()) {
            // Everything was removed: forget the id instead of pointing at a
            // template that no longer exists.
            self->single_template_id_.store(-1, std::memory_order_release);
        } else if (ids.size() == 1) {
            self->single_template_id_.store(ids.front(), std::memory_order_release);
        }
        LOGI("session: enumerated %zu template(s)", ids.size());
        self->emitEnrollmentsEnumerated(ids);
    });
}

::ndk::ScopedAStatus Session::removeEnrollments(
    const std::vector<int32_t>& in_enrollmentIds) {
    if (closed_.load(std::memory_order_acquire)) {
        emitEnrollmentsRemoved({});
        return ::ndk::ScopedAStatus::ok();
    }

    std::vector<int32_t> ids = in_enrollmentIds;
    const uint32_t gid = static_cast<uint32_t>(user_id_);
    const std::weak_ptr<Session> weak = self_;
    if (!worker_.post([weak, ids = std::move(ids), gid] {
            auto self = weak.lock();
            if (!self || self->closed_.load(std::memory_order_acquire)) {
                return;
            }
            std::vector<int32_t> removed;
            VendorEngine& engine = VendorEngine::instance();
            if (engine.open() && self->ensureUserGroup()) {
                for (int32_t id : ids) {
                    const int rc = engine.removeEnrollment(gid, static_cast<uint32_t>(id));
                    LOGI("session: remove(%d) rc=%d", id, rc);
                    if (rc == 0) {
                        removed.push_back(id);
                    }
                }
            } else {
                LOGE("session: removeEnrollments without a sensor");
            }
            const std::weak_ptr<Session> inner_weak = weak;
            self->postToUi([inner_weak, removed = std::move(removed)] {
                auto inner = inner_weak.lock();
                if (!inner || inner->closed_.load(std::memory_order_acquire)) {
                    return;
                }
                inner->emitEnrollmentsRemoved(removed);
            });
        })) {
        emitEnrollmentsRemoved({});
    }
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Session::getAuthenticatorId() {
    if (closed_.load(std::memory_order_acquire)) {
        emitAuthenticatorId(0, /*invalidated=*/false);
        return ::ndk::ScopedAStatus::ok();
    }
    const std::weak_ptr<Session> weak = self_;
    if (!worker_.post([weak] {
            auto self = weak.lock();
            if (!self || self->closed_.load(std::memory_order_acquire)) {
                return;
            }
            int64_t id = 0;
            // Do not open the sensor for this query: AOSP calls it during user
            // start, before any operation, and the spec explicitly forbids
            // opening the sensor there.
            if (VendorEngine::instance().isOpen()) {
                id = static_cast<int64_t>(VendorEngine::instance().getAuthenticatorId());
            }
            self->postToUi([weak, id] {
                auto inner = weak.lock();
                if (!inner || inner->closed_.load(std::memory_order_acquire)) {
                    return;
                }
                inner->emitAuthenticatorId(id, /*invalidated=*/false);
            });
        })) {
        emitAuthenticatorId(0, /*invalidated=*/false);
    }
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Session::invalidateAuthenticatorId() {
    if (closed_.load(std::memory_order_acquire)) {
        emitAuthenticatorId(1, /*invalidated=*/true);
        return ::ndk::ScopedAStatus::ok();
    }
    const std::weak_ptr<Session> weak = self_;
    if (!worker_.post([weak] {
            auto self = weak.lock();
            if (!self || self->closed_.load(std::memory_order_acquire)) {
                return;
            }
            int64_t id = 1;
            if (VendorEngine::instance().isOpen()) {
                id = static_cast<int64_t>(VendorEngine::instance().getAuthenticatorId()) + 1;
            }
            self->postToUi([weak, id] {
                auto inner = weak.lock();
                if (!inner || inner->closed_.load(std::memory_order_acquire)) {
                    return;
                }
                inner->emitAuthenticatorId(id, /*invalidated=*/true);
            });
        })) {
        emitAuthenticatorId(1, /*invalidated=*/true);
    }
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Session::resetLockout(const AuthToken& /*in_hat*/) {
    if (callback_) {
        checkCallback(callback_->onLockoutCleared(), "onLockoutCleared");
    }
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Session::close() {
    // Publish the closed flag before touching the operation state: a concurrent
    // startEnroll/startAuthenticate must not slip in between the two steps.
    closed_.store(true, std::memory_order_release);
    State old_state = State::kIdle;
    {
        std::lock_guard<std::mutex> lock(op_mutex_);
        old_state = state_.load(std::memory_order_acquire);
        state_.store(State::kIdle, std::memory_order_release);
        ++generation_;
        ++cancel_epoch_;
        if (timeout_id_ != 0) {
            timers_.cancel(timeout_id_);
            timeout_id_ = 0;
        }
    }
    LOGI("session: closing (old state %d)", static_cast<int>(old_state));

    HbmController::instance().requestOff();
    postToWorker([] { VendorEngine::instance().cancel(); });

    if (old_state == State::kEnrolling) {
        const std::weak_ptr<Session> weak = self_;
        if (!worker_.post([weak] {
                if (auto self = weak.lock()) {
                    VendorEngine::instance().postEnroll();
                }
            })) {
            VendorEngine::instance().postEnroll();
        }
    }

    emitSessionClosed();
    return ::ndk::ScopedAStatus::ok();
}

void Session::shutdown() {
    closed_.store(true, std::memory_order_release);
    State old_state = State::kIdle;
    {
        std::lock_guard<std::mutex> lock(op_mutex_);
        old_state = state_.load(std::memory_order_acquire);
        state_.store(State::kIdle, std::memory_order_release);
        ++generation_;
        ++cancel_epoch_;
        if (timeout_id_ != 0) {
            timers_.cancel(timeout_id_);
            timeout_id_ = 0;
        }
    }
    HbmController::instance().requestOff();
    postToWorker([] { VendorEngine::instance().cancel(); });
    if (old_state == State::kEnrolling) {
        const std::weak_ptr<Session> weak = self_;
        worker_.post([weak] {
            if (auto self = weak.lock()) {
                VendorEngine::instance().postEnroll();
            }
        });
    }
}

// ------------------------------------------------------ AIDL: pointer input

::ndk::ScopedAStatus Session::onPointerDown(int32_t /*in_pointerId*/, int32_t in_x,
                                            int32_t in_y, float /*in_minor*/,
                                            float /*in_major*/) {
    onTouchEvent(in_x, in_y, true);
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Session::onPointerUp(int32_t /*in_pointerId*/) {
    // A pointer-up used to be a no-op, so a framework that sends pointer events
    // would leave the finger "down" forever (review H-36).
    onTouchEvent(0, 0, false);
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Session::onPointerDownWithContext(const PointerContext& in_context) {
    onTouchEvent(static_cast<int>(in_context.x), static_cast<int>(in_context.y), true);
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Session::onPointerUpWithContext(const PointerContext& /*in_context*/) {
    onTouchEvent(0, 0, false);
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Session::onPointerCancelWithContext(const PointerContext& /*in_context*/) {
    return ::ndk::ScopedAStatus::ok();
}

// ------------------------------------------------------- AIDL: UI lifecycle

::ndk::ScopedAStatus Session::onUiReady() {
    if (closed_.load(std::memory_order_acquire)) {
        return ::ndk::ScopedAStatus::ok();
    }
    LOGI("session: onUiReady");
    // The framework only reports UI-ready while the fingerprint overlay is on
    // screen, so this is what marks an illumination request as legitimate.
    markUiReady();
    // Do not open the sensor and do not turn on HBM here. HBM is opened in
    // armCapture(), immediately before the capture command, and verified by
    // readback. Opening it on a mere UI-ready callback would leave the panel
    // bright with no capture owner if the framework dies before authenticate.
    VendorEngine& engine = VendorEngine::instance();
    if (engine.isOpen()) {
        engine.notifyScreenState(VendorEngine::kScreenStateOn);
        engine.notifyUiReady(1);
    }
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Session::onContextChanged(const Context& in_context) {
    const int32_t display = static_cast<int32_t>(in_context.displayState);
    const int32_t previous = display_state_.exchange(display, std::memory_order_acq_rel);
    if (display != previous) {
        LOGD("session: display state %d -> %d", previous, display);
        // Entering AOD/doze mid-operation leaves the illumination window in a
        // state the panel no longer honours: the driver drops HBM writes while
        // the display is idle, yet the controller keeps believing the panel is
        // bright and would skip the next arm. Close the window so the next arm
        // really issues the panel command again.
        // The controller drops its gesture bit when it suspends, and this device
        // never reports AOD: the only signal before sleep is the keyguard
        // appearing. Rearm there, through the vendor call that actually works.
        if (display == static_cast<int32_t>(bc::DisplayState::LOCKSCREEN)) {
            LOGI("session: keyguard shown, re-arming the FOD gesture");
            rearmGestureMode();
        }
        if (display == static_cast<int32_t>(bc::DisplayState::NO_UI)) {
            // Unlocked: nothing must keep the sensor area lit any more.
            LOGI("session: unlocked, closing the illumination window");
            illumination_pending_.store(false, std::memory_order_release);
            HbmController::instance().requestOff();
        }
        if (display == static_cast<int32_t>(bc::DisplayState::AOD) ||
            display == static_cast<int32_t>(bc::DisplayState::UNKNOWN)) {
            LOGI("session: display entering doze, closing illumination window");
            illumination_pending_.store(false, std::memory_order_release);
            HbmController::instance().requestOff();
            rearmGestureMode();
        }
    }
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Session::setIgnoreDisplayTouches(bool in_shouldIgnore) {
    ignore_touches_.store(in_shouldIgnore, std::memory_order_relaxed);
    LOGD("session: ignore display touches = %d", in_shouldIgnore ? 1 : 0);
    return ::ndk::ScopedAStatus::ok();
}

// ------------------------------------------------ AIDL: context-bound calls

::ndk::ScopedAStatus Session::enrollWithContext(
    const AuthToken& in_hat, const Context& in_context,
    std::shared_ptr<CancellationSignal>* _aidl_return) {
    display_state_.store(static_cast<int32_t>(in_context.displayState),
                         std::memory_order_release);
    return startEnroll(in_hat, _aidl_return);
}

::ndk::ScopedAStatus Session::authenticateWithContext(
    int64_t in_operationId, const Context& in_context,
    std::shared_ptr<CancellationSignal>* _aidl_return) {
    display_state_.store(static_cast<int32_t>(in_context.displayState),
                         std::memory_order_release);
    return startAuthenticate(in_operationId, _aidl_return);
}

::ndk::ScopedAStatus Session::detectInteractionWithContext(
    const Context& in_context, std::shared_ptr<CancellationSignal>* _aidl_return) {
    display_state_.store(static_cast<int32_t>(in_context.displayState),
                         std::memory_order_release);
    return detectInteraction(_aidl_return);
}

// --------------------------------------------------------- callback helpers

bool Session::checkCallback(::ndk::ScopedAStatus status, const char* what) {
    if (status.isOk()) {
        return true;
    }
    LOGW("session: %s failed (status=%d)", what, static_cast<int>(status.getStatus()));
    handleDeadCallback();
    return false;
}

void Session::handleDeadCallback() {
    if (closed_.exchange(true, std::memory_order_acq_rel)) {
        return;
    }
    LOGE("session: framework callback is dead; aborting the operation");
    {
        std::lock_guard<std::mutex> lock(op_mutex_);
        state_.store(State::kIdle, std::memory_order_release);
        ++generation_;
        ++cancel_epoch_;
        if (timeout_id_ != 0) {
            timers_.cancel(timeout_id_);
            timeout_id_ = 0;
        }
    }
    HbmController::instance().requestOff();
    postToWorker([] { VendorEngine::instance().cancel(); });
}

bool Session::emitError(fp::Error error, int32_t vendor_code) {
    if (!callback_) {
        return false;
    }
    LOGW("session: onError(%d, vendor=%d)", static_cast<int>(error), vendor_code);
    return checkCallback(callback_->onError(error, vendor_code), "onError");
}

bool Session::emitAcquired(int32_t engine_code) {
    if (!callback_) {
        return false;
    }
    if (engine_code == 0) {
        // The engine says "this frame is good": a comparison is expected to follow.
        // Only such a frame may later become a failed attempt.
        frame_good_.store(true, std::memory_order_release);
    }
    const fp::AcquiredInfo info = mapEngineAcquired(engine_code);
    LOGD("session: onAcquired(%d) from engine code %d", static_cast<int>(info), engine_code);
    return checkCallback(callback_->onAcquired(info, engine_code), "onAcquired");
}

bool Session::emitEnrollmentProgress(int32_t id, int32_t remaining) {
    if (!callback_) {
        return false;
    }
    return checkCallback(callback_->onEnrollmentProgress(id, remaining),
                         "onEnrollmentProgress");
}

bool Session::emitAuthenticationSucceeded(int32_t id, const AuthToken& token) {
    if (!callback_) {
        return false;
    }
    return checkCallback(callback_->onAuthenticationSucceeded(id, token),
                         "onAuthenticationSucceeded");
}

bool Session::emitAuthenticationFailed() {
    if (!callback_) {
        return false;
    }
    return checkCallback(callback_->onAuthenticationFailed(), "onAuthenticationFailed");
}

bool Session::emitInteractionDetected() {
    if (!callback_) {
        return false;
    }
    return checkCallback(callback_->onInteractionDetected(), "onInteractionDetected");
}

bool Session::emitChallengeGenerated(int64_t challenge) {
    if (!callback_) {
        return false;
    }
    return checkCallback(callback_->onChallengeGenerated(challenge), "onChallengeGenerated");
}

bool Session::emitEnrollmentsEnumerated(const std::vector<int32_t>& ids) {
    if (!callback_) {
        return false;
    }
    return checkCallback(callback_->onEnrollmentsEnumerated(ids), "onEnrollmentsEnumerated");
}

bool Session::emitEnrollmentsRemoved(const std::vector<int32_t>& ids) {
    if (!callback_) {
        return false;
    }
    return checkCallback(callback_->onEnrollmentsRemoved(ids), "onEnrollmentsRemoved");
}

bool Session::emitAuthenticatorId(int64_t id, bool invalidated) {
    if (!callback_) {
        return false;
    }
    if (invalidated) {
        return checkCallback(callback_->onAuthenticatorIdInvalidated(id),
                             "onAuthenticatorIdInvalidated");
    }
    return checkCallback(callback_->onAuthenticatorIdRetrieved(id),
                         "onAuthenticatorIdRetrieved");
}

bool Session::emitSessionClosed() {
    if (!callback_) {
        return false;
    }
    return checkCallback(callback_->onSessionClosed(), "onSessionClosed");
}

}  // namespace km6fod
