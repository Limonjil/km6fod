/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/limonjil/android/sdk/build-tools/35.0.0/aidl --lang=ndk --min_sdk_version=31 --structured --stability vintf -o gen -Iflat --header_out=gen/include flat/android/hardware/biometrics/common/AuthenticateReason.aidl
 */
#include "aidl/android/hardware/biometrics/common/AuthenticateReason.h"

#include <android/binder_parcel_utils.h>

namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace common {
const char* AuthenticateReason::descriptor = "android.hardware.biometrics.common.AuthenticateReason";

binder_status_t AuthenticateReason::readFromParcel(const AParcel* _parcel) {
  binder_status_t _aidl_ret_status;
  int32_t _aidl_tag;
  if ((_aidl_ret_status = ::ndk::AParcel_readData(_parcel, &_aidl_tag)) != STATUS_OK) return _aidl_ret_status;
  switch (static_cast<Tag>(_aidl_tag)) {
  case vendorAuthenticateReason: {
    ::aidl::android::hardware::biometrics::common::AuthenticateReason::Vendor _aidl_value;
    if ((_aidl_ret_status = ::ndk::AParcel_readData(_parcel, &_aidl_value)) != STATUS_OK) return _aidl_ret_status;
    if constexpr (std::is_trivially_copyable_v<::aidl::android::hardware::biometrics::common::AuthenticateReason::Vendor>) {
      set<vendorAuthenticateReason>(_aidl_value);
    } else {
      // NOLINTNEXTLINE(performance-move-const-arg)
      set<vendorAuthenticateReason>(std::move(_aidl_value));
    }
    return STATUS_OK; }
  case faceAuthenticateReason: {
    ::aidl::android::hardware::biometrics::common::AuthenticateReason::Face _aidl_value;
    if ((_aidl_ret_status = ::ndk::AParcel_readData(_parcel, &_aidl_value)) != STATUS_OK) return _aidl_ret_status;
    if constexpr (std::is_trivially_copyable_v<::aidl::android::hardware::biometrics::common::AuthenticateReason::Face>) {
      set<faceAuthenticateReason>(_aidl_value);
    } else {
      // NOLINTNEXTLINE(performance-move-const-arg)
      set<faceAuthenticateReason>(std::move(_aidl_value));
    }
    return STATUS_OK; }
  case fingerprintAuthenticateReason: {
    ::aidl::android::hardware::biometrics::common::AuthenticateReason::Fingerprint _aidl_value;
    if ((_aidl_ret_status = ::ndk::AParcel_readData(_parcel, &_aidl_value)) != STATUS_OK) return _aidl_ret_status;
    if constexpr (std::is_trivially_copyable_v<::aidl::android::hardware::biometrics::common::AuthenticateReason::Fingerprint>) {
      set<fingerprintAuthenticateReason>(_aidl_value);
    } else {
      // NOLINTNEXTLINE(performance-move-const-arg)
      set<fingerprintAuthenticateReason>(std::move(_aidl_value));
    }
    return STATUS_OK; }
  }
  return STATUS_BAD_VALUE;
}
binder_status_t AuthenticateReason::writeToParcel(AParcel* _parcel) const {
  binder_status_t _aidl_ret_status = ::ndk::AParcel_writeData(_parcel, static_cast<int32_t>(getTag()));
  if (_aidl_ret_status != STATUS_OK) return _aidl_ret_status;
  switch (getTag()) {
  case vendorAuthenticateReason: return ::ndk::AParcel_writeData(_parcel, get<vendorAuthenticateReason>());
  case faceAuthenticateReason: return ::ndk::AParcel_writeData(_parcel, get<faceAuthenticateReason>());
  case fingerprintAuthenticateReason: return ::ndk::AParcel_writeData(_parcel, get<fingerprintAuthenticateReason>());
  }
  __assert2(__FILE__, __LINE__, __PRETTY_FUNCTION__, "can't reach here");
}

}  // namespace common
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace common {
const char* AuthenticateReason::Vendor::descriptor = "android.hardware.biometrics.common.AuthenticateReason.Vendor";

binder_status_t AuthenticateReason::Vendor::readFromParcel(const AParcel* _aidl_parcel) {
  binder_status_t _aidl_ret_status = STATUS_OK;
  int32_t _aidl_start_pos = AParcel_getDataPosition(_aidl_parcel);
  int32_t _aidl_parcelable_size = 0;
  _aidl_ret_status = AParcel_readInt32(_aidl_parcel, &_aidl_parcelable_size);
  if (_aidl_ret_status != STATUS_OK) return _aidl_ret_status;

  if (_aidl_parcelable_size < 4) return STATUS_BAD_VALUE;
  if (_aidl_start_pos > INT32_MAX - _aidl_parcelable_size) return STATUS_BAD_VALUE;
  if (AParcel_getDataPosition(_aidl_parcel) - _aidl_start_pos >= _aidl_parcelable_size) {
    AParcel_setDataPosition(_aidl_parcel, _aidl_start_pos + _aidl_parcelable_size);
    return _aidl_ret_status;
  }
  _aidl_ret_status = ::ndk::AParcel_readData(_aidl_parcel, &extension);
  if (_aidl_ret_status != STATUS_OK) return _aidl_ret_status;

  AParcel_setDataPosition(_aidl_parcel, _aidl_start_pos + _aidl_parcelable_size);
  return _aidl_ret_status;
}
binder_status_t AuthenticateReason::Vendor::writeToParcel(AParcel* _aidl_parcel) const {
  binder_status_t _aidl_ret_status;
  size_t _aidl_start_pos = AParcel_getDataPosition(_aidl_parcel);
  _aidl_ret_status = AParcel_writeInt32(_aidl_parcel, 0);
  if (_aidl_ret_status != STATUS_OK) return _aidl_ret_status;

  _aidl_ret_status = ::ndk::AParcel_writeData(_aidl_parcel, extension);
  if (_aidl_ret_status != STATUS_OK) return _aidl_ret_status;

  size_t _aidl_end_pos = AParcel_getDataPosition(_aidl_parcel);
  AParcel_setDataPosition(_aidl_parcel, _aidl_start_pos);
  AParcel_writeInt32(_aidl_parcel, _aidl_end_pos - _aidl_start_pos);
  AParcel_setDataPosition(_aidl_parcel, _aidl_end_pos);
  return _aidl_ret_status;
}

}  // namespace common
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
