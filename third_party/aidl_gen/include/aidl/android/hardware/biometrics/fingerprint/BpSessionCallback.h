/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: C:\Users\Limonjil\Desktop\android\android-sdk\build-tools\35.0.0\aidl.exe --lang=ndk --structured --stability=vintf --min_sdk_version=31 -v 5 -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\keymaster\aidl -h C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\include -o C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\out C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl\android\hardware\biometrics\fingerprint\ISessionCallback.aidl
 */
#pragma once

#include "aidl/android/hardware/biometrics/fingerprint/ISessionCallback.h"

#include <android/binder_ibinder.h>

namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {
class BpSessionCallback : public ::ndk::BpCInterface<ISessionCallback> {
public:
  explicit BpSessionCallback(const ::ndk::SpAIBinder& binder);
  virtual ~BpSessionCallback();

  ::ndk::ScopedAStatus onChallengeGenerated(int64_t in_challenge) override;
  ::ndk::ScopedAStatus onChallengeRevoked(int64_t in_challenge) override;
  ::ndk::ScopedAStatus onAcquired(::aidl::android::hardware::biometrics::fingerprint::AcquiredInfo in_info, int32_t in_vendorCode) override;
  ::ndk::ScopedAStatus onError(::aidl::android::hardware::biometrics::fingerprint::Error in_error, int32_t in_vendorCode) override;
  ::ndk::ScopedAStatus onEnrollmentProgress(int32_t in_enrollmentId, int32_t in_remaining) override;
  ::ndk::ScopedAStatus onAuthenticationSucceeded(int32_t in_enrollmentId, const ::aidl::android::hardware::keymaster::HardwareAuthToken& in_hat) override __attribute__((deprecated("use {@link onAuthenticationSucceededWithResult} instead. Calling this method has the same effect but only allows the enrollmentId and hat to be returned to the framework.")));
  ::ndk::ScopedAStatus onAuthenticationFailed() override;
  ::ndk::ScopedAStatus onLockoutTimed(int64_t in_durationMillis) override;
  ::ndk::ScopedAStatus onLockoutPermanent() override;
  ::ndk::ScopedAStatus onLockoutCleared() override;
  ::ndk::ScopedAStatus onInteractionDetected() override;
  ::ndk::ScopedAStatus onEnrollmentsEnumerated(const std::vector<int32_t>& in_enrollmentIds) override;
  ::ndk::ScopedAStatus onEnrollmentsRemoved(const std::vector<int32_t>& in_enrollmentIds) override;
  ::ndk::ScopedAStatus onAuthenticatorIdRetrieved(int64_t in_authenticatorId) override;
  ::ndk::ScopedAStatus onAuthenticatorIdInvalidated(int64_t in_newAuthenticatorId) override;
  ::ndk::ScopedAStatus onSessionClosed() override;
  ::ndk::ScopedAStatus onAuthenticationSucceededWithResult(const ::aidl::android::hardware::biometrics::fingerprint::AuthenticateSuccess& in_result) override;
  ::ndk::ScopedAStatus getInterfaceVersion(int32_t* _aidl_return) override;
  int32_t _aidl_cached_version = -1;
};
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
