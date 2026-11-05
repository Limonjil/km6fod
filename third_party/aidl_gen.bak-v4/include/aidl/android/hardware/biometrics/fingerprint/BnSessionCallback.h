/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/limonjil/android/sdk/build-tools/35.0.0/aidl --lang=ndk --min_sdk_version=31 --structured --stability vintf -o gen -Iflat --header_out=gen/include flat/android/hardware/biometrics/fingerprint/ISessionCallback.aidl
 */
#pragma once

#include "aidl/android/hardware/biometrics/fingerprint/ISessionCallback.h"

#include <android/binder_ibinder.h>
#include <cassert>

#ifndef __BIONIC__
#ifndef __assert2
#define __assert2(a,b,c,d) ((void)0)
#endif
#endif

namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {
class BnSessionCallback : public ::ndk::BnCInterface<ISessionCallback> {
public:
  BnSessionCallback();
  virtual ~BnSessionCallback();
protected:
  ::ndk::SpAIBinder createBinder() override;
private:
};
class ISessionCallbackDelegator : public BnSessionCallback {
public:
  explicit ISessionCallbackDelegator(const std::shared_ptr<ISessionCallback> &impl) : _impl(impl) {
  }

  ::ndk::ScopedAStatus onChallengeGenerated(int64_t in_challenge) override {
    return _impl->onChallengeGenerated(in_challenge);
  }
  ::ndk::ScopedAStatus onChallengeRevoked(int64_t in_challenge) override {
    return _impl->onChallengeRevoked(in_challenge);
  }
  ::ndk::ScopedAStatus onAcquired(::aidl::android::hardware::biometrics::fingerprint::AcquiredInfo in_info, int32_t in_vendorCode) override {
    return _impl->onAcquired(in_info, in_vendorCode);
  }
  ::ndk::ScopedAStatus onError(::aidl::android::hardware::biometrics::fingerprint::Error in_error, int32_t in_vendorCode) override {
    return _impl->onError(in_error, in_vendorCode);
  }
  ::ndk::ScopedAStatus onEnrollmentProgress(int32_t in_enrollmentId, int32_t in_remaining) override {
    return _impl->onEnrollmentProgress(in_enrollmentId, in_remaining);
  }
  ::ndk::ScopedAStatus onAuthenticationSucceeded(int32_t in_enrollmentId, const ::aidl::android::hardware::keymaster::HardwareAuthToken& in_hat) override {
    return _impl->onAuthenticationSucceeded(in_enrollmentId, in_hat);
  }
  ::ndk::ScopedAStatus onAuthenticationFailed() override {
    return _impl->onAuthenticationFailed();
  }
  ::ndk::ScopedAStatus onLockoutTimed(int64_t in_durationMillis) override {
    return _impl->onLockoutTimed(in_durationMillis);
  }
  ::ndk::ScopedAStatus onLockoutPermanent() override {
    return _impl->onLockoutPermanent();
  }
  ::ndk::ScopedAStatus onLockoutCleared() override {
    return _impl->onLockoutCleared();
  }
  ::ndk::ScopedAStatus onInteractionDetected() override {
    return _impl->onInteractionDetected();
  }
  ::ndk::ScopedAStatus onEnrollmentsEnumerated(const std::vector<int32_t>& in_enrollmentIds) override {
    return _impl->onEnrollmentsEnumerated(in_enrollmentIds);
  }
  ::ndk::ScopedAStatus onEnrollmentsRemoved(const std::vector<int32_t>& in_enrollmentIds) override {
    return _impl->onEnrollmentsRemoved(in_enrollmentIds);
  }
  ::ndk::ScopedAStatus onAuthenticatorIdRetrieved(int64_t in_authenticatorId) override {
    return _impl->onAuthenticatorIdRetrieved(in_authenticatorId);
  }
  ::ndk::ScopedAStatus onAuthenticatorIdInvalidated(int64_t in_newAuthenticatorId) override {
    return _impl->onAuthenticatorIdInvalidated(in_newAuthenticatorId);
  }
  ::ndk::ScopedAStatus onSessionClosed() override {
    return _impl->onSessionClosed();
  }
protected:
private:
  std::shared_ptr<ISessionCallback> _impl;
};

}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
