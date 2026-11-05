/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/limonjil/android/sdk/build-tools/35.0.0/aidl --lang=ndk --min_sdk_version=31 --structured --stability vintf -o gen -Iflat --header_out=gen/include flat/android/hardware/biometrics/fingerprint/IFingerprint.aidl
 */
#pragma once

#include "aidl/android/hardware/biometrics/fingerprint/IFingerprint.h"

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
class BnFingerprint : public ::ndk::BnCInterface<IFingerprint> {
public:
  BnFingerprint();
  virtual ~BnFingerprint();
protected:
  ::ndk::SpAIBinder createBinder() override;
private:
};
class IFingerprintDelegator : public BnFingerprint {
public:
  explicit IFingerprintDelegator(const std::shared_ptr<IFingerprint> &impl) : _impl(impl) {
  }

  ::ndk::ScopedAStatus getSensorProps(std::vector<::aidl::android::hardware::biometrics::fingerprint::SensorProps>* _aidl_return) override {
    return _impl->getSensorProps(_aidl_return);
  }
  ::ndk::ScopedAStatus createSession(int32_t in_sensorId, int32_t in_userId, const std::shared_ptr<::aidl::android::hardware::biometrics::fingerprint::ISessionCallback>& in_cb, std::shared_ptr<::aidl::android::hardware::biometrics::fingerprint::ISession>* _aidl_return) override {
    return _impl->createSession(in_sensorId, in_userId, in_cb, _aidl_return);
  }
protected:
private:
  std::shared_ptr<IFingerprint> _impl;
};

}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
