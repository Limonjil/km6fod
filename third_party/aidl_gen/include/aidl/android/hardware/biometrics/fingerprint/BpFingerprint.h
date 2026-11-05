/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: C:\Users\Limonjil\Desktop\android\android-sdk\build-tools\35.0.0\aidl.exe --lang=ndk --structured --stability=vintf --min_sdk_version=31 -v 5 -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\keymaster\aidl -h C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\include -o C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\out C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl\android\hardware\biometrics\fingerprint\IFingerprint.aidl
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
  ::ndk::ScopedAStatus getInterfaceVersion(int32_t* _aidl_return) override;
  int32_t _aidl_cached_version = -1;
};
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
