/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/limonjil/android/sdk/build-tools/35.0.0/aidl --lang=ndk --min_sdk_version=31 --structured --stability vintf -o gen -Iflat --header_out=gen/include flat/android/hardware/biometrics/fingerprint/SensorShape.aidl
 */
#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <android/binder_enums.h>
#ifdef BINDER_STABILITY_SUPPORT
#include <android/binder_stability.h>
#endif  // BINDER_STABILITY_SUPPORT

namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {
enum class SensorShape : int8_t {
  SQUARE = 0,
  CIRCLE = 1,
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
[[nodiscard]] static inline std::string toString(SensorShape val) {
  switch(val) {
  case SensorShape::SQUARE:
    return "SQUARE";
  case SensorShape::CIRCLE:
    return "CIRCLE";
  default:
    return std::to_string(static_cast<int8_t>(val));
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
constexpr inline std::array<aidl::android::hardware::biometrics::fingerprint::SensorShape, 2> enum_values<aidl::android::hardware::biometrics::fingerprint::SensorShape> = {
  aidl::android::hardware::biometrics::fingerprint::SensorShape::SQUARE,
  aidl::android::hardware::biometrics::fingerprint::SensorShape::CIRCLE,
};
#pragma clang diagnostic pop
}  // namespace internal
}  // namespace ndk
