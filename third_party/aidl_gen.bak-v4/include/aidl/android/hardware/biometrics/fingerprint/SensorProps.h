/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/limonjil/android/sdk/build-tools/35.0.0/aidl --lang=ndk --min_sdk_version=31 --structured --stability vintf -o gen -Iflat --header_out=gen/include flat/android/hardware/biometrics/fingerprint/SensorProps.aidl
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
#include <aidl/android/hardware/biometrics/common/CommonProps.h>
#include <aidl/android/hardware/biometrics/fingerprint/FingerprintSensorType.h>
#include <aidl/android/hardware/biometrics/fingerprint/SensorLocation.h>
#include <aidl/android/hardware/biometrics/fingerprint/TouchDetectionParameters.h>
#ifdef BINDER_STABILITY_SUPPORT
#include <android/binder_stability.h>
#endif  // BINDER_STABILITY_SUPPORT

namespace aidl::android::hardware::biometrics::common {
class CommonProps;
}  // namespace aidl::android::hardware::biometrics::common
namespace aidl::android::hardware::biometrics::fingerprint {
class SensorLocation;
class TouchDetectionParameters;
}  // namespace aidl::android::hardware::biometrics::fingerprint
namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {
class SensorProps {
public:
  typedef std::false_type fixed_size;
  static const char* descriptor;

  ::aidl::android::hardware::biometrics::common::CommonProps commonProps;
  ::aidl::android::hardware::biometrics::fingerprint::FingerprintSensorType sensorType = ::aidl::android::hardware::biometrics::fingerprint::FingerprintSensorType::UNKNOWN;
  std::vector<::aidl::android::hardware::biometrics::fingerprint::SensorLocation> sensorLocations;
  bool supportsNavigationGestures = false;
  bool supportsDetectInteraction = false;
  bool halHandlesDisplayTouches = false;
  bool halControlsIllumination = false;
  std::optional<::aidl::android::hardware::biometrics::fingerprint::TouchDetectionParameters> touchDetectionParameters;

  binder_status_t readFromParcel(const AParcel* parcel);
  binder_status_t writeToParcel(AParcel* parcel) const;

  inline bool operator==(const SensorProps& _rhs) const {
    return std::tie(commonProps, sensorType, sensorLocations, supportsNavigationGestures, supportsDetectInteraction, halHandlesDisplayTouches, halControlsIllumination, touchDetectionParameters) == std::tie(_rhs.commonProps, _rhs.sensorType, _rhs.sensorLocations, _rhs.supportsNavigationGestures, _rhs.supportsDetectInteraction, _rhs.halHandlesDisplayTouches, _rhs.halControlsIllumination, _rhs.touchDetectionParameters);
  }
  inline bool operator<(const SensorProps& _rhs) const {
    return std::tie(commonProps, sensorType, sensorLocations, supportsNavigationGestures, supportsDetectInteraction, halHandlesDisplayTouches, halControlsIllumination, touchDetectionParameters) < std::tie(_rhs.commonProps, _rhs.sensorType, _rhs.sensorLocations, _rhs.supportsNavigationGestures, _rhs.supportsDetectInteraction, _rhs.halHandlesDisplayTouches, _rhs.halControlsIllumination, _rhs.touchDetectionParameters);
  }
  inline bool operator!=(const SensorProps& _rhs) const {
    return !(*this == _rhs);
  }
  inline bool operator>(const SensorProps& _rhs) const {
    return _rhs < *this;
  }
  inline bool operator>=(const SensorProps& _rhs) const {
    return !(*this < _rhs);
  }
  inline bool operator<=(const SensorProps& _rhs) const {
    return !(_rhs < *this);
  }

  static const ::ndk::parcelable_stability_t _aidl_stability = ::ndk::STABILITY_VINTF;
  inline std::string toString() const {
    std::ostringstream _aidl_os;
    _aidl_os << "SensorProps{";
    _aidl_os << "commonProps: " << ::android::internal::ToString(commonProps);
    _aidl_os << ", sensorType: " << ::android::internal::ToString(sensorType);
    _aidl_os << ", sensorLocations: " << ::android::internal::ToString(sensorLocations);
    _aidl_os << ", supportsNavigationGestures: " << ::android::internal::ToString(supportsNavigationGestures);
    _aidl_os << ", supportsDetectInteraction: " << ::android::internal::ToString(supportsDetectInteraction);
    _aidl_os << ", halHandlesDisplayTouches: " << ::android::internal::ToString(halHandlesDisplayTouches);
    _aidl_os << ", halControlsIllumination: " << ::android::internal::ToString(halControlsIllumination);
    _aidl_os << ", touchDetectionParameters: " << ::android::internal::ToString(touchDetectionParameters);
    _aidl_os << "}";
    return _aidl_os.str();
  }
};
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
