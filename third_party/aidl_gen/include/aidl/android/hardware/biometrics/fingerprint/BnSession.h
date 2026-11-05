/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: C:\Users\Limonjil\Desktop\android\android-sdk\build-tools\35.0.0\aidl.exe --lang=ndk --structured --stability=vintf --min_sdk_version=31 -v 5 -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\keymaster\aidl -h C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\include -o C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\out C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl\android\hardware\biometrics\fingerprint\ISession.aidl
 */
#pragma once

#include "aidl/android/hardware/biometrics/fingerprint/ISession.h"

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
class BnSession : public ::ndk::BnCInterface<ISession> {
public:
  BnSession();
  virtual ~BnSession();
  ::ndk::ScopedAStatus getInterfaceVersion(int32_t* _aidl_return) final;
protected:
  ::ndk::SpAIBinder createBinder() override;
private:
};
class ISessionDelegator : public BnSession {
public:
  explicit ISessionDelegator(const std::shared_ptr<ISession> &impl) : _impl(impl) {
     int32_t _impl_ver = 0;
     if (!impl->getInterfaceVersion(&_impl_ver).isOk()) {;
        __assert2(__FILE__, __LINE__, __PRETTY_FUNCTION__, "Delegator failed to get version of the implementation.");
     }
     if (_impl_ver != ISession::version) {
        __assert2(__FILE__, __LINE__, __PRETTY_FUNCTION__, "Mismatched versions of delegator and implementation is not allowed.");
     }
  }

  ::ndk::ScopedAStatus generateChallenge() override {
    return _impl->generateChallenge();
  }
  ::ndk::ScopedAStatus revokeChallenge(int64_t in_challenge) override {
    return _impl->revokeChallenge(in_challenge);
  }
  ::ndk::ScopedAStatus enroll(const ::aidl::android::hardware::keymaster::HardwareAuthToken& in_hat, std::shared_ptr<::aidl::android::hardware::biometrics::common::ICancellationSignal>* _aidl_return) override {
    return _impl->enroll(in_hat, _aidl_return);
  }
  ::ndk::ScopedAStatus authenticate(int64_t in_operationId, std::shared_ptr<::aidl::android::hardware::biometrics::common::ICancellationSignal>* _aidl_return) override {
    return _impl->authenticate(in_operationId, _aidl_return);
  }
  ::ndk::ScopedAStatus detectInteraction(std::shared_ptr<::aidl::android::hardware::biometrics::common::ICancellationSignal>* _aidl_return) override {
    return _impl->detectInteraction(_aidl_return);
  }
  ::ndk::ScopedAStatus enumerateEnrollments() override {
    return _impl->enumerateEnrollments();
  }
  ::ndk::ScopedAStatus removeEnrollments(const std::vector<int32_t>& in_enrollmentIds) override {
    return _impl->removeEnrollments(in_enrollmentIds);
  }
  ::ndk::ScopedAStatus getAuthenticatorId() override {
    return _impl->getAuthenticatorId();
  }
  ::ndk::ScopedAStatus invalidateAuthenticatorId() override {
    return _impl->invalidateAuthenticatorId();
  }
  ::ndk::ScopedAStatus resetLockout(const ::aidl::android::hardware::keymaster::HardwareAuthToken& in_hat) override {
    return _impl->resetLockout(in_hat);
  }
  ::ndk::ScopedAStatus close() override {
    return _impl->close();
  }
  ::ndk::ScopedAStatus onPointerDown(int32_t in_pointerId, int32_t in_x, int32_t in_y, float in_minor, float in_major) override __attribute__((deprecated("use onPointerDownWithContext instead."))) {
    return _impl->onPointerDown(in_pointerId, in_x, in_y, in_minor, in_major);
  }
  ::ndk::ScopedAStatus onPointerUp(int32_t in_pointerId) override __attribute__((deprecated("use onPointerUpWithContext instead."))) {
    return _impl->onPointerUp(in_pointerId);
  }
  ::ndk::ScopedAStatus onUiReady() override {
    return _impl->onUiReady();
  }
  ::ndk::ScopedAStatus authenticateWithContext(int64_t in_operationId, const ::aidl::android::hardware::biometrics::common::OperationContext& in_context, std::shared_ptr<::aidl::android::hardware::biometrics::common::ICancellationSignal>* _aidl_return) override {
    return _impl->authenticateWithContext(in_operationId, in_context, _aidl_return);
  }
  ::ndk::ScopedAStatus enrollWithContext(const ::aidl::android::hardware::keymaster::HardwareAuthToken& in_hat, const ::aidl::android::hardware::biometrics::common::OperationContext& in_context, std::shared_ptr<::aidl::android::hardware::biometrics::common::ICancellationSignal>* _aidl_return) override {
    return _impl->enrollWithContext(in_hat, in_context, _aidl_return);
  }
  ::ndk::ScopedAStatus detectInteractionWithContext(const ::aidl::android::hardware::biometrics::common::OperationContext& in_context, std::shared_ptr<::aidl::android::hardware::biometrics::common::ICancellationSignal>* _aidl_return) override {
    return _impl->detectInteractionWithContext(in_context, _aidl_return);
  }
  ::ndk::ScopedAStatus onPointerDownWithContext(const ::aidl::android::hardware::biometrics::fingerprint::PointerContext& in_context) override {
    return _impl->onPointerDownWithContext(in_context);
  }
  ::ndk::ScopedAStatus onPointerUpWithContext(const ::aidl::android::hardware::biometrics::fingerprint::PointerContext& in_context) override {
    return _impl->onPointerUpWithContext(in_context);
  }
  ::ndk::ScopedAStatus onContextChanged(const ::aidl::android::hardware::biometrics::common::OperationContext& in_context) override {
    return _impl->onContextChanged(in_context);
  }
  ::ndk::ScopedAStatus onPointerCancelWithContext(const ::aidl::android::hardware::biometrics::fingerprint::PointerContext& in_context) override {
    return _impl->onPointerCancelWithContext(in_context);
  }
  ::ndk::ScopedAStatus setIgnoreDisplayTouches(bool in_shouldIgnore) override __attribute__((deprecated("use isHardwareIgnoringTouches in OperationContext from onContextChanged instead"))) {
    return _impl->setIgnoreDisplayTouches(in_shouldIgnore);
  }
protected:
private:
  std::shared_ptr<ISession> _impl;
};

}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
