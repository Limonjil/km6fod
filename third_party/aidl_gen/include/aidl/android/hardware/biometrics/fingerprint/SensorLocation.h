/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: C:\Users\Limonjil\Desktop\android\android-sdk\build-tools\35.0.0\aidl.exe --lang=ndk --structured --stability=vintf --min_sdk_version=31 -v 5 -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\keymaster\aidl -h C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\include -o C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\out C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl\android\hardware\biometrics\fingerprint\SensorLocation.aidl
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
#include <aidl/android/hardware/biometrics/fingerprint/SensorLocationData.h>
#include <aidl/android/hardware/biometrics/fingerprint/SensorShape.h>
#ifdef BINDER_STABILITY_SUPPORT
#include <android/binder_stability.h>
#endif  // BINDER_STABILITY_SUPPORT

namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
class SensorLocation {
public:
  typedef std::false_type fixed_size;
  static const char* descriptor;

  int32_t __attribute__((deprecated("use the display field instead. This field was never used."))) displayId = 0;
  int32_t __attribute__((deprecated("use sensorLocationData with a specific struct (e.g., UnderDisplayLocation) instead. This value will be ignored if sensorLocationData is set."))) sensorLocationX = 0;
  int32_t __attribute__((deprecated("use sensorLocationData with a specific struct (e.g., UnderDisplayLocation) instead. This value will be ignored if sensorLocationData is set."))) sensorLocationY = 0;
  int32_t __attribute__((deprecated("use sensorLocationData with a specific struct (e.g., UnderDisplayLocation) instead. This value will be ignored if sensorLocationData is set."))) sensorRadius = 0;
  std::string display = "";
  ::aidl::android::hardware::biometrics::fingerprint::SensorShape sensorShape = ::aidl::android::hardware::biometrics::fingerprint::SensorShape::CIRCLE;
  std::optional<::aidl::android::hardware::biometrics::fingerprint::SensorLocationData> sensorLocationData;

  binder_status_t readFromParcel(const AParcel* parcel);
  binder_status_t writeToParcel(AParcel* parcel) const;

  inline bool operator==(const SensorLocation& _rhs) const {
    return std::tie(displayId, sensorLocationX, sensorLocationY, sensorRadius, display, sensorShape, sensorLocationData) == std::tie(_rhs.displayId, _rhs.sensorLocationX, _rhs.sensorLocationY, _rhs.sensorRadius, _rhs.display, _rhs.sensorShape, _rhs.sensorLocationData);
  }
  inline bool operator<(const SensorLocation& _rhs) const {
    return std::tie(displayId, sensorLocationX, sensorLocationY, sensorRadius, display, sensorShape, sensorLocationData) < std::tie(_rhs.displayId, _rhs.sensorLocationX, _rhs.sensorLocationY, _rhs.sensorRadius, _rhs.display, _rhs.sensorShape, _rhs.sensorLocationData);
  }
  inline bool operator!=(const SensorLocation& _rhs) const {
    return !(*this == _rhs);
  }
  inline bool operator>(const SensorLocation& _rhs) const {
    return _rhs < *this;
  }
  inline bool operator>=(const SensorLocation& _rhs) const {
    return !(*this < _rhs);
  }
  inline bool operator<=(const SensorLocation& _rhs) const {
    return !(_rhs < *this);
  }

  static const ::ndk::parcelable_stability_t _aidl_stability = ::ndk::STABILITY_VINTF;
  inline std::string toString() const {
    std::ostringstream _aidl_os;
    _aidl_os << "SensorLocation{";
    _aidl_os << "displayId: " << ::android::internal::ToString(displayId);
    _aidl_os << ", sensorLocationX: " << ::android::internal::ToString(sensorLocationX);
    _aidl_os << ", sensorLocationY: " << ::android::internal::ToString(sensorLocationY);
    _aidl_os << ", sensorRadius: " << ::android::internal::ToString(sensorRadius);
    _aidl_os << ", display: " << ::android::internal::ToString(display);
    _aidl_os << ", sensorShape: " << ::android::internal::ToString(sensorShape);
    _aidl_os << ", sensorLocationData: " << ::android::internal::ToString(sensorLocationData);
    _aidl_os << "}";
    return _aidl_os.str();
  }
};
#pragma clang diagnostic pop
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
