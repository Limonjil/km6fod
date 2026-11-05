/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: C:\Users\Limonjil\Desktop\android\android-sdk\build-tools\35.0.0\aidl.exe --lang=ndk --structured --stability=vintf --min_sdk_version=31 -v 5 -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\keymaster\aidl -h C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\include -o C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\out C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl\android\hardware\biometrics\common\ComponentInfo.aidl
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
namespace common {
class ComponentInfo {
public:
  typedef std::false_type fixed_size;
  static const char* descriptor;

  std::string componentId;
  std::string hardwareVersion;
  std::string firmwareVersion;
  std::string serialNumber;
  std::string softwareVersion;

  binder_status_t readFromParcel(const AParcel* parcel);
  binder_status_t writeToParcel(AParcel* parcel) const;

  inline bool operator==(const ComponentInfo& _rhs) const {
    return std::tie(componentId, hardwareVersion, firmwareVersion, serialNumber, softwareVersion) == std::tie(_rhs.componentId, _rhs.hardwareVersion, _rhs.firmwareVersion, _rhs.serialNumber, _rhs.softwareVersion);
  }
  inline bool operator<(const ComponentInfo& _rhs) const {
    return std::tie(componentId, hardwareVersion, firmwareVersion, serialNumber, softwareVersion) < std::tie(_rhs.componentId, _rhs.hardwareVersion, _rhs.firmwareVersion, _rhs.serialNumber, _rhs.softwareVersion);
  }
  inline bool operator!=(const ComponentInfo& _rhs) const {
    return !(*this == _rhs);
  }
  inline bool operator>(const ComponentInfo& _rhs) const {
    return _rhs < *this;
  }
  inline bool operator>=(const ComponentInfo& _rhs) const {
    return !(*this < _rhs);
  }
  inline bool operator<=(const ComponentInfo& _rhs) const {
    return !(_rhs < *this);
  }

  static const ::ndk::parcelable_stability_t _aidl_stability = ::ndk::STABILITY_VINTF;
  inline std::string toString() const {
    std::ostringstream _aidl_os;
    _aidl_os << "ComponentInfo{";
    _aidl_os << "componentId: " << ::android::internal::ToString(componentId);
    _aidl_os << ", hardwareVersion: " << ::android::internal::ToString(hardwareVersion);
    _aidl_os << ", firmwareVersion: " << ::android::internal::ToString(firmwareVersion);
    _aidl_os << ", serialNumber: " << ::android::internal::ToString(serialNumber);
    _aidl_os << ", softwareVersion: " << ::android::internal::ToString(softwareVersion);
    _aidl_os << "}";
    return _aidl_os.str();
  }
};
}  // namespace common
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
