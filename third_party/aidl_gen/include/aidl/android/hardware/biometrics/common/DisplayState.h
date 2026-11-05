/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: C:\Users\Limonjil\Desktop\android\android-sdk\build-tools\35.0.0\aidl.exe --lang=ndk --structured --stability=vintf --min_sdk_version=31 -v 5 -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\keymaster\aidl -h C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\include -o C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\out C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl\android\hardware\biometrics\common\DisplayState.aidl
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
enum class DisplayState : int32_t {
  UNKNOWN = 0,
  LOCKSCREEN = 1,
  NO_UI = 2,
  SCREENSAVER = 3,
  AOD = 4,
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
[[nodiscard]] static inline std::string toString(DisplayState val) {
  switch(val) {
  case DisplayState::UNKNOWN:
    return "UNKNOWN";
  case DisplayState::LOCKSCREEN:
    return "LOCKSCREEN";
  case DisplayState::NO_UI:
    return "NO_UI";
  case DisplayState::SCREENSAVER:
    return "SCREENSAVER";
  case DisplayState::AOD:
    return "AOD";
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
constexpr inline std::array<aidl::android::hardware::biometrics::common::DisplayState, 5> enum_values<aidl::android::hardware::biometrics::common::DisplayState> = {
  aidl::android::hardware::biometrics::common::DisplayState::UNKNOWN,
  aidl::android::hardware::biometrics::common::DisplayState::LOCKSCREEN,
  aidl::android::hardware::biometrics::common::DisplayState::NO_UI,
  aidl::android::hardware::biometrics::common::DisplayState::SCREENSAVER,
  aidl::android::hardware::biometrics::common::DisplayState::AOD,
};
#pragma clang diagnostic pop
}  // namespace internal
}  // namespace ndk
