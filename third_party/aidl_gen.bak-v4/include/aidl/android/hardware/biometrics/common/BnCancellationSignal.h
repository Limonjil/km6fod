/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/limonjil/android/sdk/build-tools/35.0.0/aidl --lang=ndk --min_sdk_version=31 --structured --stability vintf -o gen -Iflat --header_out=gen/include flat/android/hardware/biometrics/common/ICancellationSignal.aidl
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
protected:
  ::ndk::SpAIBinder createBinder() override;
private:
};
class ICancellationSignalDelegator : public BnCancellationSignal {
public:
  explicit ICancellationSignalDelegator(const std::shared_ptr<ICancellationSignal> &impl) : _impl(impl) {
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
