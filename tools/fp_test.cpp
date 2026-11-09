// SPDX-License-Identifier: GPL-3.0-only
//
// Diagnostic AIDL client for the KM6 FOD HAL.
//
// Run only while the HAL is being tested: it creates a real session and issues
// a real authenticate() (which opens HBM). It is not part of the Magisk module.
//
//   adb push build/fp_test /data/local/tmp/
//   adb shell su -c /data/local/tmp/fp_test

#include <android/binder_ibinder.h>
#include <android/binder_interface_utils.h>
#include <android/binder_status.h>
#include <unistd.h>

#include <cstdio>
#include <memory>
#include <vector>

#include "aidl/android/hardware/biometrics/fingerprint/IFingerprint.h"
#include "aidl/android/hardware/biometrics/fingerprint/ISession.h"
#include "aidl/android/hardware/biometrics/fingerprint/ISessionCallback.h"
#include "aidl/android/hardware/biometrics/fingerprint/BnSessionCallback.h"

extern "C" {
AIBinder* AServiceManager_waitForService(const char* instance);
void ABinderProcess_setThreadPoolMaxThreadCount(uint32_t num_threads);
void ABinderProcess_startThreadPool(void);
}

namespace {

namespace fp = aidl::android::hardware::biometrics::fingerprint;
namespace bc = aidl::android::hardware::biometrics::common;

constexpr const char* kInstance =
    "android.hardware.biometrics.fingerprint.IFingerprint/default";

class TestCallback : public fp::BnSessionCallback {
  public:
    ::ndk::ScopedAStatus onChallengeGenerated(int64_t challenge) override {
        std::printf("callback: onChallengeGenerated(%lld)\n", static_cast<long long>(challenge));
        return ::ndk::ScopedAStatus::ok();
    }
    ::ndk::ScopedAStatus onChallengeRevoked(int64_t challenge) override {
        std::printf("callback: onChallengeRevoked(%lld)\n", static_cast<long long>(challenge));
        return ::ndk::ScopedAStatus::ok();
    }
    ::ndk::ScopedAStatus onAcquired(fp::AcquiredInfo info, int32_t vendor) override {
        std::printf("callback: onAcquired(%d, %d)\n", static_cast<int>(info), vendor);
        return ::ndk::ScopedAStatus::ok();
    }
    ::ndk::ScopedAStatus onError(fp::Error error, int32_t vendor) override {
        std::printf("callback: onError(%d, %d)\n", static_cast<int>(error), vendor);
        return ::ndk::ScopedAStatus::ok();
    }
    ::ndk::ScopedAStatus onEnrollmentProgress(int32_t id, int32_t remaining) override {
        std::printf("callback: onEnrollmentProgress(%d, %d)\n", id, remaining);
        return ::ndk::ScopedAStatus::ok();
    }
    ::ndk::ScopedAStatus onAuthenticationSucceeded(
        int32_t id, const aidl::android::hardware::keymaster::HardwareAuthToken& hat) override {
        std::printf("callback: onAuthenticationSucceeded(%d, mac=%zu)\n", id, hat.mac.size());
        return ::ndk::ScopedAStatus::ok();
    }
    ::ndk::ScopedAStatus onAuthenticationFailed() override {
        std::printf("callback: onAuthenticationFailed()\n");
        return ::ndk::ScopedAStatus::ok();
    }
    ::ndk::ScopedAStatus onLockoutTimed(int64_t duration) override {
        std::printf("callback: onLockoutTimed(%lld)\n", static_cast<long long>(duration));
        return ::ndk::ScopedAStatus::ok();
    }
    ::ndk::ScopedAStatus onLockoutPermanent() override {
        std::printf("callback: onLockoutPermanent()\n");
        return ::ndk::ScopedAStatus::ok();
    }
    ::ndk::ScopedAStatus onLockoutCleared() override {
        std::printf("callback: onLockoutCleared()\n");
        return ::ndk::ScopedAStatus::ok();
    }
    ::ndk::ScopedAStatus onInteractionDetected() override {
        std::printf("callback: onInteractionDetected()\n");
        return ::ndk::ScopedAStatus::ok();
    }
    ::ndk::ScopedAStatus onEnrollmentsEnumerated(
        const std::vector<int32_t>& ids) override {
        std::printf("callback: onEnrollmentsEnumerated(%zu):", ids.size());
        for (int32_t id : ids) {
            std::printf(" %d", id);
        }
        std::printf("\n");
        return ::ndk::ScopedAStatus::ok();
    }
    ::ndk::ScopedAStatus onEnrollmentsRemoved(const std::vector<int32_t>& ids) override {
        std::printf("callback: onEnrollmentsRemoved(%zu)\n", ids.size());
        return ::ndk::ScopedAStatus::ok();
    }
    ::ndk::ScopedAStatus onAuthenticatorIdRetrieved(int64_t id) override {
        std::printf("callback: onAuthenticatorIdRetrieved(%lld)\n",
                    static_cast<long long>(id));
        return ::ndk::ScopedAStatus::ok();
    }
    ::ndk::ScopedAStatus onAuthenticatorIdInvalidated(int64_t id) override {
        std::printf("callback: onAuthenticatorIdInvalidated(%lld)\n",
                    static_cast<long long>(id));
        return ::ndk::ScopedAStatus::ok();
    }
    ::ndk::ScopedAStatus onSessionClosed() override {
        std::printf("callback: onSessionClosed()\n");
        return ::ndk::ScopedAStatus::ok();
    }
};

}  // namespace

