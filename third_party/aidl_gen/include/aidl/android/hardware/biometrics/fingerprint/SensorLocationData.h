/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: C:\Users\Limonjil\Desktop\android\android-sdk\build-tools\35.0.0\aidl.exe --lang=ndk --structured --stability=vintf --min_sdk_version=31 -v 5 -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\keymaster\aidl -h C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\include -o C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\out C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl\android\hardware\biometrics\fingerprint\SensorLocationData.aidl
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
#include <aidl/android/hardware/biometrics/fingerprint/location/HomeButtonLocation.h>
#include <aidl/android/hardware/biometrics/fingerprint/location/PowerButtonDisplayLocation.h>
#include <aidl/android/hardware/biometrics/fingerprint/location/PowerButtonPhysicalLocation.h>
#include <aidl/android/hardware/biometrics/fingerprint/location/RearLocation.h>
#include <aidl/android/hardware/biometrics/fingerprint/location/StandaloneLocation.h>
#include <aidl/android/hardware/biometrics/fingerprint/location/UnderDisplayLocation.h>
#ifdef BINDER_STABILITY_SUPPORT
#include <android/binder_stability.h>
#endif  // BINDER_STABILITY_SUPPORT

#ifndef __BIONIC__
#define __assert2(a,b,c,d) ((void)0)
#endif

