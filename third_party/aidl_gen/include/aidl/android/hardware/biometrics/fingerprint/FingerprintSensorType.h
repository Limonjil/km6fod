/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: C:\Users\Limonjil\Desktop\android\android-sdk\build-tools\35.0.0\aidl.exe --lang=ndk --structured --stability=vintf --min_sdk_version=31 -v 5 -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\keymaster\aidl -h C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\include -o C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\out C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl\android\hardware\biometrics\fingerprint\FingerprintSensorType.aidl
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
  STANDALONE = 6,
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
  case FingerprintSensorType::STANDALONE:
    return "STANDALONE";
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
constexpr inline std::array<aidl::android::hardware::biometrics::fingerprint::FingerprintSensorType, 7> enum_values<aidl::android::hardware::biometrics::fingerprint::FingerprintSensorType> = {
  aidl::android::hardware::biometrics::fingerprint::FingerprintSensorType::UNKNOWN,
  aidl::android::hardware::biometrics::fingerprint::FingerprintSensorType::REAR,
  aidl::android::hardware::biometrics::fingerprint::FingerprintSensorType::UNDER_DISPLAY_ULTRASONIC,
  aidl::android::hardware::biometrics::fingerprint::FingerprintSensorType::UNDER_DISPLAY_OPTICAL,
  aidl::android::hardware::biometrics::fingerprint::FingerprintSensorType::POWER_BUTTON,
  aidl::android::hardware::biometrics::fingerprint::FingerprintSensorType::HOME_BUTTON,
  aidl::android::hardware::biometrics::fingerprint::FingerprintSensorType::STANDALONE,
};
#pragma clang diagnostic pop
}  // namespace internal
}  // namespace ndk