int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(2);
    ABinderProcess_startThreadPool();

    AIBinder* raw = AServiceManager_waitForService(kInstance);
    if (raw == nullptr) {
        std::printf("client: service %s not found\n", kInstance);
        return 2;
    }
    auto fingerprint = fp::IFingerprint::fromBinder(::ndk::SpAIBinder(raw));
    if (!fingerprint) {
        std::printf("client: fromBinder failed\n");
        return 2;
    }

    std::vector<fp::SensorProps> props;
    auto status = fingerprint->getSensorProps(&props);
    std::printf("client: getSensorProps status=%d count=%zu\n", static_cast<int>(status.getStatus()),
                props.size());
    for (const auto& prop : props) {
        std::printf("  sensor=%d strength=%d max=%d type=%d locations=%zu\n",
                    prop.commonProps.sensorId, static_cast<int>(prop.commonProps.sensorStrength),
                    prop.commonProps.maxEnrollmentsPerUser, static_cast<int>(prop.sensorType),
                    prop.sensorLocations.size());
        for (const auto& location : prop.sensorLocations) {
            std::printf("  location display='%s' x=%d y=%d r=%d shape=%d\n",
                        location.display.c_str(), location.sensorLocationX,
                        location.sensorLocationY, location.sensorRadius,
                        static_cast<int>(location.sensorShape));
        }
    }

    auto callback = ::ndk::SharedRefBase::make<TestCallback>();
    std::shared_ptr<fp::ISession> session;
    status = fingerprint->createSession(0, 0, callback, &session);
    std::printf("client: createSession status=%d session=%p\n",
                static_cast<int>(status.getStatus()), session.get());
    if (!session) {
        return 3;
    }

    status = session->generateChallenge();
    std::printf("client: generateChallenge status=%d\n", static_cast<int>(status.getStatus()));
    ::usleep(1200 * 1000);

    status = session->getAuthenticatorId();
    std::printf("client: getAuthenticatorId status=%d\n", static_cast<int>(status.getStatus()));
    ::usleep(300 * 1000);

    status = session->enumerateEnrollments();
    std::printf("client: enumerateEnrollments status=%d\n", static_cast<int>(status.getStatus()));
    ::usleep(1500 * 1000);

    std::shared_ptr<bc::ICancellationSignal> cancellation;
    status = session->authenticate(1, &cancellation);
    std::printf("client: authenticate status=%d cancellation=%p\n",
                static_cast<int>(status.getStatus()), cancellation.get());
    ::usleep(2500 * 1000);

    if (cancellation) {
        status = cancellation->cancel();
        std::printf("client: cancel status=%d\n", static_cast<int>(status.getStatus()));
    }
    ::usleep(800 * 1000);

    status = session->close();
    std::printf("client: close status=%d\n", static_cast<int>(status.getStatus()));
    ::usleep(500 * 1000);
    return 0;
}