namespace aidl::android::hardware::biometrics::fingerprint::location {
class HomeButtonLocation;
class PowerButtonDisplayLocation;
class PowerButtonPhysicalLocation;
class RearLocation;
class StandaloneLocation;
class UnderDisplayLocation;
}  // namespace aidl::android::hardware::biometrics::fingerprint::location
namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {
class SensorLocationData {
public:
  typedef std::false_type fixed_size;
  static const char* descriptor;

  enum class Tag : int32_t {
    underDisplayLocation = 0,
    powerButtonDisplayLocation = 1,
    powerButtonPhysicalLocation = 2,
    standaloneLocation = 3,
    homeButtonLocation = 4,
    rearLocation = 5,
  };

  // Expose tag symbols for legacy code
  static const inline Tag underDisplayLocation = Tag::underDisplayLocation;
  static const inline Tag powerButtonDisplayLocation = Tag::powerButtonDisplayLocation;
  static const inline Tag powerButtonPhysicalLocation = Tag::powerButtonPhysicalLocation;
  static const inline Tag standaloneLocation = Tag::standaloneLocation;
  static const inline Tag homeButtonLocation = Tag::homeButtonLocation;
  static const inline Tag rearLocation = Tag::rearLocation;

  template<typename _Tp>
  static constexpr bool _not_self = !std::is_same_v<std::remove_cv_t<std::remove_reference_t<_Tp>>, SensorLocationData>;

  SensorLocationData() : _value(std::in_place_index<static_cast<size_t>(underDisplayLocation)>, ::aidl::android::hardware::biometrics::fingerprint::location::UnderDisplayLocation()) { }

  template <typename _Tp, typename = std::enable_if_t<_not_self<_Tp>>>
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr SensorLocationData(_Tp&& _arg)
      : _value(std::forward<_Tp>(_arg)) {}

  template <size_t _Np, typename... _Tp>
  constexpr explicit SensorLocationData(std::in_place_index_t<_Np>, _Tp&&... _args)
      : _value(std::in_place_index<_Np>, std::forward<_Tp>(_args)...) {}

  template <Tag _tag, typename... _Tp>
  static SensorLocationData make(_Tp&&... _args) {
    return SensorLocationData(std::in_place_index<static_cast<size_t>(_tag)>, std::forward<_Tp>(_args)...);
  }

  template <Tag _tag, typename _Tp, typename... _Up>
  static SensorLocationData make(std::initializer_list<_Tp> _il, _Up&&... _args) {
    return SensorLocationData(std::in_place_index<static_cast<size_t>(_tag)>, std::move(_il), std::forward<_Up>(_args)...);
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

  inline bool operator==(const SensorLocationData& _rhs) const {
    return _value == _rhs._value;
  }
  inline bool operator<(const SensorLocationData& _rhs) const {
    return _value < _rhs._value;
  }
  inline bool operator!=(const SensorLocationData& _rhs) const {
    return !(*this == _rhs);
  }
  inline bool operator>(const SensorLocationData& _rhs) const {
    return _rhs < *this;
  }
  inline bool operator>=(const SensorLocationData& _rhs) const {
    return !(*this < _rhs);
  }
  inline bool operator<=(const SensorLocationData& _rhs) const {
    return !(_rhs < *this);
  }

  static const ::ndk::parcelable_stability_t _aidl_stability = ::ndk::STABILITY_VINTF;
  inline std::string toString() const {
    std::ostringstream os;
    os << "SensorLocationData{";
    switch (getTag()) {
    case underDisplayLocation: os << "underDisplayLocation: " << ::android::internal::ToString(get<underDisplayLocation>()); break;
    case powerButtonDisplayLocation: os << "powerButtonDisplayLocation: " << ::android::internal::ToString(get<powerButtonDisplayLocation>()); break;
    case powerButtonPhysicalLocation: os << "powerButtonPhysicalLocation: " << ::android::internal::ToString(get<powerButtonPhysicalLocation>()); break;
    case standaloneLocation: os << "standaloneLocation: " << ::android::internal::ToString(get<standaloneLocation>()); break;
    case homeButtonLocation: os << "homeButtonLocation: " << ::android::internal::ToString(get<homeButtonLocation>()); break;
    case rearLocation: os << "rearLocation: " << ::android::internal::ToString(get<rearLocation>()); break;
    }
    os << "}";
    return os.str();
  }
private:
  std::variant<::aidl::android::hardware::biometrics::fingerprint::location::UnderDisplayLocation, ::aidl::android::hardware::biometrics::fingerprint::location::PowerButtonDisplayLocation, ::aidl::android::hardware::biometrics::fingerprint::location::PowerButtonPhysicalLocation, ::aidl::android::hardware::biometrics::fingerprint::location::StandaloneLocation, ::aidl::android::hardware::biometrics::fingerprint::location::HomeButtonLocation, ::aidl::android::hardware::biometrics::fingerprint::location::RearLocation> _value;
};
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
[[nodiscard]] static inline std::string toString(SensorLocationData::Tag val) {
  switch(val) {
  case SensorLocationData::Tag::underDisplayLocation:
    return "underDisplayLocation";
  case SensorLocationData::Tag::powerButtonDisplayLocation:
    return "powerButtonDisplayLocation";
  case SensorLocationData::Tag::powerButtonPhysicalLocation:
    return "powerButtonPhysicalLocation";
  case SensorLocationData::Tag::standaloneLocation:
    return "standaloneLocation";
  case SensorLocationData::Tag::homeButtonLocation:
    return "homeButtonLocation";
  case SensorLocationData::Tag::rearLocation:
    return "rearLocation";
  default:
    return std::to_string(static_cast<int32_t>(val));
  }
}
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
constexpr inline std::array<aidl::android::hardware::biometrics::fingerprint::SensorLocationData::Tag, 6> enum_values<aidl::android::hardware::biometrics::fingerprint::SensorLocationData::Tag> = {
  aidl::android::hardware::biometrics::fingerprint::SensorLocationData::Tag::underDisplayLocation,
  aidl::android::hardware::biometrics::fingerprint::SensorLocationData::Tag::powerButtonDisplayLocation,
  aidl::android::hardware::biometrics::fingerprint::SensorLocationData::Tag::powerButtonPhysicalLocation,
  aidl::android::hardware::biometrics::fingerprint::SensorLocationData::Tag::standaloneLocation,
  aidl::android::hardware::biometrics::fingerprint::SensorLocationData::Tag::homeButtonLocation,
  aidl::android::hardware::biometrics::fingerprint::SensorLocationData::Tag::rearLocation,
};
#pragma clang diagnostic pop
}  // namespace internal
}  // namespace ndk
