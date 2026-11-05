/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/limonjil/android/sdk/build-tools/35.0.0/aidl --lang=ndk --min_sdk_version=31 --structured --stability vintf -o gen -Iflat --header_out=gen/include flat/android/hardware/biometrics/fingerprint/ISessionCallback.aidl
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
  ::ndk::ScopedAStatus onAuthenticationSucceeded(int32_t in_enrollmentId, const ::aidl::android::hardware::keymaster::HardwareAuthToken& in_hat) override;
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
};
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
