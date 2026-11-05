/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: C:\Users\Limonjil\Desktop\android\android-sdk\build-tools\35.0.0\aidl.exe --lang=ndk --structured --stability=vintf --min_sdk_version=31 -v 5 -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\keymaster\aidl -h C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\include -o C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\out C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl\android\hardware\biometrics\common\ICancellationSignal.aidl
 */
#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <android/binder_interface_utils.h>
#ifdef BINDER_STABILITY_SUPPORT
#include <android/binder_stability.h>
#endif  // BINDER_STABILITY_SUPPORT

namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace common {
class ICancellationSignalDelegator;

class ICancellationSignal : public ::ndk::ICInterface {
public:
  typedef ICancellationSignalDelegator DefaultDelegator;
  static const char* descriptor;
  ICancellationSignal();
  virtual ~ICancellationSignal();

  static inline const int32_t version = 5;
  static constexpr uint32_t TRANSACTION_cancel = FIRST_CALL_TRANSACTION + 0;

  static std::shared_ptr<ICancellationSignal> fromBinder(const ::ndk::SpAIBinder& binder);
  static binder_status_t writeToParcel(AParcel* parcel, const std::shared_ptr<ICancellationSignal>& instance);
  static binder_status_t readFromParcel(const AParcel* parcel, std::shared_ptr<ICancellationSignal>* instance);
  static bool setDefaultImpl(const std::shared_ptr<ICancellationSignal>& impl);
  static const std::shared_ptr<ICancellationSignal>& getDefaultImpl();
  virtual ::ndk::ScopedAStatus cancel() = 0;
  virtual ::ndk::ScopedAStatus getInterfaceVersion(int32_t* _aidl_return) = 0;
private:
  static std::shared_ptr<ICancellationSignal> default_impl;
};
class ICancellationSignalDefault : public ICancellationSignal {
public:
  ::ndk::ScopedAStatus cancel() override;
  ::ndk::ScopedAStatus getInterfaceVersion(int32_t* _aidl_return) override;
  ::ndk::SpAIBinder asBinder() override;
  bool isRemote() override;
};
}  // namespace common
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
