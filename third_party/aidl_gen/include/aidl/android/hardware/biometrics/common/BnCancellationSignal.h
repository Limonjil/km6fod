/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: C:\Users\Limonjil\Desktop\android\android-sdk\build-tools\35.0.0\aidl.exe --lang=ndk --structured --stability=vintf --min_sdk_version=31 -v 5 -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\keymaster\aidl -h C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\include -o C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\out C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl\android\hardware\biometrics\common\ICancellationSignal.aidl
 */
#pragma once

#include "aidl/android/hardware/biometrics/common/ICancellationSignal.h"

#include <android/binder_ibinder.h>
#include <cassert>

#ifndef __BIONIC__
#ifndef __assert2
#define __assert2(a,b,c,d) ((void)0)
#endif
#endif

namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace common {
class BnCancellationSignal : public ::ndk::BnCInterface<ICancellationSignal> {
public:
  BnCancellationSignal();
  virtual ~BnCancellationSignal();
  ::ndk::ScopedAStatus getInterfaceVersion(int32_t* _aidl_return) final;
protected:
  ::ndk::SpAIBinder createBinder() override;
private:
};
class ICancellationSignalDelegator : public BnCancellationSignal {
public:
  explicit ICancellationSignalDelegator(const std::shared_ptr<ICancellationSignal> &impl) : _impl(impl) {
     int32_t _impl_ver = 0;
     if (!impl->getInterfaceVersion(&_impl_ver).isOk()) {;
        __assert2(__FILE__, __LINE__, __PRETTY_FUNCTION__, "Delegator failed to get version of the implementation.");
     }
     if (_impl_ver != ICancellationSignal::version) {
        __assert2(__FILE__, __LINE__, __PRETTY_FUNCTION__, "Mismatched versions of delegator and implementation is not allowed.");
     }
  }

  ::ndk::ScopedAStatus cancel() override {
    return _impl->cancel();
  }
protected:
private:
  std::shared_ptr<ICancellationSignal> _impl;
};

}  // namespace common
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
