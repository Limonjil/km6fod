/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/limonjil/android/sdk/build-tools/35.0.0/aidl --lang=ndk --min_sdk_version=31 --structured --stability vintf -o gen -Iflat --header_out=gen/include flat/android/hardware/keymaster/VerificationToken.aidl
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
#include <aidl/android/hardware/keymaster/SecurityLevel.h>
#include <aidl/android/hardware/keymaster/Timestamp.h>
#ifdef BINDER_STABILITY_SUPPORT
#include <android/binder_stability.h>
#endif  // BINDER_STABILITY_SUPPORT

namespace aidl::android::hardware::keymaster {
class Timestamp;
}  // namespace aidl::android::hardware::keymaster
namespace aidl {
namespace android {
namespace hardware {
namespace keymaster {
class VerificationToken {
public:
  typedef std::false_type fixed_size;
  static const char* descriptor;

  int64_t challenge = 0L;
  ::aidl::android::hardware::keymaster::Timestamp timestamp;
  ::aidl::android::hardware::keymaster::SecurityLevel securityLevel = ::aidl::android::hardware::keymaster::SecurityLevel::SOFTWARE;
  std::vector<uint8_t> mac;

  binder_status_t readFromParcel(const AParcel* parcel);
  binder_status_t writeToParcel(AParcel* parcel) const;

  inline bool operator==(const VerificationToken& _rhs) const {
    return std::tie(challenge, timestamp, securityLevel, mac) == std::tie(_rhs.challenge, _rhs.timestamp, _rhs.securityLevel, _rhs.mac);
  }
  inline bool operator<(const VerificationToken& _rhs) const {
    return std::tie(challenge, timestamp, securityLevel, mac) < std::tie(_rhs.challenge, _rhs.timestamp, _rhs.securityLevel, _rhs.mac);
  }
  inline bool operator!=(const VerificationToken& _rhs) const {
    return !(*this == _rhs);
  }
  inline bool operator>(const VerificationToken& _rhs) const {
    return _rhs < *this;
  }
  inline bool operator>=(const VerificationToken& _rhs) const {
    return !(*this < _rhs);
  }
  inline bool operator<=(const VerificationToken& _rhs) const {
    return !(_rhs < *this);
  }

  static const ::ndk::parcelable_stability_t _aidl_stability = ::ndk::STABILITY_VINTF;
  inline std::string toString() const {
    std::ostringstream _aidl_os;
    _aidl_os << "VerificationToken{";
    _aidl_os << "challenge: " << ::android::internal::ToString(challenge);
    _aidl_os << ", timestamp: " << ::android::internal::ToString(timestamp);
    _aidl_os << ", securityLevel: " << ::android::internal::ToString(securityLevel);
    _aidl_os << ", mac: " << ::android::internal::ToString(mac);
    _aidl_os << "}";
    return _aidl_os.str();
  }
};
}  // namespace keymaster
}  // namespace hardware
}  // namespace android
}  // namespace aidl
