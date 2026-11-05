/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/limonjil/android/sdk/build-tools/35.0.0/aidl --lang=ndk --min_sdk_version=31 -o gen -Iflat --header_out=gen/include flat/android/hardware/biometrics/fingerprint/virtualhal/AcquiredInfoAndVendorCode.aidl
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
#include <aidl/android/hardware/biometrics/fingerprint/AcquiredInfo.h>
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
namespace fingerprint {
namespace virtualhal {
class AcquiredInfoAndVendorCode {
public:
  typedef std::false_type fixed_size;
  static const char* descriptor;

  enum class Tag : int32_t {
    acquiredInfo = 0,
    vendorCode = 1,
  };

  // Expose tag symbols for legacy code
  static const inline Tag acquiredInfo = Tag::acquiredInfo;
  static const inline Tag vendorCode = Tag::vendorCode;

  template<typename _Tp>
  static constexpr bool _not_self = !std::is_same_v<std::remove_cv_t<std::remove_reference_t<_Tp>>, AcquiredInfoAndVendorCode>;

  AcquiredInfoAndVendorCode() : _value(std::in_place_index<static_cast<size_t>(acquiredInfo)>, ::aidl::android::hardware::biometrics::fingerprint::AcquiredInfo(::aidl::android::hardware::biometrics::fingerprint::AcquiredInfo::UNKNOWN)) { }

  template <typename _Tp, typename = std::enable_if_t<_not_self<_Tp>>>
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr AcquiredInfoAndVendorCode(_Tp&& _arg)
      : _value(std::forward<_Tp>(_arg)) {}

  template <size_t _Np, typename... _Tp>
  constexpr explicit AcquiredInfoAndVendorCode(std::in_place_index_t<_Np>, _Tp&&... _args)
      : _value(std::in_place_index<_Np>, std::forward<_Tp>(_args)...) {}

  template <Tag _tag, typename... _Tp>
  static AcquiredInfoAndVendorCode make(_Tp&&... _args) {
    return AcquiredInfoAndVendorCode(std::in_place_index<static_cast<size_t>(_tag)>, std::forward<_Tp>(_args)...);
  }

  template <Tag _tag, typename _Tp, typename... _Up>
  static AcquiredInfoAndVendorCode make(std::initializer_list<_Tp> _il, _Up&&... _args) {
    return AcquiredInfoAndVendorCode(std::in_place_index<static_cast<size_t>(_tag)>, std::move(_il), std::forward<_Up>(_args)...);
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

  inline bool operator==(const AcquiredInfoAndVendorCode& _rhs) const {
    return _value == _rhs._value;
  }
  inline bool operator<(const AcquiredInfoAndVendorCode& _rhs) const {
    return _value < _rhs._value;
  }
  inline bool operator!=(const AcquiredInfoAndVendorCode& _rhs) const {
    return !(*this == _rhs);
  }
  inline bool operator>(const AcquiredInfoAndVendorCode& _rhs) const {
    return _rhs < *this;
  }
  inline bool operator>=(const AcquiredInfoAndVendorCode& _rhs) const {
    return !(*this < _rhs);
  }
  inline bool operator<=(const AcquiredInfoAndVendorCode& _rhs) const {
    return !(_rhs < *this);
  }

  static const ::ndk::parcelable_stability_t _aidl_stability = ::ndk::STABILITY_LOCAL;
  inline std::string toString() const {
    std::ostringstream os;
    os << "AcquiredInfoAndVendorCode{";
    switch (getTag()) {
    case acquiredInfo: os << "acquiredInfo: " << ::android::internal::ToString(get<acquiredInfo>()); break;
    case vendorCode: os << "vendorCode: " << ::android::internal::ToString(get<vendorCode>()); break;
    }
    os << "}";
    return os.str();
  }
private:
  std::variant<::aidl::android::hardware::biometrics::fingerprint::AcquiredInfo, int32_t> _value;
};
}  // namespace virtualhal
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {
namespace virtualhal {
[[nodiscard]] static inline std::string toString(AcquiredInfoAndVendorCode::Tag val) {
  switch(val) {
  case AcquiredInfoAndVendorCode::Tag::acquiredInfo:
    return "acquiredInfo";
  case AcquiredInfoAndVendorCode::Tag::vendorCode:
    return "vendorCode";
  default:
    return std::to_string(static_cast<int32_t>(val));
  }
}
}  // namespace virtualhal
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
namespace ndk {
namespace internal {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++17-extensions"
template <>
constexpr inline std::array<aidl::android::hardware::biometrics::fingerprint::virtualhal::AcquiredInfoAndVendorCode::Tag, 2> enum_values<aidl::android::hardware::biometrics::fingerprint::virtualhal::AcquiredInfoAndVendorCode::Tag> = {
  aidl::android::hardware::biometrics::fingerprint::virtualhal::AcquiredInfoAndVendorCode::Tag::acquiredInfo,
  aidl::android::hardware::biometrics::fingerprint::virtualhal::AcquiredInfoAndVendorCode::Tag::vendorCode,
};
#pragma clang diagnostic pop
}  // namespace internal
}  // namespace ndk
