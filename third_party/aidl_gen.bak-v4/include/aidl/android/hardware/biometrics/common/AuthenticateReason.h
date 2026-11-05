/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/limonjil/android/sdk/build-tools/35.0.0/aidl --lang=ndk --min_sdk_version=31 --structured --stability vintf -o gen -Iflat --header_out=gen/include flat/android/hardware/biometrics/common/AuthenticateReason.aidl
 */
#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>
#include <android/binder_enums.h>
#include <android/binder_interface_utils.h>
#include <android/binder_parcelable_utils.h>
#include <android/binder_to_string.h>
#include <aidl/android/hardware/biometrics/common/AuthenticateReason.h>
#ifdef BINDER_STABILITY_SUPPORT
#include <android/binder_stability.h>
#endif  // BINDER_STABILITY_SUPPORT

#ifndef __BIONIC__
#define __assert2(a,b,c,d) ((void)0)
#endif

namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace common {
class AuthenticateReason {
public:
  typedef std::false_type fixed_size;
  static const char* descriptor;

  class Vendor {
  public:
    typedef std::false_type fixed_size;
    static const char* descriptor;

    ::ndk::AParcelableHolder extension{::ndk::STABILITY_VINTF};

    binder_status_t readFromParcel(const AParcel* parcel);
    binder_status_t writeToParcel(AParcel* parcel) const;

    inline bool operator==(const Vendor& _rhs) const {
      return std::tie(extension) == std::tie(_rhs.extension);
    }
    inline bool operator<(const Vendor& _rhs) const {
      return std::tie(extension) < std::tie(_rhs.extension);
    }
    inline bool operator!=(const Vendor& _rhs) const {
      return !(*this == _rhs);
    }
    inline bool operator>(const Vendor& _rhs) const {
      return _rhs < *this;
    }
    inline bool operator>=(const Vendor& _rhs) const {
      return !(*this < _rhs);
    }
    inline bool operator<=(const Vendor& _rhs) const {
      return !(_rhs < *this);
    }

    static const ::ndk::parcelable_stability_t _aidl_stability = ::ndk::STABILITY_VINTF;
    inline std::string toString() const {
      std::ostringstream _aidl_os;
      _aidl_os << "Vendor{";
      _aidl_os << "extension: " << ::android::internal::ToString(extension);
      _aidl_os << "}";
      return _aidl_os.str();
    }
  };
  enum class Fingerprint : int32_t {
    UNKNOWN = 0,
  };

  enum class Face : int32_t {
    UNKNOWN = 0,
    STARTED_WAKING_UP = 1,
    PRIMARY_BOUNCER_SHOWN = 2,
    ASSISTANT_VISIBLE = 3,
    ALTERNATE_BIOMETRIC_BOUNCER_SHOWN = 4,
    NOTIFICATION_PANEL_CLICKED = 5,
    OCCLUDING_APP_REQUESTED = 6,
    PICK_UP_GESTURE_TRIGGERED = 7,
    QS_EXPANDED = 8,
    SWIPE_UP_ON_BOUNCER = 9,
    UDFPS_POINTER_DOWN = 10,
  };

  enum class Tag : int32_t {
    vendorAuthenticateReason = 0,
    faceAuthenticateReason = 1,
    fingerprintAuthenticateReason = 2,
  };

  // Expose tag symbols for legacy code
  static const inline Tag vendorAuthenticateReason = Tag::vendorAuthenticateReason;
  static const inline Tag faceAuthenticateReason = Tag::faceAuthenticateReason;
  static const inline Tag fingerprintAuthenticateReason = Tag::fingerprintAuthenticateReason;

  template<typename _Tp>
  static constexpr bool _not_self = !std::is_same_v<std::remove_cv_t<std::remove_reference_t<_Tp>>, AuthenticateReason>;

  AuthenticateReason() : _value(std::in_place_index<static_cast<size_t>(vendorAuthenticateReason)>, ::aidl::android::hardware::biometrics::common::AuthenticateReason::Vendor()) { }

  template <typename _Tp, typename = std::enable_if_t<_not_self<_Tp>>>
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr AuthenticateReason(_Tp&& _arg)
      : _value(std::forward<_Tp>(_arg)) {}

  template <size_t _Np, typename... _Tp>
  constexpr explicit AuthenticateReason(std::in_place_index_t<_Np>, _Tp&&... _args)
      : _value(std::in_place_index<_Np>, std::forward<_Tp>(_args)...) {}

  template <Tag _tag, typename... _Tp>
  static AuthenticateReason make(_Tp&&... _args) {
    return AuthenticateReason(std::in_place_index<static_cast<size_t>(_tag)>, std::forward<_Tp>(_args)...);
  }

  template <Tag _tag, typename _Tp, typename... _Up>
  static AuthenticateReason make(std::initializer_list<_Tp> _il, _Up&&... _args) {
    return AuthenticateReason(std::in_place_index<static_cast<size_t>(_tag)>, std::move(_il), std::forward<_Up>(_args)...);
  }

  Tag getTag() const {
    return static_cast<Tag>(_value.index());
  }

  template <Tag _tag>
  const auto& get() const {
    if (getTag() != _tag) { __assert2(__FILE__, __LINE__, __PRETTY_FUNCTION__, "bad access: a wrong tag"); }
    return std::get<static_cast<size_t>(_tag)>(_value);
  }

  template <Tag _tag>
  auto& get() {
    if (getTag() != _tag) { __assert2(__FILE__, __LINE__, __PRETTY_FUNCTION__, "bad access: a wrong tag"); }
    return std::get<static_cast<size_t>(_tag)>(_value);
  }

  template <Tag _tag, typename... _Tp>
  void set(_Tp&&... _args) {
    _value.emplace<static_cast<size_t>(_tag)>(std::forward<_Tp>(_args)...);
  }

  binder_status_t readFromParcel(const AParcel* _parcel);
  binder_status_t writeToParcel(AParcel* _parcel) const;

  inline bool operator==(const AuthenticateReason& _rhs) const {
    return _value == _rhs._value;
  }
  inline bool operator<(const AuthenticateReason& _rhs) const {
    return _value < _rhs._value;
  }
  inline bool operator!=(const AuthenticateReason& _rhs) const {
    return !(*this == _rhs);
  }
  inline bool operator>(const AuthenticateReason& _rhs) const {
    return _rhs < *this;
  }
  inline bool operator>=(const AuthenticateReason& _rhs) const {
    return !(*this < _rhs);
  }
  inline bool operator<=(const AuthenticateReason& _rhs) const {
    return !(_rhs < *this);
  }

  static const ::ndk::parcelable_stability_t _aidl_stability = ::ndk::STABILITY_VINTF;
  inline std::string toString() const {
    std::ostringstream os;
    os << "AuthenticateReason{";
    switch (getTag()) {
    case vendorAuthenticateReason: os << "vendorAuthenticateReason: " << ::android::internal::ToString(get<vendorAuthenticateReason>()); break;
    case faceAuthenticateReason: os << "faceAuthenticateReason: " << ::android::internal::ToString(get<faceAuthenticateReason>()); break;
    case fingerprintAuthenticateReason: os << "fingerprintAuthenticateReason: " << ::android::internal::ToString(get<fingerprintAuthenticateReason>()); break;
    }
    os << "}";
    return os.str();
  }
private:
  std::variant<::aidl::android::hardware::biometrics::common::AuthenticateReason::Vendor, ::aidl::android::hardware::biometrics::common::AuthenticateReason::Face, ::aidl::android::hardware::biometrics::common::AuthenticateReason::Fingerprint> _value;
};
}  // namespace common
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace common {
[[nodiscard]] static inline std::string toString(AuthenticateReason::Fingerprint val) {
  switch(val) {
  case AuthenticateReason::Fingerprint::UNKNOWN:
    return "UNKNOWN";
  default:
    return std::to_string(static_cast<int32_t>(val));
  }
}
}  // namespace common
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
namespace ndk {
namespace internal {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++17-extensions"
template <>
constexpr inline std::array<aidl::android::hardware::biometrics::common::AuthenticateReason::Fingerprint, 1> enum_values<aidl::android::hardware::biometrics::common::AuthenticateReason::Fingerprint> = {
  aidl::android::hardware::biometrics::common::AuthenticateReason::Fingerprint::UNKNOWN,
};
#pragma clang diagnostic pop
}  // namespace internal
}  // namespace ndk
namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace common {
[[nodiscard]] static inline std::string toString(AuthenticateReason::Face val) {
  switch(val) {
  case AuthenticateReason::Face::UNKNOWN:
    return "UNKNOWN";
  case AuthenticateReason::Face::STARTED_WAKING_UP:
    return "STARTED_WAKING_UP";
  case AuthenticateReason::Face::PRIMARY_BOUNCER_SHOWN:
    return "PRIMARY_BOUNCER_SHOWN";
  case AuthenticateReason::Face::ASSISTANT_VISIBLE:
    return "ASSISTANT_VISIBLE";
  case AuthenticateReason::Face::ALTERNATE_BIOMETRIC_BOUNCER_SHOWN:
    return "ALTERNATE_BIOMETRIC_BOUNCER_SHOWN";
  case AuthenticateReason::Face::NOTIFICATION_PANEL_CLICKED:
    return "NOTIFICATION_PANEL_CLICKED";
  case AuthenticateReason::Face::OCCLUDING_APP_REQUESTED:
    return "OCCLUDING_APP_REQUESTED";
  case AuthenticateReason::Face::PICK_UP_GESTURE_TRIGGERED:
    return "PICK_UP_GESTURE_TRIGGERED";
  case AuthenticateReason::Face::QS_EXPANDED:
    return "QS_EXPANDED";
  case AuthenticateReason::Face::SWIPE_UP_ON_BOUNCER:
    return "SWIPE_UP_ON_BOUNCER";
  case AuthenticateReason::Face::UDFPS_POINTER_DOWN:
    return "UDFPS_POINTER_DOWN";
  default:
    return std::to_string(static_cast<int32_t>(val));
  }
}
}  // namespace common
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
namespace ndk {
namespace internal {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++17-extensions"
template <>
constexpr inline std::array<aidl::android::hardware::biometrics::common::AuthenticateReason::Face, 11> enum_values<aidl::android::hardware::biometrics::common::AuthenticateReason::Face> = {
  aidl::android::hardware::biometrics::common::AuthenticateReason::Face::UNKNOWN,
  aidl::android::hardware::biometrics::common::AuthenticateReason::Face::STARTED_WAKING_UP,
  aidl::android::hardware::biometrics::common::AuthenticateReason::Face::PRIMARY_BOUNCER_SHOWN,
  aidl::android::hardware::biometrics::common::AuthenticateReason::Face::ASSISTANT_VISIBLE,
  aidl::android::hardware::biometrics::common::AuthenticateReason::Face::ALTERNATE_BIOMETRIC_BOUNCER_SHOWN,
  aidl::android::hardware::biometrics::common::AuthenticateReason::Face::NOTIFICATION_PANEL_CLICKED,
  aidl::android::hardware::biometrics::common::AuthenticateReason::Face::OCCLUDING_APP_REQUESTED,
  aidl::android::hardware::biometrics::common::AuthenticateReason::Face::PICK_UP_GESTURE_TRIGGERED,
  aidl::android::hardware::biometrics::common::AuthenticateReason::Face::QS_EXPANDED,
  aidl::android::hardware::biometrics::common::AuthenticateReason::Face::SWIPE_UP_ON_BOUNCER,
  aidl::android::hardware::biometrics::common::AuthenticateReason::Face::UDFPS_POINTER_DOWN,
};
#pragma clang diagnostic pop
}  // namespace internal
}  // namespace ndk
namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace common {
[[nodiscard]] static inline std::string toString(AuthenticateReason::Tag val) {
  switch(val) {
  case AuthenticateReason::Tag::vendorAuthenticateReason:
    return "vendorAuthenticateReason";
  case AuthenticateReason::Tag::faceAuthenticateReason:
    return "faceAuthenticateReason";
  case AuthenticateReason::Tag::fingerprintAuthenticateReason:
    return "fingerprintAuthenticateReason";
  default:
    return std::to_string(static_cast<int32_t>(val));
  }
}
}  // namespace common
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
namespace ndk {
namespace internal {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++17-extensions"
template <>
constexpr inline std::array<aidl::android::hardware::biometrics::common::AuthenticateReason::Tag, 3> enum_values<aidl::android::hardware::biometrics::common::AuthenticateReason::Tag> = {
  aidl::android::hardware::biometrics::common::AuthenticateReason::Tag::vendorAuthenticateReason,
  aidl::android::hardware::biometrics::common::AuthenticateReason::Tag::faceAuthenticateReason,
  aidl::android::hardware::biometrics::common::AuthenticateReason::Tag::fingerprintAuthenticateReason,
};
#pragma clang diagnostic pop
}  // namespace internal
}  // namespace ndk
