/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/limonjil/android/sdk/build-tools/35.0.0/aidl --lang=ndk --min_sdk_version=31 -o gen -Iflat --header_out=gen/include flat/android/hardware/biometrics/fingerprint/virtualhal/NextEnrollment.aidl
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
#include <aidl/android/hardware/biometrics/fingerprint/virtualhal/EnrollmentProgressStep.h>
#ifdef BINDER_STABILITY_SUPPORT
#include <android/binder_stability.h>
#endif  // BINDER_STABILITY_SUPPORT

namespace aidl::android::hardware::biometrics::fingerprint::virtualhal {
class EnrollmentProgressStep;
}  // namespace aidl::android::hardware::biometrics::fingerprint::virtualhal
namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {
namespace virtualhal {
class NextEnrollment {
public:
  typedef std::false_type fixed_size;
  static const char* descriptor;

  int32_t id = 0;
  std::vector<::aidl::android::hardware::biometrics::fingerprint::virtualhal::EnrollmentProgressStep> progressSteps;
  bool result = true;

  binder_status_t readFromParcel(const AParcel* parcel);
  binder_status_t writeToParcel(AParcel* parcel) const;

  inline bool operator==(const NextEnrollment& _rhs) const {
    return std::tie(id, progressSteps, result) == std::tie(_rhs.id, _rhs.progressSteps, _rhs.result);
  }
  inline bool operator<(const NextEnrollment& _rhs) const {
    return std::tie(id, progressSteps, result) < std::tie(_rhs.id, _rhs.progressSteps, _rhs.result);
  }
  inline bool operator!=(const NextEnrollment& _rhs) const {
    return !(*this == _rhs);
  }
  inline bool operator>(const NextEnrollment& _rhs) const {
    return _rhs < *this;
  }
  inline bool operator>=(const NextEnrollment& _rhs) const {
    return !(*this < _rhs);
  }
  inline bool operator<=(const NextEnrollment& _rhs) const {
    return !(_rhs < *this);
  }

  static const ::ndk::parcelable_stability_t _aidl_stability = ::ndk::STABILITY_LOCAL;
  inline std::string toString() const {
    std::ostringstream _aidl_os;
    _aidl_os << "NextEnrollment{";
    _aidl_os << "id: " << ::android::internal::ToString(id);
    _aidl_os << ", progressSteps: " << ::android::internal::ToString(progressSteps);
    _aidl_os << ", result: " << ::android::internal::ToString(result);
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
