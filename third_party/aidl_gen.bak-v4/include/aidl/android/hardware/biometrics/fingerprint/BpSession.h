/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/limonjil/android/sdk/build-tools/35.0.0/aidl --lang=ndk --min_sdk_version=31 --structured --stability vintf -o gen -Iflat --header_out=gen/include flat/android/hardware/biometrics/fingerprint/ISession.aidl
 */
#pragma once

#include "aidl/android/hardware/biometrics/fingerprint/ISession.h"

#include <android/binder_ibinder.h>

namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {
class BpSession : public ::ndk::BpCInterface<ISession> {
public:
  explicit BpSession(const ::ndk::SpAIBinder& binder);
  virtual ~BpSession();

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
};
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
