/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: C:\Users\Limonjil\Desktop\android\android-sdk\build-tools\35.0.0\aidl.exe --lang=ndk --structured --stability=vintf --min_sdk_version=31 -v 5 -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\keymaster\aidl -h C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\include -o C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\out C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl\android\hardware\biometrics\fingerprint\location\PhysicalSensorLocation.aidl
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
namespace location {
enum class PhysicalSensorLocation : int8_t {
  UNKNOWN = 0,
  KEYBOARD_BOTTOM_LEFT = 1,
  KEYBOARD_BOTTOM_RIGHT = 2,
  KEYBOARD_TOP_RIGHT = 3,
  RIGHT_SIDE = 4,
  LEFT_SIDE = 5,
  LEFT_OF_POWER_BUTTON_TOP_RIGHT = 6,
  POWER_BUTTON_TOP_RIGHT_KEY = 7,
};

}  // namespace location
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
namespace location {
[[nodiscard]] static inline std::string toString(PhysicalSensorLocation val) {
  switch(val) {
  case PhysicalSensorLocation::UNKNOWN:
    return "UNKNOWN";
  case PhysicalSensorLocation::KEYBOARD_BOTTOM_LEFT:
    return "KEYBOARD_BOTTOM_LEFT";
  case PhysicalSensorLocation::KEYBOARD_BOTTOM_RIGHT:
    return "KEYBOARD_BOTTOM_RIGHT";
  case PhysicalSensorLocation::KEYBOARD_TOP_RIGHT:
    return "KEYBOARD_TOP_RIGHT";
  case PhysicalSensorLocation::RIGHT_SIDE:
    return "RIGHT_SIDE";
  case PhysicalSensorLocation::LEFT_SIDE:
    return "LEFT_SIDE";
  case PhysicalSensorLocation::LEFT_OF_POWER_BUTTON_TOP_RIGHT:
    return "LEFT_OF_POWER_BUTTON_TOP_RIGHT";
  case PhysicalSensorLocation::POWER_BUTTON_TOP_RIGHT_KEY:
    return "POWER_BUTTON_TOP_RIGHT_KEY";
  default:
    return std::to_string(static_cast<int8_t>(val));
  }
}
}  // namespace location
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
constexpr inline std::array<aidl::android::hardware::biometrics::fingerprint::location::PhysicalSensorLocation, 8> enum_values<aidl::android::hardware::biometrics::fingerprint::location::PhysicalSensorLocation> = {
  aidl::android::hardware::biometrics::fingerprint::location::PhysicalSensorLocation::UNKNOWN,
  aidl::android::hardware::biometrics::fingerprint::location::PhysicalSensorLocation::KEYBOARD_BOTTOM_LEFT,
  aidl::android::hardware::biometrics::fingerprint::location::PhysicalSensorLocation::KEYBOARD_BOTTOM_RIGHT,
  aidl::android::hardware::biometrics::fingerprint::location::PhysicalSensorLocation::KEYBOARD_TOP_RIGHT,
  aidl::android::hardware::biometrics::fingerprint::location::PhysicalSensorLocation::RIGHT_SIDE,
  aidl::android::hardware::biometrics::fingerprint::location::PhysicalSensorLocation::LEFT_SIDE,
  aidl::android::hardware::biometrics::fingerprint::location::PhysicalSensorLocation::LEFT_OF_POWER_BUTTON_TOP_RIGHT,
  aidl::android::hardware::biometrics::fingerprint::location::PhysicalSensorLocation::POWER_BUTTON_TOP_RIGHT_KEY,
};
#pragma clang diagnostic pop
}  // namespace internal
}  // namespace ndk
