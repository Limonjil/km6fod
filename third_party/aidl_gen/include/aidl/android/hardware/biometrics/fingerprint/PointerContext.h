/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: C:\Users\Limonjil\Desktop\android\android-sdk\build-tools\35.0.0\aidl.exe --lang=ndk --structured --stability=vintf --min_sdk_version=31 -v 5 -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\keymaster\aidl -h C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\include -o C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\out C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl\android\hardware\biometrics\fingerprint\PointerContext.aidl
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
class PointerContext {
public:
  typedef std::false_type fixed_size;
  static const char* descriptor;

  int32_t pointerId = -1;
  float x = 0.000000f;
  float y = 0.000000f;
  float minor = 0.000000f;
  float major = 0.000000f;
  float orientation = 0.000000f;
  bool isAod = false;
  int64_t time = 0L;
  int64_t gestureStart = 0L;

  binder_status_t readFromParcel(const AParcel* parcel);
  binder_status_t writeToParcel(AParcel* parcel) const;

  inline bool operator==(const PointerContext& _rhs) const {
    return std::tie(pointerId, x, y, minor, major, orientation, isAod, time, gestureStart) == std::tie(_rhs.pointerId, _rhs.x, _rhs.y, _rhs.minor, _rhs.major, _rhs.orientation, _rhs.isAod, _rhs.time, _rhs.gestureStart);
  }
  inline bool operator<(const PointerContext& _rhs) const {
    return std::tie(pointerId, x, y, minor, major, orientation, isAod, time, gestureStart) < std::tie(_rhs.pointerId, _rhs.x, _rhs.y, _rhs.minor, _rhs.major, _rhs.orientation, _rhs.isAod, _rhs.time, _rhs.gestureStart);
  }
  inline bool operator!=(const PointerContext& _rhs) const {
    return !(*this == _rhs);
  }
  inline bool operator>(const PointerContext& _rhs) const {
    return _rhs < *this;
  }
  inline bool operator>=(const PointerContext& _rhs) const {
    return !(*this < _rhs);
  }
  inline bool operator<=(const PointerContext& _rhs) const {
    return !(_rhs < *this);
  }

  static const ::ndk::parcelable_stability_t _aidl_stability = ::ndk::STABILITY_VINTF;
  inline std::string toString() const {
    std::ostringstream _aidl_os;
    _aidl_os << "PointerContext{";
    _aidl_os << "pointerId: " << ::android::internal::ToString(pointerId);
    _aidl_os << ", x: " << ::android::internal::ToString(x);
    _aidl_os << ", y: " << ::android::internal::ToString(y);
    _aidl_os << ", minor: " << ::android::internal::ToString(minor);
    _aidl_os << ", major: " << ::android::internal::ToString(major);
    _aidl_os << ", orientation: " << ::android::internal::ToString(orientation);
    _aidl_os << ", isAod: " << ::android::internal::ToString(isAod);
    _aidl_os << ", time: " << ::android::internal::ToString(time);
    _aidl_os << ", gestureStart: " << ::android::internal::ToString(gestureStart);
    _aidl_os << "}";
    return _aidl_os.str();
  }
};
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
