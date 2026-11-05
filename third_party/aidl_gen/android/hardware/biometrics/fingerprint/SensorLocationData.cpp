/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: C:\Users\Limonjil\Desktop\android\android-sdk\build-tools\35.0.0\aidl.exe --lang=ndk --structured --stability=vintf --min_sdk_version=31 -v 5 -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\keymaster\aidl -h C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\include -o C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\out C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl\android\hardware\biometrics\fingerprint\SensorLocationData.aidl
 */
#include "aidl/android/hardware/biometrics/fingerprint/SensorLocationData.h"

#include <android/binder_parcel_utils.h>

namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {
const char* SensorLocationData::descriptor = "android.hardware.biometrics.fingerprint.SensorLocationData";

binder_status_t SensorLocationData::readFromParcel(const AParcel* _parcel) {
  binder_status_t _aidl_ret_status;
  int32_t _aidl_tag;
  if ((_aidl_ret_status = ::ndk::AParcel_readData(_parcel, &_aidl_tag)) != STATUS_OK) return _aidl_ret_status;
  switch (static_cast<Tag>(_aidl_tag)) {
  case underDisplayLocation: {
    ::aidl::android::hardware::biometrics::fingerprint::location::UnderDisplayLocation _aidl_value;
    if ((_aidl_ret_status = ::ndk::AParcel_readData(_parcel, &_aidl_value)) != STATUS_OK) return _aidl_ret_status;
    if constexpr (std::is_trivially_copyable_v<::aidl::android::hardware::biometrics::fingerprint::location::UnderDisplayLocation>) {
      set<underDisplayLocation>(_aidl_value);
    } else {
      // NOLINTNEXTLINE(performance-move-const-arg)
      set<underDisplayLocation>(std::move(_aidl_value));
    }
    return STATUS_OK; }
  case powerButtonDisplayLocation: {
    ::aidl::android::hardware::biometrics::fingerprint::location::PowerButtonDisplayLocation _aidl_value;
    if ((_aidl_ret_status = ::ndk::AParcel_readData(_parcel, &_aidl_value)) != STATUS_OK) return _aidl_ret_status;
    if constexpr (std::is_trivially_copyable_v<::aidl::android::hardware::biometrics::fingerprint::location::PowerButtonDisplayLocation>) {
      set<powerButtonDisplayLocation>(_aidl_value);
    } else {
      // NOLINTNEXTLINE(performance-move-const-arg)
      set<powerButtonDisplayLocation>(std::move(_aidl_value));
    }
    return STATUS_OK; }
  case powerButtonPhysicalLocation: {
    ::aidl::android::hardware::biometrics::fingerprint::location::PowerButtonPhysicalLocation _aidl_value;
    if ((_aidl_ret_status = ::ndk::AParcel_readData(_parcel, &_aidl_value)) != STATUS_OK) return _aidl_ret_status;
    if constexpr (std::is_trivially_copyable_v<::aidl::android::hardware::biometrics::fingerprint::location::PowerButtonPhysicalLocation>) {
      set<powerButtonPhysicalLocation>(_aidl_value);
    } else {
      // NOLINTNEXTLINE(performance-move-const-arg)
      set<powerButtonPhysicalLocation>(std::move(_aidl_value));
    }
    return STATUS_OK; }
  case standaloneLocation: {
    ::aidl::android::hardware::biometrics::fingerprint::location::StandaloneLocation _aidl_value;
    if ((_aidl_ret_status = ::ndk::AParcel_readData(_parcel, &_aidl_value)) != STATUS_OK) return _aidl_ret_status;
    if constexpr (std::is_trivially_copyable_v<::aidl::android::hardware::biometrics::fingerprint::location::StandaloneLocation>) {
      set<standaloneLocation>(_aidl_value);
    } else {
      // NOLINTNEXTLINE(performance-move-const-arg)
      set<standaloneLocation>(std::move(_aidl_value));
    }
    return STATUS_OK; }
  case homeButtonLocation: {
    ::aidl::android::hardware::biometrics::fingerprint::location::HomeButtonLocation _aidl_value;
    if ((_aidl_ret_status = ::ndk::AParcel_readData(_parcel, &_aidl_value)) != STATUS_OK) return _aidl_ret_status;
    if constexpr (std::is_trivially_copyable_v<::aidl::android::hardware::biometrics::fingerprint::location::HomeButtonLocation>) {
      set<homeButtonLocation>(_aidl_value);
    } else {
      // NOLINTNEXTLINE(performance-move-const-arg)
      set<homeButtonLocation>(std::move(_aidl_value));
    }
    return STATUS_OK; }
  case rearLocation: {
    ::aidl::android::hardware::biometrics::fingerprint::location::RearLocation _aidl_value;
    if ((_aidl_ret_status = ::ndk::AParcel_readData(_parcel, &_aidl_value)) != STATUS_OK) return _aidl_ret_status;
    if constexpr (std::is_trivially_copyable_v<::aidl::android::hardware::biometrics::fingerprint::location::RearLocation>) {
      set<rearLocation>(_aidl_value);
    } else {
      // NOLINTNEXTLINE(performance-move-const-arg)
      set<rearLocation>(std::move(_aidl_value));
    }
    return STATUS_OK; }
  }
  return STATUS_BAD_VALUE;
}
binder_status_t SensorLocationData::writeToParcel(AParcel* _parcel) const {
  binder_status_t _aidl_ret_status = ::ndk::AParcel_writeData(_parcel, static_cast<int32_t>(getTag()));
  if (_aidl_ret_status != STATUS_OK) return _aidl_ret_status;
  switch (getTag()) {
  case underDisplayLocation: return ::ndk::AParcel_writeData(_parcel, get<underDisplayLocation>());
  case powerButtonDisplayLocation: return ::ndk::AParcel_writeData(_parcel, get<powerButtonDisplayLocation>());
  case powerButtonPhysicalLocation: return ::ndk::AParcel_writeData(_parcel, get<powerButtonPhysicalLocation>());
  case standaloneLocation: return ::ndk::AParcel_writeData(_parcel, get<standaloneLocation>());
  case homeButtonLocation: return ::ndk::AParcel_writeData(_parcel, get<homeButtonLocation>());
  case rearLocation: return ::ndk::AParcel_writeData(_parcel, get<rearLocation>());
  }
  __assert2(__FILE__, __LINE__, __PRETTY_FUNCTION__, "can't reach here");
}

}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
