/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: C:\Users\Limonjil\Desktop\android\android-sdk\build-tools\35.0.0\aidl.exe --lang=ndk --structured --stability=vintf --min_sdk_version=31 -v 5 -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\keymaster\aidl -h C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\include -o C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\out C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl\android\hardware\biometrics\fingerprint\Error.aidl
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
enum class Error : int8_t {
  UNKNOWN = 0,
  HW_UNAVAILABLE = 1,
  UNABLE_TO_PROCESS = 2,
  TIMEOUT = 3,
  NO_SPACE = 4,
  CANCELED = 5,
  UNABLE_TO_REMOVE = 6,
  VENDOR = 7,
  BAD_CALIBRATION = 8,
  POWER_PRESS = 9,
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
[[nodiscard]] static inline std::string toString(Error val) {
  switch(val) {
  case Error::UNKNOWN:
    return "UNKNOWN";
  case Error::HW_UNAVAILABLE:
    return "HW_UNAVAILABLE";
  case Error::UNABLE_TO_PROCESS:
    return "UNABLE_TO_PROCESS";
  case Error::TIMEOUT:
    return "TIMEOUT";
  case Error::NO_SPACE:
    return "NO_SPACE";
  case Error::CANCELED:
    return "CANCELED";
  case Error::UNABLE_TO_REMOVE:
    return "UNABLE_TO_REMOVE";
  case Error::VENDOR:
    return "VENDOR";
  case Error::BAD_CALIBRATION:
    return "BAD_CALIBRATION";
  case Error::POWER_PRESS:
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
constexpr inline std::array<aidl::android::hardware::biometrics::fingerprint::Error, 10> enum_values<aidl::android::hardware::biometrics::fingerprint::Error> = {
  aidl::android::hardware::biometrics::fingerprint::Error::UNKNOWN,
  aidl::android::hardware::biometrics::fingerprint::Error::HW_UNAVAILABLE,
  aidl::android::hardware::biometrics::fingerprint::Error::UNABLE_TO_PROCESS,
  aidl::android::hardware::biometrics::fingerprint::Error::TIMEOUT,
  aidl::android::hardware::biometrics::fingerprint::Error::NO_SPACE,
  aidl::android::hardware::biometrics::fingerprint::Error::CANCELED,
  aidl::android::hardware::biometrics::fingerprint::Error::UNABLE_TO_REMOVE,
  aidl::android::hardware::biometrics::fingerprint::Error::VENDOR,
  aidl::android::hardware::biometrics::fingerprint::Error::BAD_CALIBRATION,
  aidl::android::hardware::biometrics::fingerprint::Error::POWER_PRESS,
};
#pragma clang diagnostic pop
}  // namespace internal
}  // namespace ndk
