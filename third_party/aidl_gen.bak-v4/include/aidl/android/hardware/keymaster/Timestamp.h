/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/limonjil/android/sdk/build-tools/35.0.0/aidl --lang=ndk --min_sdk_version=31 --structured --stability vintf -o gen -Iflat --header_out=gen/include flat/android/hardware/keymaster/Timestamp.aidl
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
namespace keymaster {
class Timestamp {
public:
  typedef std::false_type fixed_size;
  static const char* descriptor;

  int64_t milliSeconds = 0L;

  binder_status_t readFromParcel(const AParcel* parcel);
  binder_status_t writeToParcel(AParcel* parcel) const;

  inline bool operator==(const Timestamp& _rhs) const {
    return std::tie(milliSeconds) == std::tie(_rhs.milliSeconds);
  }
  inline bool operator<(const Timestamp& _rhs) const {
    return std::tie(milliSeconds) < std::tie(_rhs.milliSeconds);
  }
  inline bool operator!=(const Timestamp& _rhs) const {
    return !(*this == _rhs);
  }
  inline bool operator>(const Timestamp& _rhs) const {
    return _rhs < *this;
  }
  inline bool operator>=(const Timestamp& _rhs) const {
    return !(*this < _rhs);
  }
  inline bool operator<=(const Timestamp& _rhs) const {
    return !(_rhs < *this);
  }

  static const ::ndk::parcelable_stability_t _aidl_stability = ::ndk::STABILITY_VINTF;
  inline std::string toString() const {
    std::ostringstream _aidl_os;
    _aidl_os << "Timestamp{";
    _aidl_os << "milliSeconds: " << ::android::internal::ToString(milliSeconds);
    _aidl_os << "}";
    return _aidl_os.str();
  }
};
}  // namespace keymaster
}  // namespace hardware
}  // namespace android
}  // namespace aidl
