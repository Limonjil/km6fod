/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: C:\Users\Limonjil\Desktop\android\android-sdk\build-tools\35.0.0\aidl.exe --lang=ndk --structured --stability=vintf --min_sdk_version=31 -v 5 -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\keymaster\aidl -h C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\include -o C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\out C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl\android\hardware\biometrics\common\ICancellationSignal.aidl
 */
#pragma once

#include "aidl/android/hardware/biometrics/common/ICancellationSignal.h"

#include <android/binder_ibinder.h>

namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace common {
class BpCancellationSignal : public ::ndk::BpCInterface<ICancellationSignal> {
public:
  explicit BpCancellationSignal(const ::ndk::SpAIBinder& binder);
  virtual ~BpCancellationSignal();

  ::ndk::ScopedAStatus cancel() override;
  ::ndk::ScopedAStatus getInterfaceVersion(int32_t* _aidl_return) override;
  int32_t _aidl_cached_version = -1;
};
}  // namespace common
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
