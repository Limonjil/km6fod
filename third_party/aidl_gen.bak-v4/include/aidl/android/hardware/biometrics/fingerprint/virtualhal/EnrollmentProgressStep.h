/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/limonjil/android/sdk/build-tools/35.0.0/aidl --lang=ndk --min_sdk_version=31 -o gen -Iflat --header_out=gen/include flat/android/hardware/biometrics/fingerprint/virtualhal/EnrollmentProgressStep.aidl
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
#include <aidl/android/hardware/biometrics/fingerprint/virtualhal/AcquiredInfoAndVendorCode.h>
#ifdef BINDER_STABILITY_SUPPORT
#include <android/binder_stability.h>
#endif  // BINDER_STABILITY_SUPPORT

namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {
namespace virtualhal {
class EnrollmentProgressStep {
public:
  typedef std::false_type fixed_size;
  static const char* descriptor;

  int32_t durationMs = 0;
  std::vector<::aidl::android::hardware::biometrics::fingerprint::virtualhal::AcquiredInfoAndVendorCode> acquiredInfoAndVendorCodes;

  binder_status_t readFromParcel(const AParcel* parcel);
  binder_status_t writeToParcel(AParcel* parcel) const;

  inline bool operator==(const EnrollmentProgressStep& _rhs) const {
    return std::tie(durationMs, acquiredInfoAndVendorCodes) == std::tie(_rhs.durationMs, _rhs.acquiredInfoAndVendorCodes);
  }
  inline bool operator<(const EnrollmentProgressStep& _rhs) const {
    return std::tie(durationMs, acquiredInfoAndVendorCodes) < std::tie(_rhs.durationMs, _rhs.acquiredInfoAndVendorCodes);
  }
  inline bool operator!=(const EnrollmentProgressStep& _rhs) const {
    return !(*this == _rhs);
  }
  inline bool operator>(const EnrollmentProgressStep& _rhs) const {
    return _rhs < *this;
  }
  inline bool operator>=(const EnrollmentProgressStep& _rhs) const {
    return !(*this < _rhs);
  }
  inline bool operator<=(const EnrollmentProgressStep& _rhs) const {
    return !(_rhs < *this);
  }

  static const ::ndk::parcelable_stability_t _aidl_stability = ::ndk::STABILITY_LOCAL;
  inline std::string toString() const {
    std::ostringstream _aidl_os;
    _aidl_os << "EnrollmentProgressStep{";
    _aidl_os << "durationMs: " << ::android::internal::ToString(durationMs);
    _aidl_os << ", acquiredInfoAndVendorCodes: " << ::android::internal::ToString(acquiredInfoAndVendorCodes);
    _aidl_os << "}";
    return _aidl_os.str();
  }
};
}  // namespace virtualhal
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
