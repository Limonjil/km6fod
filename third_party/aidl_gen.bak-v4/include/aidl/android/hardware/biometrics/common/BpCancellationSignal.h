/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/limonjil/android/sdk/build-tools/35.0.0/aidl --lang=ndk --min_sdk_version=31 --structured --stability vintf -o gen -Iflat --header_out=gen/include flat/android/hardware/biometrics/common/ICancellationSignal.aidl
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
};
}  // namespace common
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
