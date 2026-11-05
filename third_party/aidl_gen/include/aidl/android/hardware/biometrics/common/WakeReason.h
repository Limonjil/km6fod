/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: C:\Users\Limonjil\Desktop\android\android-sdk\build-tools\35.0.0\aidl.exe --lang=ndk --structured --stability=vintf --min_sdk_version=31 -v 5 -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\keymaster\aidl -h C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\include -o C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\out C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl\android\hardware\biometrics\common\WakeReason.aidl
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
namespace common {
enum class WakeReason : int32_t {
  UNKNOWN = 0,
  POWER_BUTTON = 1,
  GESTURE = 2,
  WAKE_KEY = 3,
  WAKE_MOTION = 4,
  LID = 5,
  DISPLAY_GROUP_ADDED = 6,
  TAP = 7,
  LIFT = 8,
  BIOMETRIC = 9,
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
[[nodiscard]] static inline std::string toString(WakeReason val) {
  switch(val) {
  case WakeReason::UNKNOWN:
    return "UNKNOWN";
  case WakeReason::POWER_BUTTON:
    return "POWER_BUTTON";
  case WakeReason::GESTURE:
    return "GESTURE";
  case WakeReason::WAKE_KEY:
    return "WAKE_KEY";
  case WakeReason::WAKE_MOTION:
    return "WAKE_MOTION";
  case WakeReason::LID:
    return "LID";
  case WakeReason::DISPLAY_GROUP_ADDED:
    return "DISPLAY_GROUP_ADDED";
  case WakeReason::TAP:
    return "TAP";
  case WakeReason::LIFT:
    return "LIFT";
  case WakeReason::BIOMETRIC:
    return "BIOMETRIC";
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
constexpr inline std::array<aidl::android::hardware::biometrics::common::WakeReason, 10> enum_values<aidl::android::hardware::biometrics::common::WakeReason> = {
  aidl::android::hardware::biometrics::common::WakeReason::UNKNOWN,
  aidl::android::hardware::biometrics::common::WakeReason::POWER_BUTTON,
  aidl::android::hardware::biometrics::common::WakeReason::GESTURE,
  aidl::android::hardware::biometrics::common::WakeReason::WAKE_KEY,
  aidl::android::hardware::biometrics::common::WakeReason::WAKE_MOTION,
  aidl::android::hardware::biometrics::common::WakeReason::LID,
  aidl::android::hardware::biometrics::common::WakeReason::DISPLAY_GROUP_ADDED,
  aidl::android::hardware::biometrics::common::WakeReason::TAP,
  aidl::android::hardware::biometrics::common::WakeReason::LIFT,
  aidl::android::hardware::biometrics::common::WakeReason::BIOMETRIC,
};
#pragma clang diagnostic pop
}  // namespace internal
}  // namespace ndk
