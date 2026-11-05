/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/limonjil/android/sdk/build-tools/35.0.0/aidl --lang=ndk --min_sdk_version=31 --structured --stability vintf -o gen -Iflat --header_out=gen/include flat/android/hardware/biometrics/fingerprint/ISession.aidl
 */
#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <android/binder_interface_utils.h>
#include <aidl/android/hardware/biometrics/common/ICancellationSignal.h>
#include <aidl/android/hardware/biometrics/common/OperationContext.h>
#include <aidl/android/hardware/biometrics/fingerprint/PointerContext.h>
#include <aidl/android/hardware/keymaster/HardwareAuthToken.h>
#ifdef BINDER_STABILITY_SUPPORT
#include <android/binder_stability.h>
#endif  // BINDER_STABILITY_SUPPORT

namespace aidl::android::hardware::biometrics::common {
class ICancellationSignal;
class OperationContext;
}  // namespace aidl::android::hardware::biometrics::common
namespace aidl::android::hardware::biometrics::fingerprint {
class PointerContext;
}  // namespace aidl::android::hardware::biometrics::fingerprint
namespace aidl::android::hardware::keymaster {
class HardwareAuthToken;
}  // namespace aidl::android::hardware::keymaster
namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {
class ISessionDelegator;

class ISession : public ::ndk::ICInterface {
public:
  typedef ISessionDelegator DefaultDelegator;
  static const char* descriptor;
  ISession();
  virtual ~ISession();

  static constexpr uint32_t TRANSACTION_generateChallenge = FIRST_CALL_TRANSACTION + 0;
  static constexpr uint32_t TRANSACTION_revokeChallenge = FIRST_CALL_TRANSACTION + 1;
  static constexpr uint32_t TRANSACTION_enroll = FIRST_CALL_TRANSACTION + 2;
  static constexpr uint32_t TRANSACTION_authenticate = FIRST_CALL_TRANSACTION + 3;
  static constexpr uint32_t TRANSACTION_detectInteraction = FIRST_CALL_TRANSACTION + 4;
  static constexpr uint32_t TRANSACTION_enumerateEnrollments = FIRST_CALL_TRANSACTION + 5;
  static constexpr uint32_t TRANSACTION_removeEnrollments = FIRST_CALL_TRANSACTION + 6;
  static constexpr uint32_t TRANSACTION_getAuthenticatorId = FIRST_CALL_TRANSACTION + 7;
  static constexpr uint32_t TRANSACTION_invalidateAuthenticatorId = FIRST_CALL_TRANSACTION + 8;
  static constexpr uint32_t TRANSACTION_resetLockout = FIRST_CALL_TRANSACTION + 9;
  static constexpr uint32_t TRANSACTION_close = FIRST_CALL_TRANSACTION + 10;
  static constexpr uint32_t TRANSACTION_onPointerDown = FIRST_CALL_TRANSACTION + 11;
  static constexpr uint32_t TRANSACTION_onPointerUp = FIRST_CALL_TRANSACTION + 12;
  static constexpr uint32_t TRANSACTION_onUiReady = FIRST_CALL_TRANSACTION + 13;
  static constexpr uint32_t TRANSACTION_authenticateWithContext = FIRST_CALL_TRANSACTION + 14;
  static constexpr uint32_t TRANSACTION_enrollWithContext = FIRST_CALL_TRANSACTION + 15;
  static constexpr uint32_t TRANSACTION_detectInteractionWithContext = FIRST_CALL_TRANSACTION + 16;
  static constexpr uint32_t TRANSACTION_onPointerDownWithContext = FIRST_CALL_TRANSACTION + 17;
  static constexpr uint32_t TRANSACTION_onPointerUpWithContext = FIRST_CALL_TRANSACTION + 18;
  static constexpr uint32_t TRANSACTION_onContextChanged = FIRST_CALL_TRANSACTION + 19;
  static constexpr uint32_t TRANSACTION_onPointerCancelWithContext = FIRST_CALL_TRANSACTION + 20;
  static constexpr uint32_t TRANSACTION_setIgnoreDisplayTouches = FIRST_CALL_TRANSACTION + 21;

  static std::shared_ptr<ISession> fromBinder(const ::ndk::SpAIBinder& binder);
  static binder_status_t writeToParcel(AParcel* parcel, const std::shared_ptr<ISession>& instance);
  static binder_status_t readFromParcel(const AParcel* parcel, std::shared_ptr<ISession>* instance);
  static bool setDefaultImpl(const std::shared_ptr<ISession>& impl);
  static const std::shared_ptr<ISession>& getDefaultImpl();
  virtual ::ndk::ScopedAStatus generateChallenge() = 0;
  virtual ::ndk::ScopedAStatus revokeChallenge(int64_t in_challenge) = 0;
  virtual ::ndk::ScopedAStatus enroll(const ::aidl::android::hardware::keymaster::HardwareAuthToken& in_hat, std::shared_ptr<::aidl::android::hardware::biometrics::common::ICancellationSignal>* _aidl_return) = 0;
  virtual ::ndk::ScopedAStatus authenticate(int64_t in_operationId, std::shared_ptr<::aidl::android::hardware::biometrics::common::ICancellationSignal>* _aidl_return) = 0;
  virtual ::ndk::ScopedAStatus detectInteraction(std::shared_ptr<::aidl::android::hardware::biometrics::common::ICancellationSignal>* _aidl_return) = 0;
  virtual ::ndk::ScopedAStatus enumerateEnrollments() = 0;
  virtual ::ndk::ScopedAStatus removeEnrollments(const std::vector<int32_t>& in_enrollmentIds) = 0;
  virtual ::ndk::ScopedAStatus getAuthenticatorId() = 0;
  virtual ::ndk::ScopedAStatus invalidateAuthenticatorId() = 0;
  virtual ::ndk::ScopedAStatus resetLockout(const ::aidl::android::hardware::keymaster::HardwareAuthToken& in_hat) = 0;
  virtual ::ndk::ScopedAStatus close() = 0;
  virtual ::ndk::ScopedAStatus onPointerDown(int32_t in_pointerId, int32_t in_x, int32_t in_y, float in_minor, float in_major) __attribute__((deprecated("use onPointerDownWithContext instead."))) = 0;
  virtual ::ndk::ScopedAStatus onPointerUp(int32_t in_pointerId) __attribute__((deprecated("use onPointerUpWithContext instead."))) = 0;
  virtual ::ndk::ScopedAStatus onUiReady() = 0;
  virtual ::ndk::ScopedAStatus authenticateWithContext(int64_t in_operationId, const ::aidl::android::hardware::biometrics::common::OperationContext& in_context, std::shared_ptr<::aidl::android::hardware::biometrics::common::ICancellationSignal>* _aidl_return) = 0;
  virtual ::ndk::ScopedAStatus enrollWithContext(const ::aidl::android::hardware::keymaster::HardwareAuthToken& in_hat, const ::aidl::android::hardware::biometrics::common::OperationContext& in_context, std::shared_ptr<::aidl::android::hardware::biometrics::common::ICancellationSignal>* _aidl_return) = 0;
  virtual ::ndk::ScopedAStatus detectInteractionWithContext(const ::aidl::android::hardware::biometrics::common::OperationContext& in_context, std::shared_ptr<::aidl::android::hardware::biometrics::common::ICancellationSignal>* _aidl_return) = 0;
  virtual ::ndk::ScopedAStatus onPointerDownWithContext(const ::aidl::android::hardware::biometrics::fingerprint::PointerContext& in_context) = 0;
  virtual ::ndk::ScopedAStatus onPointerUpWithContext(const ::aidl::android::hardware::biometrics::fingerprint::PointerContext& in_context) = 0;
  virtual ::ndk::ScopedAStatus onContextChanged(const ::aidl::android::hardware::biometrics::common::OperationContext& in_context) = 0;
  virtual ::ndk::ScopedAStatus onPointerCancelWithContext(const ::aidl::android::hardware::biometrics::fingerprint::PointerContext& in_context) = 0;
  virtual ::ndk::ScopedAStatus setIgnoreDisplayTouches(bool in_shouldIgnore) __attribute__((deprecated("use isHardwareIgnoringTouches in OperationContext from onContextChanged instead"))) = 0;
private:
  static std::shared_ptr<ISession> default_impl;
};
class ISessionDefault : public ISession {
public:
  ::ndk::ScopedAStatus generateChallenge() override;
  ::ndk::ScopedAStatus revokeChallenge(int64_t in_challenge) override;
  ::ndk::ScopedAStatus enroll(const ::aidl::android::hardware::keymaster::HardwareAuthToken& in_hat, std::shared_ptr<::aidl::android::hardware::biometrics::common::ICancellationSignal>* _aidl_return) override;
  ::ndk::ScopedAStatus authenticate(int64_t in_operationId, std::shared_ptr<::aidl::android::hardware::biometrics::common::ICancellationSignal>* _aidl_return) override;
  ::ndk::ScopedAStatus detectInteraction(std::shared_ptr<::aidl::android::hardware::biometrics::common::ICancellationSignal>* _aidl_return) override;
  ::ndk::ScopedAStatus enumerateEnrollments() override;
  ::ndk::ScopedAStatus removeEnrollments(const std::vector<int32_t>& in_enrollmentIds) override;
  ::ndk::ScopedAStatus getAuthenticatorId() override;
  ::ndk::ScopedAStatus invalidateAuthenticatorId() override;
  ::ndk::ScopedAStatus resetLockout(const ::aidl::android::hardware::keymaster::HardwareAuthToken& in_hat) override;
  ::ndk::ScopedAStatus close() override;
  ::ndk::ScopedAStatus onPointerDown(int32_t in_pointerId, int32_t in_x, int32_t in_y, float in_minor, float in_major) override __attribute__((deprecated("use onPointerDownWithContext instead.")));
  ::ndk::ScopedAStatus onPointerUp(int32_t in_pointerId) override __attribute__((deprecated("use onPointerUpWithContext instead.")));
  ::ndk::ScopedAStatus onUiReady() override;
  ::ndk::ScopedAStatus authenticateWithContext(int64_t in_operationId, const ::aidl::android::hardware::biometrics::common::OperationContext& in_context, std::shared_ptr<::aidl::android::hardware::biometrics::common::ICancellationSignal>* _aidl_return) override;
  ::ndk::ScopedAStatus enrollWithContext(const ::aidl::android::hardware::keymaster::HardwareAuthToken& in_hat, const ::aidl::android::hardware::biometrics::common::OperationContext& in_context, std::shared_ptr<::aidl::android::hardware::biometrics::common::ICancellationSignal>* _aidl_return) override;
  ::ndk::ScopedAStatus detectInteractionWithContext(const ::aidl::android::hardware::biometrics::common::OperationContext& in_context, std::shared_ptr<::aidl::android::hardware::biometrics::common::ICancellationSignal>* _aidl_return) override;
  ::ndk::ScopedAStatus onPointerDownWithContext(const ::aidl::android::hardware::biometrics::fingerprint::PointerContext& in_context) override;
  ::ndk::ScopedAStatus onPointerUpWithContext(const ::aidl::android::hardware::biometrics::fingerprint::PointerContext& in_context) override;
  ::ndk::ScopedAStatus onContextChanged(const ::aidl::android::hardware::biometrics::common::OperationContext& in_context) override;
  ::ndk::ScopedAStatus onPointerCancelWithContext(const ::aidl::android::hardware::biometrics::fingerprint::PointerContext& in_context) override;
  ::ndk::ScopedAStatus setIgnoreDisplayTouches(bool in_shouldIgnore) override __attribute__((deprecated("use isHardwareIgnoringTouches in OperationContext from onContextChanged instead")));
  ::ndk::SpAIBinder asBinder() override;
  bool isRemote() override;
};
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
