/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/limonjil/android/sdk/build-tools/35.0.0/aidl --lang=ndk --min_sdk_version=31 -o gen -Iflat --header_out=gen/include flat/android/hardware/biometrics/fingerprint/virtualhal/IVirtualHal.aidl
 */
#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <android/binder_interface_utils.h>
#include <aidl/android/hardware/biometrics/common/SensorStrength.h>
#include <aidl/android/hardware/biometrics/fingerprint/FingerprintSensorType.h>
#include <aidl/android/hardware/biometrics/fingerprint/IFingerprint.h>
#include <aidl/android/hardware/biometrics/fingerprint/SensorLocation.h>
#include <aidl/android/hardware/biometrics/fingerprint/virtualhal/AcquiredInfoAndVendorCode.h>
#include <aidl/android/hardware/biometrics/fingerprint/virtualhal/NextEnrollment.h>
#ifdef BINDER_STABILITY_SUPPORT
#include <android/binder_stability.h>
#endif  // BINDER_STABILITY_SUPPORT

namespace aidl::android::hardware::biometrics::fingerprint {
class IFingerprint;
class SensorLocation;
}  // namespace aidl::android::hardware::biometrics::fingerprint
namespace aidl::android::hardware::biometrics::fingerprint::virtualhal {
class NextEnrollment;
}  // namespace aidl::android::hardware::biometrics::fingerprint::virtualhal
namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {
namespace virtualhal {
class IVirtualHalDelegator;

class IVirtualHal : public ::ndk::ICInterface {
public:
  typedef IVirtualHalDelegator DefaultDelegator;
  static const char* descriptor;
  IVirtualHal();
  virtual ~IVirtualHal();

  enum : int32_t { STATUS_INVALID_PARAMETER = 1 };
  static constexpr uint32_t TRANSACTION_setEnrollments = FIRST_CALL_TRANSACTION + 0;
  static constexpr uint32_t TRANSACTION_setEnrollmentHit = FIRST_CALL_TRANSACTION + 1;
  static constexpr uint32_t TRANSACTION_setNextEnrollment = FIRST_CALL_TRANSACTION + 2;
  static constexpr uint32_t TRANSACTION_setAuthenticatorId = FIRST_CALL_TRANSACTION + 3;
  static constexpr uint32_t TRANSACTION_setChallenge = FIRST_CALL_TRANSACTION + 4;
  static constexpr uint32_t TRANSACTION_setOperationAuthenticateFails = FIRST_CALL_TRANSACTION + 5;
  static constexpr uint32_t TRANSACTION_setOperationAuthenticateLatency = FIRST_CALL_TRANSACTION + 6;
  static constexpr uint32_t TRANSACTION_setOperationAuthenticateDuration = FIRST_CALL_TRANSACTION + 7;
  static constexpr uint32_t TRANSACTION_setOperationAuthenticateError = FIRST_CALL_TRANSACTION + 8;
  static constexpr uint32_t TRANSACTION_setOperationAuthenticateAcquired = FIRST_CALL_TRANSACTION + 9;
  static constexpr uint32_t TRANSACTION_setOperationEnrollError = FIRST_CALL_TRANSACTION + 10;
  static constexpr uint32_t TRANSACTION_setOperationEnrollLatency = FIRST_CALL_TRANSACTION + 11;
  static constexpr uint32_t TRANSACTION_setOperationDetectInteractionLatency = FIRST_CALL_TRANSACTION + 12;
  static constexpr uint32_t TRANSACTION_setOperationDetectInteractionError = FIRST_CALL_TRANSACTION + 13;
  static constexpr uint32_t TRANSACTION_setOperationDetectInteractionDuration = FIRST_CALL_TRANSACTION + 14;
  static constexpr uint32_t TRANSACTION_setOperationDetectInteractionAcquired = FIRST_CALL_TRANSACTION + 15;
  static constexpr uint32_t TRANSACTION_setLockout = FIRST_CALL_TRANSACTION + 16;
  static constexpr uint32_t TRANSACTION_setLockoutEnable = FIRST_CALL_TRANSACTION + 17;
  static constexpr uint32_t TRANSACTION_setLockoutTimedThreshold = FIRST_CALL_TRANSACTION + 18;
  static constexpr uint32_t TRANSACTION_setLockoutTimedDuration = FIRST_CALL_TRANSACTION + 19;
  static constexpr uint32_t TRANSACTION_setLockoutPermanentThreshold = FIRST_CALL_TRANSACTION + 20;
  static constexpr uint32_t TRANSACTION_resetConfigurations = FIRST_CALL_TRANSACTION + 21;
  static constexpr uint32_t TRANSACTION_setType = FIRST_CALL_TRANSACTION + 22;
  static constexpr uint32_t TRANSACTION_setSensorId = FIRST_CALL_TRANSACTION + 23;
  static constexpr uint32_t TRANSACTION_setSensorStrength = FIRST_CALL_TRANSACTION + 24;
  static constexpr uint32_t TRANSACTION_setMaxEnrollmentPerUser = FIRST_CALL_TRANSACTION + 25;
  static constexpr uint32_t TRANSACTION_setSensorLocation = FIRST_CALL_TRANSACTION + 26;
  static constexpr uint32_t TRANSACTION_setNavigationGuesture = FIRST_CALL_TRANSACTION + 27;
  static constexpr uint32_t TRANSACTION_setDetectInteraction = FIRST_CALL_TRANSACTION + 28;
  static constexpr uint32_t TRANSACTION_setDisplayTouch = FIRST_CALL_TRANSACTION + 29;
  static constexpr uint32_t TRANSACTION_setControlIllumination = FIRST_CALL_TRANSACTION + 30;
  static constexpr uint32_t TRANSACTION_getFingerprintHal = FIRST_CALL_TRANSACTION + 31;

  static std::shared_ptr<IVirtualHal> fromBinder(const ::ndk::SpAIBinder& binder);
  static binder_status_t writeToParcel(AParcel* parcel, const std::shared_ptr<IVirtualHal>& instance);
  static binder_status_t readFromParcel(const AParcel* parcel, std::shared_ptr<IVirtualHal>* instance);
  static bool setDefaultImpl(const std::shared_ptr<IVirtualHal>& impl);
  static const std::shared_ptr<IVirtualHal>& getDefaultImpl();
  virtual ::ndk::ScopedAStatus setEnrollments(const std::vector<int32_t>& in_id) = 0;
  virtual ::ndk::ScopedAStatus setEnrollmentHit(int32_t in_hit_id) = 0;
  virtual ::ndk::ScopedAStatus setNextEnrollment(const ::aidl::android::hardware::biometrics::fingerprint::virtualhal::NextEnrollment& in_next_enrollment) = 0;
  virtual ::ndk::ScopedAStatus setAuthenticatorId(int64_t in_id) = 0;
  virtual ::ndk::ScopedAStatus setChallenge(int64_t in_challenge) = 0;
  virtual ::ndk::ScopedAStatus setOperationAuthenticateFails(bool in_fail) = 0;
  virtual ::ndk::ScopedAStatus setOperationAuthenticateLatency(const std::vector<int32_t>& in_latencyMs) = 0;
  virtual ::ndk::ScopedAStatus setOperationAuthenticateDuration(int32_t in_durationMs) = 0;
  virtual ::ndk::ScopedAStatus setOperationAuthenticateError(int32_t in_error) = 0;
  virtual ::ndk::ScopedAStatus setOperationAuthenticateAcquired(const std::vector<::aidl::android::hardware::biometrics::fingerprint::virtualhal::AcquiredInfoAndVendorCode>& in_acquired) = 0;
  virtual ::ndk::ScopedAStatus setOperationEnrollError(int32_t in_error) = 0;
  virtual ::ndk::ScopedAStatus setOperationEnrollLatency(const std::vector<int32_t>& in_latencyMs) = 0;
  virtual ::ndk::ScopedAStatus setOperationDetectInteractionLatency(const std::vector<int32_t>& in_latencyMs) = 0;
  virtual ::ndk::ScopedAStatus setOperationDetectInteractionError(int32_t in_error) = 0;
  virtual ::ndk::ScopedAStatus setOperationDetectInteractionDuration(int32_t in_durationMs) = 0;
  virtual ::ndk::ScopedAStatus setOperationDetectInteractionAcquired(const std::vector<::aidl::android::hardware::biometrics::fingerprint::virtualhal::AcquiredInfoAndVendorCode>& in_acquired) = 0;
  virtual ::ndk::ScopedAStatus setLockout(bool in_lockout) = 0;
  virtual ::ndk::ScopedAStatus setLockoutEnable(bool in_enable) = 0;
  virtual ::ndk::ScopedAStatus setLockoutTimedThreshold(int32_t in_threshold) = 0;
  virtual ::ndk::ScopedAStatus setLockoutTimedDuration(int32_t in_durationMs) = 0;
  virtual ::ndk::ScopedAStatus setLockoutPermanentThreshold(int32_t in_threshold) = 0;
  virtual ::ndk::ScopedAStatus resetConfigurations() = 0;
  virtual ::ndk::ScopedAStatus setType(::aidl::android::hardware::biometrics::fingerprint::FingerprintSensorType in_type) = 0;
  virtual ::ndk::ScopedAStatus setSensorId(int32_t in_id) = 0;
  virtual ::ndk::ScopedAStatus setSensorStrength(::aidl::android::hardware::biometrics::common::SensorStrength in_strength) = 0;
  virtual ::ndk::ScopedAStatus setMaxEnrollmentPerUser(int32_t in_max) = 0;
  virtual ::ndk::ScopedAStatus setSensorLocation(const ::aidl::android::hardware::biometrics::fingerprint::SensorLocation& in_loc) = 0;
  virtual ::ndk::ScopedAStatus setNavigationGuesture(bool in_v) = 0;
  virtual ::ndk::ScopedAStatus setDetectInteraction(bool in_v) = 0;
  virtual ::ndk::ScopedAStatus setDisplayTouch(bool in_v) = 0;
  virtual ::ndk::ScopedAStatus setControlIllumination(bool in_v) = 0;
  virtual ::ndk::ScopedAStatus getFingerprintHal(std::shared_ptr<::aidl::android::hardware::biometrics::fingerprint::IFingerprint>* _aidl_return) = 0;
private:
  static std::shared_ptr<IVirtualHal> default_impl;
};
class IVirtualHalDefault : public IVirtualHal {
public:
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
  ::ndk::SpAIBinder asBinder() override;
  bool isRemote() override;
};
}  // namespace virtualhal
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
