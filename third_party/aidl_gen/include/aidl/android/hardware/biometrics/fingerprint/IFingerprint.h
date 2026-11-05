/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: C:\Users\Limonjil\Desktop\android\android-sdk\build-tools\35.0.0\aidl.exe --lang=ndk --structured --stability=vintf --min_sdk_version=31 -v 5 -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\keymaster\aidl -h C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\include -o C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\out C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl\android\hardware\biometrics\fingerprint\IFingerprint.aidl
 */
#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <android/binder_interface_utils.h>
#include <aidl/android/hardware/biometrics/fingerprint/ISession.h>
#include <aidl/android/hardware/biometrics/fingerprint/ISessionCallback.h>
#include <aidl/android/hardware/biometrics/fingerprint/SensorProps.h>
#ifdef BINDER_STABILITY_SUPPORT
#include <android/binder_stability.h>
#endif  // BINDER_STABILITY_SUPPORT

namespace aidl::android::hardware::biometrics::fingerprint {
class ISession;
class ISessionCallback;
class SensorProps;
}  // namespace aidl::android::hardware::biometrics::fingerprint
namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {
class IFingerprintDelegator;

class IFingerprint : public ::ndk::ICInterface {
public:
  typedef IFingerprintDelegator DefaultDelegator;
  static const char* descriptor;
  IFingerprint();
  virtual ~IFingerprint();

  static inline const int32_t version = 5;
  static constexpr uint32_t TRANSACTION_getSensorProps = FIRST_CALL_TRANSACTION + 0;
  static constexpr uint32_t TRANSACTION_createSession = FIRST_CALL_TRANSACTION + 1;

  static std::shared_ptr<IFingerprint> fromBinder(const ::ndk::SpAIBinder& binder);
  static binder_status_t writeToParcel(AParcel* parcel, const std::shared_ptr<IFingerprint>& instance);
  static binder_status_t readFromParcel(const AParcel* parcel, std::shared_ptr<IFingerprint>* instance);
  static bool setDefaultImpl(const std::shared_ptr<IFingerprint>& impl);
  static const std::shared_ptr<IFingerprint>& getDefaultImpl();
  virtual ::ndk::ScopedAStatus getSensorProps(std::vector<::aidl::android::hardware::biometrics::fingerprint::SensorProps>* _aidl_return) = 0;
  virtual ::ndk::ScopedAStatus createSession(int32_t in_sensorId, int32_t in_userId, const std::shared_ptr<::aidl::android::hardware::biometrics::fingerprint::ISessionCallback>& in_cb, std::shared_ptr<::aidl::android::hardware::biometrics::fingerprint::ISession>* _aidl_return) = 0;
  virtual ::ndk::ScopedAStatus getInterfaceVersion(int32_t* _aidl_return) = 0;
private:
  static std::shared_ptr<IFingerprint> default_impl;
};
class IFingerprintDefault : public IFingerprint {
public:
  ::ndk::ScopedAStatus getSensorProps(std::vector<::aidl::android::hardware::biometrics::fingerprint::SensorProps>* _aidl_return) override;
  ::ndk::ScopedAStatus createSession(int32_t in_sensorId, int32_t in_userId, const std::shared_ptr<::aidl::android::hardware::biometrics::fingerprint::ISessionCallback>& in_cb, std::shared_ptr<::aidl::android::hardware::biometrics::fingerprint::ISession>* _aidl_return) override;
  ::ndk::ScopedAStatus getInterfaceVersion(int32_t* _aidl_return) override;
  ::ndk::SpAIBinder asBinder() override;
  bool isRemote() override;
};
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
