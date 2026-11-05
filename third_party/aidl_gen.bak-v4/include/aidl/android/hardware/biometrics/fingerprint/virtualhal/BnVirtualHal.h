/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/limonjil/android/sdk/build-tools/35.0.0/aidl --lang=ndk --min_sdk_version=31 -o gen -Iflat --header_out=gen/include flat/android/hardware/biometrics/fingerprint/virtualhal/IVirtualHal.aidl
 */
#pragma once

#include "aidl/android/hardware/biometrics/fingerprint/virtualhal/IVirtualHal.h"

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
namespace virtualhal {
class BnVirtualHal : public ::ndk::BnCInterface<IVirtualHal> {
public:
  BnVirtualHal();
  virtual ~BnVirtualHal();
protected:
  ::ndk::SpAIBinder createBinder() override;
private:
};
class IVirtualHalDelegator : public BnVirtualHal {
public:
  explicit IVirtualHalDelegator(const std::shared_ptr<IVirtualHal> &impl) : _impl(impl) {
  }

  ::ndk::ScopedAStatus setEnrollments(const std::vector<int32_t>& in_id) override {
    return _impl->setEnrollments(in_id);
  }
  ::ndk::ScopedAStatus setEnrollmentHit(int32_t in_hit_id) override {
    return _impl->setEnrollmentHit(in_hit_id);
  }
  ::ndk::ScopedAStatus setNextEnrollment(const ::aidl::android::hardware::biometrics::fingerprint::virtualhal::NextEnrollment& in_next_enrollment) override {
    return _impl->setNextEnrollment(in_next_enrollment);
  }
  ::ndk::ScopedAStatus setAuthenticatorId(int64_t in_id) override {
    return _impl->setAuthenticatorId(in_id);
  }
  ::ndk::ScopedAStatus setChallenge(int64_t in_challenge) override {
    return _impl->setChallenge(in_challenge);
  }
  ::ndk::ScopedAStatus setOperationAuthenticateFails(bool in_fail) override {
    return _impl->setOperationAuthenticateFails(in_fail);
  }
  ::ndk::ScopedAStatus setOperationAuthenticateLatency(const std::vector<int32_t>& in_latencyMs) override {
    return _impl->setOperationAuthenticateLatency(in_latencyMs);
  }
  ::ndk::ScopedAStatus setOperationAuthenticateDuration(int32_t in_durationMs) override {
    return _impl->setOperationAuthenticateDuration(in_durationMs);
  }
  ::ndk::ScopedAStatus setOperationAuthenticateError(int32_t in_error) override {
    return _impl->setOperationAuthenticateError(in_error);
  }
  ::ndk::ScopedAStatus setOperationAuthenticateAcquired(const std::vector<::aidl::android::hardware::biometrics::fingerprint::virtualhal::AcquiredInfoAndVendorCode>& in_acquired) override {
    return _impl->setOperationAuthenticateAcquired(in_acquired);
  }
  ::ndk::ScopedAStatus setOperationEnrollError(int32_t in_error) override {
    return _impl->setOperationEnrollError(in_error);
  }
  ::ndk::ScopedAStatus setOperationEnrollLatency(const std::vector<int32_t>& in_latencyMs) override {
    return _impl->setOperationEnrollLatency(in_latencyMs);
  }
  ::ndk::ScopedAStatus setOperationDetectInteractionLatency(const std::vector<int32_t>& in_latencyMs) override {
    return _impl->setOperationDetectInteractionLatency(in_latencyMs);
  }
  ::ndk::ScopedAStatus setOperationDetectInteractionError(int32_t in_error) override {
    return _impl->setOperationDetectInteractionError(in_error);
  }
  ::ndk::ScopedAStatus setOperationDetectInteractionDuration(int32_t in_durationMs) override {
    return _impl->setOperationDetectInteractionDuration(in_durationMs);
  }
  ::ndk::ScopedAStatus setOperationDetectInteractionAcquired(const std::vector<::aidl::android::hardware::biometrics::fingerprint::virtualhal::AcquiredInfoAndVendorCode>& in_acquired) override {
    return _impl->setOperationDetectInteractionAcquired(in_acquired);
  }
  ::ndk::ScopedAStatus setLockout(bool in_lockout) override {
    return _impl->setLockout(in_lockout);
  }
  ::ndk::ScopedAStatus setLockoutEnable(bool in_enable) override {
    return _impl->setLockoutEnable(in_enable);
  }
  ::ndk::ScopedAStatus setLockoutTimedThreshold(int32_t in_threshold) override {
    return _impl->setLockoutTimedThreshold(in_threshold);
  }
  ::ndk::ScopedAStatus setLockoutTimedDuration(int32_t in_durationMs) override {
    return _impl->setLockoutTimedDuration(in_durationMs);
  }
  ::ndk::ScopedAStatus setLockoutPermanentThreshold(int32_t in_threshold) override {
    return _impl->setLockoutPermanentThreshold(in_threshold);
  }
  ::ndk::ScopedAStatus resetConfigurations() override {
    return _impl->resetConfigurations();
  }
  ::ndk::ScopedAStatus setType(::aidl::android::hardware::biometrics::fingerprint::FingerprintSensorType in_type) override {
    return _impl->setType(in_type);
  }
  ::ndk::ScopedAStatus setSensorId(int32_t in_id) override {
    return _impl->setSensorId(in_id);
  }
  ::ndk::ScopedAStatus setSensorStrength(::aidl::android::hardware::biometrics::common::SensorStrength in_strength) override {
    return _impl->setSensorStrength(in_strength);
  }
  ::ndk::ScopedAStatus setMaxEnrollmentPerUser(int32_t in_max) override {
    return _impl->setMaxEnrollmentPerUser(in_max);
  }
  ::ndk::ScopedAStatus setSensorLocation(const ::aidl::android::hardware::biometrics::fingerprint::SensorLocation& in_loc) override {
    return _impl->setSensorLocation(in_loc);
  }
  ::ndk::ScopedAStatus setNavigationGuesture(bool in_v) override {
    return _impl->setNavigationGuesture(in_v);
  }
  ::ndk::ScopedAStatus setDetectInteraction(bool in_v) override {
    return _impl->setDetectInteraction(in_v);
  }
  ::ndk::ScopedAStatus setDisplayTouch(bool in_v) override {
    return _impl->setDisplayTouch(in_v);
  }
  ::ndk::ScopedAStatus setControlIllumination(bool in_v) override {
    return _impl->setControlIllumination(in_v);
  }
  ::ndk::ScopedAStatus getFingerprintHal(std::shared_ptr<::aidl::android::hardware::biometrics::fingerprint::IFingerprint>* _aidl_return) override {
    return _impl->getFingerprintHal(_aidl_return);
  }
protected:
private:
  std::shared_ptr<IVirtualHal> _impl;
};

}  // namespace virtualhal
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
