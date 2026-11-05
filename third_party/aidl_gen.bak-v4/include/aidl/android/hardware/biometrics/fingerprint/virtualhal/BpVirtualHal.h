/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/limonjil/android/sdk/build-tools/35.0.0/aidl --lang=ndk --min_sdk_version=31 -o gen -Iflat --header_out=gen/include flat/android/hardware/biometrics/fingerprint/virtualhal/IVirtualHal.aidl
 */
#pragma once

#include "aidl/android/hardware/biometrics/fingerprint/virtualhal/IVirtualHal.h"

#include <android/binder_ibinder.h>

namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {
namespace virtualhal {
class BpVirtualHal : public ::ndk::BpCInterface<IVirtualHal> {
public:
  explicit BpVirtualHal(const ::ndk::SpAIBinder& binder);
  virtual ~BpVirtualHal();

  ::ndk::ScopedAStatus setEnrollments(const std::vector<int32_t>& in_id) override;
  ::ndk::ScopedAStatus setEnrollmentHit(int32_t in_hit_id) override;
  ::ndk::ScopedAStatus setNextEnrollment(const ::aidl::android::hardware::biometrics::fingerprint::virtualhal::NextEnrollment& in_next_enrollment) override;
  ::ndk::ScopedAStatus setAuthenticatorId(int64_t in_id) override;
  ::ndk::ScopedAStatus setChallenge(int64_t in_challenge) override;
  ::ndk::ScopedAStatus setOperationAuthenticateFails(bool in_fail) override;
  ::ndk::ScopedAStatus setOperationAuthenticateLatency(const std::vector<int32_t>& in_latencyMs) override;
  ::ndk::ScopedAStatus setOperationAuthenticateDuration(int32_t in_durationMs) override;
  ::ndk::ScopedAStatus setOperationAuthenticateError(int32_t in_error) override;
  ::ndk::ScopedAStatus setOperationAuthenticateAcquired(const std::vector<::aidl::android::hardware::biometrics::fingerprint::virtualhal::AcquiredInfoAndVendorCode>& in_acquired) override;
  ::ndk::ScopedAStatus setOperationEnrollError(int32_t in_error) override;
  ::ndk::ScopedAStatus setOperationEnrollLatency(const std::vector<int32_t>& in_latencyMs) override;
  ::ndk::ScopedAStatus setOperationDetectInteractionLatency(const std::vector<int32_t>& in_latencyMs) override;
  ::ndk::ScopedAStatus setOperationDetectInteractionError(int32_t in_error) override;
  ::ndk::ScopedAStatus setOperationDetectInteractionDuration(int32_t in_durationMs) override;
  ::ndk::ScopedAStatus setOperationDetectInteractionAcquired(const std::vector<::aidl::android::hardware::biometrics::fingerprint::virtualhal::AcquiredInfoAndVendorCode>& in_acquired) override;
  ::ndk::ScopedAStatus setLockout(bool in_lockout) override;
  ::ndk::ScopedAStatus setLockoutEnable(bool in_enable) override;
  ::ndk::ScopedAStatus setLockoutTimedThreshold(int32_t in_threshold) override;
  ::ndk::ScopedAStatus setLockoutTimedDuration(int32_t in_durationMs) override;
  ::ndk::ScopedAStatus setLockoutPermanentThreshold(int32_t in_threshold) override;
  ::ndk::ScopedAStatus resetConfigurations() override;
  ::ndk::ScopedAStatus setType(::aidl::android::hardware::biometrics::fingerprint::FingerprintSensorType in_type) override;
  ::ndk::ScopedAStatus setSensorId(int32_t in_id) override;
  ::ndk::ScopedAStatus setSensorStrength(::aidl::android::hardware::biometrics::common::SensorStrength in_strength) override;
  ::ndk::ScopedAStatus setMaxEnrollmentPerUser(int32_t in_max) override;
  ::ndk::ScopedAStatus setSensorLocation(const ::aidl::android::hardware::biometrics::fingerprint::SensorLocation& in_loc) override;
  ::ndk::ScopedAStatus setNavigationGuesture(bool in_v) override;
  ::ndk::ScopedAStatus setDetectInteraction(bool in_v) override;
  ::ndk::ScopedAStatus setDisplayTouch(bool in_v) override;
  ::ndk::ScopedAStatus setControlIllumination(bool in_v) override;
  ::ndk::ScopedAStatus getFingerprintHal(std::shared_ptr<::aidl::android::hardware::biometrics::fingerprint::IFingerprint>* _aidl_return) override;
};
}  // namespace virtualhal
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
