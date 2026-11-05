/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/limonjil/android/sdk/build-tools/35.0.0/aidl --lang=ndk --min_sdk_version=31 --structured --stability vintf -o gen -Iflat --header_out=gen/include flat/android/hardware/biometrics/fingerprint/FingerprintSensorType.aidl
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
enum class FingerprintSensorType : int8_t {
  UNKNOWN = 0,
  REAR = 1,
  UNDER_DISPLAY_ULTRASONIC = 2,
  UNDER_DISPLAY_OPTICAL = 3,
  POWER_BUTTON = 4,
  HOME_BUTTON = 5,
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
[[nodiscard]] static inline std::string toString(FingerprintSensorType val) {
  switch(val) {
  case FingerprintSensorType::UNKNOWN:
    return "UNKNOWN";
  case FingerprintSensorType::REAR:
    return "REAR";
  case FingerprintSensorType::UNDER_DISPLAY_ULTRASONIC:
    return "UNDER_DISPLAY_ULTRASONIC";
  case FingerprintSensorType::UNDER_DISPLAY_OPTICAL:
    return "UNDER_DISPLAY_OPTICAL";
  case FingerprintSensorType::POWER_BUTTON:
    return "POWER_BUTTON";
  case FingerprintSensorType::HOME_BUTTON:
    return "HOME_BUTTON";
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
constexpr inline std::array<aidl::android::hardware::biometrics::fingerprint::FingerprintSensorType, 6> enum_values<aidl::android::hardware::biometrics::fingerprint::FingerprintSensorType> = {
  aidl::android::hardware::biometrics::fingerprint::FingerprintSensorType::UNKNOWN,
  aidl::android::hardware::biometrics::fingerprint::FingerprintSensorType::REAR,
  aidl::android::hardware::biometrics::fingerprint::FingerprintSensorType::UNDER_DISPLAY_ULTRASONIC,
  aidl::android::hardware::biometrics::fingerprint::FingerprintSensorType::UNDER_DISPLAY_OPTICAL,
  aidl::android::hardware::biometrics::fingerprint::FingerprintSensorType::POWER_BUTTON,
  aidl::android::hardware::biometrics::fingerprint::FingerprintSensorType::HOME_BUTTON,
};
#pragma clang diagnostic pop
}  // namespace internal
}  // namespace ndk
