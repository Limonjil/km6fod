/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: C:\Users\Limonjil\Desktop\android\android-sdk\build-tools\35.0.0\aidl.exe --lang=ndk --structured --stability=vintf --min_sdk_version=31 -v 5 -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\keymaster\aidl -h C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\include -o C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\out C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl\android\hardware\biometrics\fingerprint\AcquiredInfo.aidl
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
enum class AcquiredInfo : int8_t {
  UNKNOWN = 0,
  GOOD = 1,
  PARTIAL = 2,
  INSUFFICIENT = 3,
  SENSOR_DIRTY = 4,
  TOO_SLOW = 5,
  TOO_FAST = 6,
  VENDOR = 7,
  START = 8,
  TOO_DARK = 9,
  TOO_BRIGHT = 10,
  IMMOBILE = 11,
  RETRYING_CAPTURE = 12,
  LIFT_TOO_SOON = 13,
  POWER_PRESS = 14,
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
[[nodiscard]] static inline std::string toString(AcquiredInfo val) {
  switch(val) {
  case AcquiredInfo::UNKNOWN:
    return "UNKNOWN";
  case AcquiredInfo::GOOD:
    return "GOOD";
  case AcquiredInfo::PARTIAL:
    return "PARTIAL";
  case AcquiredInfo::INSUFFICIENT:
    return "INSUFFICIENT";
  case AcquiredInfo::SENSOR_DIRTY:
    return "SENSOR_DIRTY";
  case AcquiredInfo::TOO_SLOW:
    return "TOO_SLOW";
  case AcquiredInfo::TOO_FAST:
    return "TOO_FAST";
  case AcquiredInfo::VENDOR:
    return "VENDOR";
  case AcquiredInfo::START:
    return "START";
  case AcquiredInfo::TOO_DARK:
    return "TOO_DARK";
  case AcquiredInfo::TOO_BRIGHT:
    return "TOO_BRIGHT";
  case AcquiredInfo::IMMOBILE:
    return "IMMOBILE";
  case AcquiredInfo::RETRYING_CAPTURE:
    return "RETRYING_CAPTURE";
  case AcquiredInfo::LIFT_TOO_SOON:
    return "LIFT_TOO_SOON";
  case AcquiredInfo::POWER_PRESS:
    return "POWER_PRESS";
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
constexpr inline std::array<aidl::android::hardware::biometrics::fingerprint::AcquiredInfo, 15> enum_values<aidl::android::hardware::biometrics::fingerprint::AcquiredInfo> = {
  aidl::android::hardware::biometrics::fingerprint::AcquiredInfo::UNKNOWN,
  aidl::android::hardware::biometrics::fingerprint::AcquiredInfo::GOOD,
  aidl::android::hardware::biometrics::fingerprint::AcquiredInfo::PARTIAL,
  aidl::android::hardware::biometrics::fingerprint::AcquiredInfo::INSUFFICIENT,
  aidl::android::hardware::biometrics::fingerprint::AcquiredInfo::SENSOR_DIRTY,
  aidl::android::hardware::biometrics::fingerprint::AcquiredInfo::TOO_SLOW,
  aidl::android::hardware::biometrics::fingerprint::AcquiredInfo::TOO_FAST,
  aidl::android::hardware::biometrics::fingerprint::AcquiredInfo::VENDOR,
  aidl::android::hardware::biometrics::fingerprint::AcquiredInfo::START,
  aidl::android::hardware::biometrics::fingerprint::AcquiredInfo::TOO_DARK,
  aidl::android::hardware::biometrics::fingerprint::AcquiredInfo::TOO_BRIGHT,
  aidl::android::hardware::biometrics::fingerprint::AcquiredInfo::IMMOBILE,
  aidl::android::hardware::biometrics::fingerprint::AcquiredInfo::RETRYING_CAPTURE,
  aidl::android::hardware::biometrics::fingerprint::AcquiredInfo::LIFT_TOO_SOON,
  aidl::android::hardware::biometrics::fingerprint::AcquiredInfo::POWER_PRESS,
};
#pragma clang diagnostic pop
}  // namespace internal
}  // namespace ndk
