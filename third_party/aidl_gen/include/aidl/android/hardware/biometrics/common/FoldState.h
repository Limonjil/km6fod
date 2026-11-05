/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: C:\Users\Limonjil\Desktop\android\android-sdk\build-tools\35.0.0\aidl.exe --lang=ndk --structured --stability=vintf --min_sdk_version=31 -v 5 -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\keymaster\aidl -h C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\include -o C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\out C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl\android\hardware\biometrics\common\FoldState.aidl
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
enum class FoldState : int32_t {
  UNKNOWN = 0,
  HALF_OPENED = 1,
  FULLY_OPENED = 2,
  FULLY_CLOSED = 3,
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
[[nodiscard]] static inline std::string toString(FoldState val) {
  switch(val) {
  case FoldState::UNKNOWN:
    return "UNKNOWN";
  case FoldState::HALF_OPENED:
    return "HALF_OPENED";
  case FoldState::FULLY_OPENED:
    return "FULLY_OPENED";
  case FoldState::FULLY_CLOSED:
    return "FULLY_CLOSED";
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
constexpr inline std::array<aidl::android::hardware::biometrics::common::FoldState, 4> enum_values<aidl::android::hardware::biometrics::common::FoldState> = {
  aidl::android::hardware::biometrics::common::FoldState::UNKNOWN,
  aidl::android::hardware::biometrics::common::FoldState::HALF_OPENED,
  aidl::android::hardware::biometrics::common::FoldState::FULLY_OPENED,
  aidl::android::hardware::biometrics::common::FoldState::FULLY_CLOSED,
};
#pragma clang diagnostic pop
}  // namespace internal
}  // namespace ndk
