/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/limonjil/android/sdk/build-tools/35.0.0/aidl --lang=ndk --min_sdk_version=31 --structured --stability vintf -o gen -Iflat --header_out=gen/include flat/android/hardware/biometrics/fingerprint/IFingerprint.aidl
 */
#pragma once

#include "aidl/android/hardware/biometrics/fingerprint/IFingerprint.h"

#include <android/binder_ibinder.h>

namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {
class BpFingerprint : public ::ndk::BpCInterface<IFingerprint> {
public:
  explicit BpFingerprint(const ::ndk::SpAIBinder& binder);
  virtual ~BpFingerprint();

  ::ndk::ScopedAStatus getSensorProps(std::vector<::aidl::android::hardware::biometrics::fingerprint::SensorProps>* _aidl_return) override;
  ::ndk::ScopedAStatus createSession(int32_t in_sensorId, int32_t in_userId, const std::shared_ptr<::aidl::android::hardware::biometrics::fingerprint::ISessionCallback>& in_cb, std::shared_ptr<::aidl::android::hardware::biometrics::fingerprint::ISession>* _aidl_return) override;
};
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
