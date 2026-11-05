/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/limonjil/android/sdk/build-tools/35.0.0/aidl --lang=ndk --min_sdk_version=31 --structured --stability vintf -o gen -Iflat --header_out=gen/include flat/android/hardware/biometrics/common/CommonProps.aidl
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
#include <aidl/android/hardware/biometrics/common/ComponentInfo.h>
#include <aidl/android/hardware/biometrics/common/SensorStrength.h>
#ifdef BINDER_STABILITY_SUPPORT
#include <android/binder_stability.h>
#endif  // BINDER_STABILITY_SUPPORT

namespace aidl::android::hardware::biometrics::common {
class ComponentInfo;
}  // namespace aidl::android::hardware::biometrics::common
namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace common {
class CommonProps {
public:
  typedef std::false_type fixed_size;
  static const char* descriptor;

  int32_t sensorId = 0;
  ::aidl::android::hardware::biometrics::common::SensorStrength sensorStrength = ::aidl::android::hardware::biometrics::common::SensorStrength::CONVENIENCE;
  int32_t maxEnrollmentsPerUser = 0;
  std::vector<::aidl::android::hardware::biometrics::common::ComponentInfo> componentInfo;

  binder_status_t readFromParcel(const AParcel* parcel);
  binder_status_t writeToParcel(AParcel* parcel) const;

  inline bool operator==(const CommonProps& _rhs) const {
    return std::tie(sensorId, sensorStrength, maxEnrollmentsPerUser, componentInfo) == std::tie(_rhs.sensorId, _rhs.sensorStrength, _rhs.maxEnrollmentsPerUser, _rhs.componentInfo);
  }
  inline bool operator<(const CommonProps& _rhs) const {
    return std::tie(sensorId, sensorStrength, maxEnrollmentsPerUser, componentInfo) < std::tie(_rhs.sensorId, _rhs.sensorStrength, _rhs.maxEnrollmentsPerUser, _rhs.componentInfo);
  }
  inline bool operator!=(const CommonProps& _rhs) const {
    return !(*this == _rhs);
  }
  inline bool operator>(const CommonProps& _rhs) const {
    return _rhs < *this;
  }
  inline bool operator>=(const CommonProps& _rhs) const {
    return !(*this < _rhs);
  }
  inline bool operator<=(const CommonProps& _rhs) const {
    return !(_rhs < *this);
  }

  static const ::ndk::parcelable_stability_t _aidl_stability = ::ndk::STABILITY_VINTF;
  inline std::string toString() const {
    std::ostringstream _aidl_os;
    _aidl_os << "CommonProps{";
    _aidl_os << "sensorId: " << ::android::internal::ToString(sensorId);
    _aidl_os << ", sensorStrength: " << ::android::internal::ToString(sensorStrength);
    _aidl_os << ", maxEnrollmentsPerUser: " << ::android::internal::ToString(maxEnrollmentsPerUser);
    _aidl_os << ", componentInfo: " << ::android::internal::ToString(componentInfo);
    _aidl_os << "}";
    return _aidl_os.str();
  }
};
}  // namespace common
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
