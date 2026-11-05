/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: C:\Users\Limonjil\Desktop\android\android-sdk\build-tools\35.0.0\aidl.exe --lang=ndk --structured --stability=vintf --min_sdk_version=31 -v 5 -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\keymaster\aidl -h C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\include -o C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\out C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl\android\hardware\biometrics\fingerprint\AuthenticateSuccess.aidl
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
#include <aidl/android/hardware/keymaster/HardwareAuthToken.h>
#ifdef BINDER_STABILITY_SUPPORT
#include <android/binder_stability.h>
#endif  // BINDER_STABILITY_SUPPORT

namespace aidl::android::hardware::keymaster {
class HardwareAuthToken;
}  // namespace aidl::android::hardware::keymaster
namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {
class AuthenticateSuccess {
public:
  typedef std::false_type fixed_size;
  static const char* descriptor;

  int32_t enrollmentId = 0;
  ::aidl::android::hardware::keymaster::HardwareAuthToken hat;
  ::ndk::AParcelableHolder metadata{::ndk::STABILITY_VINTF};

  binder_status_t readFromParcel(const AParcel* parcel);
  binder_status_t writeToParcel(AParcel* parcel) const;

  inline bool operator==(const AuthenticateSuccess& _rhs) const {
    return std::tie(enrollmentId, hat, metadata) == std::tie(_rhs.enrollmentId, _rhs.hat, _rhs.metadata);
  }
  inline bool operator<(const AuthenticateSuccess& _rhs) const {
    return std::tie(enrollmentId, hat, metadata) < std::tie(_rhs.enrollmentId, _rhs.hat, _rhs.metadata);
  }
  inline bool operator!=(const AuthenticateSuccess& _rhs) const {
    return !(*this == _rhs);
  }
  inline bool operator>(const AuthenticateSuccess& _rhs) const {
    return _rhs < *this;
  }
  inline bool operator>=(const AuthenticateSuccess& _rhs) const {
    return !(*this < _rhs);
  }
  inline bool operator<=(const AuthenticateSuccess& _rhs) const {
    return !(_rhs < *this);
  }

  static const ::ndk::parcelable_stability_t _aidl_stability = ::ndk::STABILITY_VINTF;
  inline std::string toString() const {
    std::ostringstream _aidl_os;
    _aidl_os << "AuthenticateSuccess{";
    _aidl_os << "enrollmentId: " << ::android::internal::ToString(enrollmentId);
    _aidl_os << ", hat: " << ::android::internal::ToString(hat);
    _aidl_os << ", metadata: " << ::android::internal::ToString(metadata);
    _aidl_os << "}";
    return _aidl_os.str();
  }
};
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
