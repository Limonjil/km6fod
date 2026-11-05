/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/limonjil/android/sdk/build-tools/35.0.0/aidl --lang=ndk --min_sdk_version=31 --structured --stability vintf -o gen -Iflat --header_out=gen/include flat/android/hardware/biometrics/fingerprint/TouchDetectionParameters.aidl
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
class TouchDetectionParameters {
public:
  typedef std::false_type fixed_size;
  static const char* descriptor;

  float targetSize = 1.000000f;
  float minOverlap = 0.000000f;

  binder_status_t readFromParcel(const AParcel* parcel);
  binder_status_t writeToParcel(AParcel* parcel) const;

  inline bool operator==(const TouchDetectionParameters& _rhs) const {
    return std::tie(targetSize, minOverlap) == std::tie(_rhs.targetSize, _rhs.minOverlap);
  }
  inline bool operator<(const TouchDetectionParameters& _rhs) const {
    return std::tie(targetSize, minOverlap) < std::tie(_rhs.targetSize, _rhs.minOverlap);
  }
  inline bool operator!=(const TouchDetectionParameters& _rhs) const {
    return !(*this == _rhs);
  }
  inline bool operator>(const TouchDetectionParameters& _rhs) const {
    return _rhs < *this;
  }
  inline bool operator>=(const TouchDetectionParameters& _rhs) const {
    return !(*this < _rhs);
  }
  inline bool operator<=(const TouchDetectionParameters& _rhs) const {
    return !(_rhs < *this);
  }

  static const ::ndk::parcelable_stability_t _aidl_stability = ::ndk::STABILITY_VINTF;
  inline std::string toString() const {
    std::ostringstream _aidl_os;
    _aidl_os << "TouchDetectionParameters{";
    _aidl_os << "targetSize: " << ::android::internal::ToString(targetSize);
    _aidl_os << ", minOverlap: " << ::android::internal::ToString(minOverlap);
    _aidl_os << "}";
    return _aidl_os.str();
  }
};
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
