/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/limonjil/android/sdk/build-tools/35.0.0/aidl --lang=ndk --min_sdk_version=31 --structured --stability vintf -o gen -Iflat --header_out=gen/include flat/android/hardware/biometrics/fingerprint/ISessionCallback.aidl
 */
#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <android/binder_interface_utils.h>
#include <aidl/android/hardware/biometrics/fingerprint/AcquiredInfo.h>
#include <aidl/android/hardware/biometrics/fingerprint/Error.h>
#include <aidl/android/hardware/keymaster/HardwareAuthToken.h>
#ifdef BINDER_STABILITY_SUPPORT
#include <android/binder_stability.h>
#endif  // BINDER_STABILITY_SUPPORT

namespace aidl::android::hardware::keymaster {
class HardwareAuthToken;
}  // namespace aidl::android::hardware::keymaster
namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {
class ISessionCallbackDelegator;

class ISessionCallback : public ::ndk::ICInterface {
public:
  typedef ISessionCallbackDelegator DefaultDelegator;
  static const char* descriptor;
  ISessionCallback();
  virtual ~ISessionCallback();

  static constexpr uint32_t TRANSACTION_onChallengeGenerated = FIRST_CALL_TRANSACTION + 0;
  static constexpr uint32_t TRANSACTION_onChallengeRevoked = FIRST_CALL_TRANSACTION + 1;
  static constexpr uint32_t TRANSACTION_onAcquired = FIRST_CALL_TRANSACTION + 2;
  static constexpr uint32_t TRANSACTION_onError = FIRST_CALL_TRANSACTION + 3;
  static constexpr uint32_t TRANSACTION_onEnrollmentProgress = FIRST_CALL_TRANSACTION + 4;
  static constexpr uint32_t TRANSACTION_onAuthenticationSucceeded = FIRST_CALL_TRANSACTION + 5;
  static constexpr uint32_t TRANSACTION_onAuthenticationFailed = FIRST_CALL_TRANSACTION + 6;
  static constexpr uint32_t TRANSACTION_onLockoutTimed = FIRST_CALL_TRANSACTION + 7;
  static constexpr uint32_t TRANSACTION_onLockoutPermanent = FIRST_CALL_TRANSACTION + 8;
  static constexpr uint32_t TRANSACTION_onLockoutCleared = FIRST_CALL_TRANSACTION + 9;
  static constexpr uint32_t TRANSACTION_onInteractionDetected = FIRST_CALL_TRANSACTION + 10;
  static constexpr uint32_t TRANSACTION_onEnrollmentsEnumerated = FIRST_CALL_TRANSACTION + 11;
  static constexpr uint32_t TRANSACTION_onEnrollmentsRemoved = FIRST_CALL_TRANSACTION + 12;
  static constexpr uint32_t TRANSACTION_onAuthenticatorIdRetrieved = FIRST_CALL_TRANSACTION + 13;
  static constexpr uint32_t TRANSACTION_onAuthenticatorIdInvalidated = FIRST_CALL_TRANSACTION + 14;
  static constexpr uint32_t TRANSACTION_onSessionClosed = FIRST_CALL_TRANSACTION + 15;

  static std::shared_ptr<ISessionCallback> fromBinder(const ::ndk::SpAIBinder& binder);
  static binder_status_t writeToParcel(AParcel* parcel, const std::shared_ptr<ISessionCallback>& instance);
  static binder_status_t readFromParcel(const AParcel* parcel, std::shared_ptr<ISessionCallback>* instance);
  static bool setDefaultImpl(const std::shared_ptr<ISessionCallback>& impl);
  static const std::shared_ptr<ISessionCallback>& getDefaultImpl();
  virtual ::ndk::ScopedAStatus onChallengeGenerated(int64_t in_challenge) = 0;
  virtual ::ndk::ScopedAStatus onChallengeRevoked(int64_t in_challenge) = 0;
  virtual ::ndk::ScopedAStatus onAcquired(::aidl::android::hardware::biometrics::fingerprint::AcquiredInfo in_info, int32_t in_vendorCode) = 0;
  virtual ::ndk::ScopedAStatus onError(::aidl::android::hardware::biometrics::fingerprint::Error in_error, int32_t in_vendorCode) = 0;
  virtual ::ndk::ScopedAStatus onEnrollmentProgress(int32_t in_enrollmentId, int32_t in_remaining) = 0;
  virtual ::ndk::ScopedAStatus onAuthenticationSucceeded(int32_t in_enrollmentId, const ::aidl::android::hardware::keymaster::HardwareAuthToken& in_hat) = 0;
  virtual ::ndk::ScopedAStatus onAuthenticationFailed() = 0;
  virtual ::ndk::ScopedAStatus onLockoutTimed(int64_t in_durationMillis) = 0;
  virtual ::ndk::ScopedAStatus onLockoutPermanent() = 0;
  virtual ::ndk::ScopedAStatus onLockoutCleared() = 0;
  virtual ::ndk::ScopedAStatus onInteractionDetected() = 0;
  virtual ::ndk::ScopedAStatus onEnrollmentsEnumerated(const std::vector<int32_t>& in_enrollmentIds) = 0;
  virtual ::ndk::ScopedAStatus onEnrollmentsRemoved(const std::vector<int32_t>& in_enrollmentIds) = 0;
  virtual ::ndk::ScopedAStatus onAuthenticatorIdRetrieved(int64_t in_authenticatorId) = 0;
  virtual ::ndk::ScopedAStatus onAuthenticatorIdInvalidated(int64_t in_newAuthenticatorId) = 0;
  virtual ::ndk::ScopedAStatus onSessionClosed() = 0;
private:
  static std::shared_ptr<ISessionCallback> default_impl;
};
class ISessionCallbackDefault : public ISessionCallback {
public:
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
  ::ndk::SpAIBinder asBinder() override;
  bool isRemote() override;
};
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
