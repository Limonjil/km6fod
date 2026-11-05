/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: C:\Users\Limonjil\Desktop\android\android-sdk\build-tools\35.0.0\aidl.exe --lang=ndk --structured --stability=vintf --min_sdk_version=31 -v 5 -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\keymaster\aidl -h C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\include -o C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\out C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl\android\hardware\biometrics\common\OperationReason.aidl
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
enum class OperationReason : int8_t {
  UNKNOWN = 0,
  BIOMETRIC_PROMPT = 1,
  KEYGUARD = 2,
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
[[nodiscard]] static inline std::string toString(OperationReason val) {
  switch(val) {
  case OperationReason::UNKNOWN:
    return "UNKNOWN";
  case OperationReason::BIOMETRIC_PROMPT:
    return "BIOMETRIC_PROMPT";
  case OperationReason::KEYGUARD:
    return "KEYGUARD";
  default:
    return std::to_string(static_cast<int8_t>(val));
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
constexpr inline std::array<aidl::android::hardware::biometrics::common::OperationReason, 3> enum_values<aidl::android::hardware::biometrics::common::OperationReason> = {
  aidl::android::hardware::biometrics::common::OperationReason::UNKNOWN,
  aidl::android::hardware::biometrics::common::OperationReason::BIOMETRIC_PROMPT,
  aidl::android::hardware::biometrics::common::OperationReason::KEYGUARD,
};
#pragma clang diagnostic pop
}  // namespace internal
}  // namespace ndk
