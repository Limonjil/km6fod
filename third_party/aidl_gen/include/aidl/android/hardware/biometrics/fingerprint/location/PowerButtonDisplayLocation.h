/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: C:\Users\Limonjil\Desktop\android\android-sdk\build-tools\35.0.0\aidl.exe --lang=ndk --structured --stability=vintf --min_sdk_version=31 -v 5 -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\keymaster\aidl -h C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\include -o C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\out C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl\android\hardware\biometrics\fingerprint\location\PowerButtonDisplayLocation.aidl
 */
#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <android/binder_interface_utils.h>
#include <android/binder_parcelable_utils.h>
#include <android/binder_to_string.h>
#ifdef BINDER_STABILITY_SUPPORT
#include <android/binder_stability.h>
#endif  // BINDER_STABILITY_SUPPORT

namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {
namespace location {
class PowerButtonDisplayLocation {
public:
  typedef std::false_type fixed_size;
  static const char* descriptor;

  int32_t sensorLocationXPixels = 0;
  int32_t sensorLocationYPixels = 0;
  int32_t sensorRadiusPixels = 0;

  binder_status_t readFromParcel(const AParcel* parcel);
  binder_status_t writeToParcel(AParcel* parcel) const;

  inline bool operator==(const PowerButtonDisplayLocation& _rhs) const {
    return std::tie(sensorLocationXPixels, sensorLocationYPixels, sensorRadiusPixels) == std::tie(_rhs.sensorLocationXPixels, _rhs.sensorLocationYPixels, _rhs.sensorRadiusPixels);
  }
  inline bool operator<(const PowerButtonDisplayLocation& _rhs) const {
    return std::tie(sensorLocationXPixels, sensorLocationYPixels, sensorRadiusPixels) < std::tie(_rhs.sensorLocationXPixels, _rhs.sensorLocationYPixels, _rhs.sensorRadiusPixels);
  }
  inline bool operator!=(const PowerButtonDisplayLocation& _rhs) const {
    return !(*this == _rhs);
  }
  inline bool operator>(const PowerButtonDisplayLocation& _rhs) const {
    return _rhs < *this;
  }
  inline bool operator>=(const PowerButtonDisplayLocation& _rhs) const {
    return !(*this < _rhs);
  }
  inline bool operator<=(const PowerButtonDisplayLocation& _rhs) const {
    return !(_rhs < *this);
  }

  static const ::ndk::parcelable_stability_t _aidl_stability = ::ndk::STABILITY_VINTF;
  inline std::string toString() const {
    std::ostringstream _aidl_os;
    _aidl_os << "PowerButtonDisplayLocation{";
    _aidl_os << "sensorLocationXPixels: " << ::android::internal::ToString(sensorLocationXPixels);
    _aidl_os << ", sensorLocationYPixels: " << ::android::internal::ToString(sensorLocationYPixels);
    _aidl_os << ", sensorRadiusPixels: " << ::android::internal::ToString(sensorRadiusPixels);
    _aidl_os << "}";
    return _aidl_os.str();
  }
};
}  // namespace location
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
