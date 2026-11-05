/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: C:\Users\Limonjil\Desktop\android\android-sdk\build-tools\35.0.0\aidl.exe --lang=ndk --structured --stability=vintf --min_sdk_version=31 -v 5 -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\keymaster\aidl -h C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\include -o C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\out C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl\android\hardware\biometrics\fingerprint\location\StandaloneLocation.aidl
 */
#include "aidl/android/hardware/biometrics/fingerprint/location/StandaloneLocation.h"

#include <android/binder_parcel_utils.h>

namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {
namespace location {
const char* StandaloneLocation::descriptor = "android.hardware.biometrics.fingerprint.location.StandaloneLocation";

binder_status_t StandaloneLocation::readFromParcel(const AParcel* _aidl_parcel) {
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
  _aidl_ret_status = ::ndk::AParcel_readData(_aidl_parcel, &location);
  if (_aidl_ret_status != STATUS_OK) return _aidl_ret_status;

  AParcel_setDataPosition(_aidl_parcel, _aidl_start_pos + _aidl_parcelable_size);
  return _aidl_ret_status;
}
binder_status_t StandaloneLocation::writeToParcel(AParcel* _aidl_parcel) const {
  binder_status_t _aidl_ret_status;
  size_t _aidl_start_pos = AParcel_getDataPosition(_aidl_parcel);
  _aidl_ret_status = AParcel_writeInt32(_aidl_parcel, 0);
  if (_aidl_ret_status != STATUS_OK) return _aidl_ret_status;

  _aidl_ret_status = ::ndk::AParcel_writeData(_aidl_parcel, location);
  if (_aidl_ret_status != STATUS_OK) return _aidl_ret_status;

  size_t _aidl_end_pos = AParcel_getDataPosition(_aidl_parcel);
  AParcel_setDataPosition(_aidl_parcel, _aidl_start_pos);
  AParcel_writeInt32(_aidl_parcel, _aidl_end_pos - _aidl_start_pos);
  AParcel_setDataPosition(_aidl_parcel, _aidl_end_pos);
  return _aidl_ret_status;
}

}  // namespace location
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
