/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/limonjil/android/sdk/build-tools/35.0.0/aidl --lang=ndk --min_sdk_version=31 --structured --stability vintf -o gen -Iflat --header_out=gen/include flat/android/hardware/biometrics/common/OperationState.aidl
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
#include <aidl/android/hardware/biometrics/common/OperationState.h>
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
class OperationState {
public:
  typedef std::false_type fixed_size;
  static const char* descriptor;

  class FingerprintOperationState {
  public:
    typedef std::false_type fixed_size;
    static const char* descriptor;

    ::ndk::AParcelableHolder extension{::ndk::STABILITY_VINTF};
    bool isHardwareIgnoringTouches = false;

    binder_status_t readFromParcel(const AParcel* parcel);
    binder_status_t writeToParcel(AParcel* parcel) const;

    inline bool operator==(const FingerprintOperationState& _rhs) const {
      return std::tie(extension, isHardwareIgnoringTouches) == std::tie(_rhs.extension, _rhs.isHardwareIgnoringTouches);
    }
    inline bool operator<(const FingerprintOperationState& _rhs) const {
      return std::tie(extension, isHardwareIgnoringTouches) < std::tie(_rhs.extension, _rhs.isHardwareIgnoringTouches);
    }
    inline bool operator!=(const FingerprintOperationState& _rhs) const {
      return !(*this == _rhs);
    }
    inline bool operator>(const FingerprintOperationState& _rhs) const {
      return _rhs < *this;
    }
    inline bool operator>=(const FingerprintOperationState& _rhs) const {
      return !(*this < _rhs);
    }
    inline bool operator<=(const FingerprintOperationState& _rhs) const {
      return !(_rhs < *this);
    }

    static const ::ndk::parcelable_stability_t _aidl_stability = ::ndk::STABILITY_VINTF;
    inline std::string toString() const {
      std::ostringstream _aidl_os;
      _aidl_os << "FingerprintOperationState{";
      _aidl_os << "extension: " << ::android::internal::ToString(extension);
      _aidl_os << ", isHardwareIgnoringTouches: " << ::android::internal::ToString(isHardwareIgnoringTouches);
      _aidl_os << "}";
      return _aidl_os.str();
    }
  };
  class FaceOperationState {
  public:
    typedef std::false_type fixed_size;
    static const char* descriptor;

    ::ndk::AParcelableHolder extension{::ndk::STABILITY_VINTF};

    binder_status_t readFromParcel(const AParcel* parcel);
    binder_status_t writeToParcel(AParcel* parcel) const;

    inline bool operator==(const FaceOperationState& _rhs) const {
      return std::tie(extension) == std::tie(_rhs.extension);
    }
    inline bool operator<(const FaceOperationState& _rhs) const {
      return std::tie(extension) < std::tie(_rhs.extension);
    }
    inline bool operator!=(const FaceOperationState& _rhs) const {
      return !(*this == _rhs);
    }
    inline bool operator>(const FaceOperationState& _rhs) const {
      return _rhs < *this;
    }
    inline bool operator>=(const FaceOperationState& _rhs) const {
      return !(*this < _rhs);
    }
    inline bool operator<=(const FaceOperationState& _rhs) const {
      return !(_rhs < *this);
    }

    static const ::ndk::parcelable_stability_t _aidl_stability = ::ndk::STABILITY_VINTF;
    inline std::string toString() const {
      std::ostringstream _aidl_os;
      _aidl_os << "FaceOperationState{";
      _aidl_os << "extension: " << ::android::internal::ToString(extension);
      _aidl_os << "}";
      return _aidl_os.str();
    }
  };
  enum class Tag : int32_t {
    fingerprintOperationState = 0,
    faceOperationState = 1,
  };

  // Expose tag symbols for legacy code
  static const inline Tag fingerprintOperationState = Tag::fingerprintOperationState;
  static const inline Tag faceOperationState = Tag::faceOperationState;

  template<typename _Tp>
  static constexpr bool _not_self = !std::is_same_v<std::remove_cv_t<std::remove_reference_t<_Tp>>, OperationState>;

  OperationState() : _value(std::in_place_index<static_cast<size_t>(fingerprintOperationState)>, ::aidl::android::hardware::biometrics::common::OperationState::FingerprintOperationState()) { }

  template <typename _Tp, typename = std::enable_if_t<_not_self<_Tp>>>
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr OperationState(_Tp&& _arg)
      : _value(std::forward<_Tp>(_arg)) {}

  template <size_t _Np, typename... _Tp>
  constexpr explicit OperationState(std::in_place_index_t<_Np>, _Tp&&... _args)
      : _value(std::in_place_index<_Np>, std::forward<_Tp>(_args)...) {}

  template <Tag _tag, typename... _Tp>
  static OperationState make(_Tp&&... _args) {
    return OperationState(std::in_place_index<static_cast<size_t>(_tag)>, std::forward<_Tp>(_args)...);
  }

  template <Tag _tag, typename _Tp, typename... _Up>
  static OperationState make(std::initializer_list<_Tp> _il, _Up&&... _args) {
    return OperationState(std::in_place_index<static_cast<size_t>(_tag)>, std::move(_il), std::forward<_Up>(_args)...);
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

  inline bool operator==(const OperationState& _rhs) const {
    return _value == _rhs._value;
  }
  inline bool operator<(const OperationState& _rhs) const {
    return _value < _rhs._value;
  }
  inline bool operator!=(const OperationState& _rhs) const {
    return !(*this == _rhs);
  }
  inline bool operator>(const OperationState& _rhs) const {
    return _rhs < *this;
  }
  inline bool operator>=(const OperationState& _rhs) const {
    return !(*this < _rhs);
  }
  inline bool operator<=(const OperationState& _rhs) const {
    return !(_rhs < *this);
  }

  static const ::ndk::parcelable_stability_t _aidl_stability = ::ndk::STABILITY_VINTF;
  inline std::string toString() const {
    std::ostringstream os;
    os << "OperationState{";
    switch (getTag()) {
    case fingerprintOperationState: os << "fingerprintOperationState: " << ::android::internal::ToString(get<fingerprintOperationState>()); break;
    case faceOperationState: os << "faceOperationState: " << ::android::internal::ToString(get<faceOperationState>()); break;
    }
    os << "}";
    return os.str();
  }
private:
  std::variant<::aidl::android::hardware::biometrics::common::OperationState::FingerprintOperationState, ::aidl::android::hardware::biometrics::common::OperationState::FaceOperationState> _value;
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
[[nodiscard]] static inline std::string toString(OperationState::Tag val) {
  switch(val) {
  case OperationState::Tag::fingerprintOperationState:
    return "fingerprintOperationState";
  case OperationState::Tag::faceOperationState:
    return "faceOperationState";
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
constexpr inline std::array<aidl::android::hardware::biometrics::common::OperationState::Tag, 2> enum_values<aidl::android::hardware::biometrics::common::OperationState::Tag> = {
  aidl::android::hardware::biometrics::common::OperationState::Tag::fingerprintOperationState,
  aidl::android::hardware::biometrics::common::OperationState::Tag::faceOperationState,
};
#pragma clang diagnostic pop
}  // namespace internal
}  // namespace ndk
