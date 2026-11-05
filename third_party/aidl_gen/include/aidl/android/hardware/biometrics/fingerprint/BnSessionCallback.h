/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: C:\Users\Limonjil\Desktop\android\android-sdk\build-tools\35.0.0\aidl.exe --lang=ndk --structured --stability=vintf --min_sdk_version=31 -v 5 -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\keymaster\aidl -h C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\include -o C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\out C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl\android\hardware\biometrics\fingerprint\ISessionCallback.aidl
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
  ::ndk::ScopedAStatus getInterfaceVersion(int32_t* _aidl_return) final;
protected:
  ::ndk::SpAIBinder createBinder() override;
private:
};
class ISessionCallbackDelegator : public BnSessionCallback {
public:
  explicit ISessionCallbackDelegator(const std::shared_ptr<ISessionCallback> &impl) : _impl(impl) {
     int32_t _impl_ver = 0;
     if (!impl->getInterfaceVersion(&_impl_ver).isOk()) {;
        __assert2(__FILE__, __LINE__, __PRETTY_FUNCTION__, "Delegator failed to get version of the implementation.");
     }
     if (_impl_ver != ISessionCallback::version) {
        __assert2(__FILE__, __LINE__, __PRETTY_FUNCTION__, "Mismatched versions of delegator and implementation is not allowed.");
     }
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
  ::ndk::ScopedAStatus onAuthenticationSucceeded(int32_t in_enrollmentId, const ::aidl::android::hardware::keymaster::HardwareAuthToken& in_hat) override __attribute__((deprecated("use {@link onAuthenticationSucceededWithResult} instead. Calling this method has the same effect but only allows the enrollmentId and hat to be returned to the framework."))) {
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
  ::ndk::ScopedAStatus onAuthenticationSucceededWithResult(const ::aidl::android::hardware::biometrics::fingerprint::AuthenticateSuccess& in_result) override {
    return _impl->onAuthenticationSucceededWithResult(in_result);
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
