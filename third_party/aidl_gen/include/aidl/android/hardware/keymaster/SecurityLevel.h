/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: C:\Users\Limonjil\Desktop\android\android-sdk\build-tools\35.0.0\aidl.exe --lang=ndk --structured --stability=vintf --min_sdk_version=31 -v 5 -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\keymaster\aidl -h C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\include -o C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\out C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\keymaster\aidl\android\hardware\keymaster\SecurityLevel.aidl
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
namespace keymaster {
enum class SecurityLevel : int32_t {
  SOFTWARE = 0,
  TRUSTED_ENVIRONMENT = 1,
  STRONGBOX = 2,
};

}  // namespace keymaster
}  // namespace hardware
}  // namespace android
}  // namespace aidl
namespace aidl {
namespace android {
namespace hardware {
namespace keymaster {
[[nodiscard]] static inline std::string toString(SecurityLevel val) {
  switch(val) {
  case SecurityLevel::SOFTWARE:
    return "SOFTWARE";
  case SecurityLevel::TRUSTED_ENVIRONMENT:
    return "TRUSTED_ENVIRONMENT";
  case SecurityLevel::STRONGBOX:
    return "STRONGBOX";
  default:
    return std::to_string(static_cast<int32_t>(val));
  }
}
}  // namespace keymaster
}  // namespace hardware
}  // namespace android
}  // namespace aidl
namespace ndk {
namespace internal {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++17-extensions"
template <>
constexpr inline std::array<aidl::android::hardware::keymaster::SecurityLevel, 3> enum_values<aidl::android::hardware::keymaster::SecurityLevel> = {
  aidl::android::hardware::keymaster::SecurityLevel::SOFTWARE,
  aidl::android::hardware::keymaster::SecurityLevel::TRUSTED_ENVIRONMENT,
  aidl::android::hardware::keymaster::SecurityLevel::STRONGBOX,
};
#pragma clang diagnostic pop
}  // namespace internal
}  // namespace ndk
