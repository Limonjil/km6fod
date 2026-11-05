/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/limonjil/android/sdk/build-tools/35.0.0/aidl --lang=ndk --min_sdk_version=31 -o gen -Iflat --header_out=gen/include flat/android/hardware/biometrics/fingerprint/virtualhal/AcquiredInfoAndVendorCode.aidl
 */
#include "aidl/android/hardware/biometrics/fingerprint/virtualhal/AcquiredInfoAndVendorCode.h"

#include <android/binder_parcel_utils.h>

namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {
namespace virtualhal {
const char* AcquiredInfoAndVendorCode::descriptor = "android.hardware.biometrics.fingerprint.virtualhal.AcquiredInfoAndVendorCode";

binder_status_t AcquiredInfoAndVendorCode::readFromParcel(const AParcel* _parcel) {
  binder_status_t _aidl_ret_status;
  int32_t _aidl_tag;
  if ((_aidl_ret_status = ::ndk::AParcel_readData(_parcel, &_aidl_tag)) != STATUS_OK) return _aidl_ret_status;
  switch (static_cast<Tag>(_aidl_tag)) {
  case acquiredInfo: {
    ::aidl::android::hardware::biometrics::fingerprint::AcquiredInfo _aidl_value;
    if ((_aidl_ret_status = ::ndk::AParcel_readData(_parcel, &_aidl_value)) != STATUS_OK) return _aidl_ret_status;
    if constexpr (std::is_trivially_copyable_v<::aidl::android::hardware::biometrics::fingerprint::AcquiredInfo>) {
      set<acquiredInfo>(_aidl_value);
    } else {
      // NOLINTNEXTLINE(performance-move-const-arg)
      set<acquiredInfo>(std::move(_aidl_value));
    }
    return STATUS_OK; }
  case vendorCode: {
    int32_t _aidl_value;
    if ((_aidl_ret_status = ::ndk::AParcel_readData(_parcel, &_aidl_value)) != STATUS_OK) return _aidl_ret_status;
    if constexpr (std::is_trivially_copyable_v<int32_t>) {
      set<vendorCode>(_aidl_value);
    } else {
      // NOLINTNEXTLINE(performance-move-const-arg)
      set<vendorCode>(std::move(_aidl_value));
    }
    return STATUS_OK; }
  }
  return STATUS_BAD_VALUE;
}
binder_status_t AcquiredInfoAndVendorCode::writeToParcel(AParcel* _parcel) const {
  binder_status_t _aidl_ret_status = ::ndk::AParcel_writeData(_parcel, static_cast<int32_t>(getTag()));
  if (_aidl_ret_status != STATUS_OK) return _aidl_ret_status;
  switch (getTag()) {
  case acquiredInfo: return ::ndk::AParcel_writeData(_parcel, get<acquiredInfo>());
  case vendorCode: return ::ndk::AParcel_writeData(_parcel, get<vendorCode>());
  }
  __assert2(__FILE__, __LINE__, __PRETTY_FUNCTION__, "can't reach here");
}

}  // namespace virtualhal
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
