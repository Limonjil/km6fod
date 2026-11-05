/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: C:\Users\Limonjil\Desktop\android\android-sdk\build-tools\35.0.0\aidl.exe --lang=ndk --structured --stability=vintf --min_sdk_version=31 -v 5 -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\keymaster\aidl -h C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\include -o C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\out C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl\android\hardware\biometrics\common\OperationContext.aidl
 */
#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <android/binder_interface_utils.h>
#include <android/binder_parcelable_utils.h>
#include <android/binder_to_string.h>
#include <aidl/android/hardware/biometrics/common/AuthenticateReason.h>
#include <aidl/android/hardware/biometrics/common/DisplayState.h>
#include <aidl/android/hardware/biometrics/common/FoldState.h>
#include <aidl/android/hardware/biometrics/common/OperationReason.h>
#include <aidl/android/hardware/biometrics/common/OperationState.h>
#include <aidl/android/hardware/biometrics/common/WakeReason.h>
#ifdef BINDER_STABILITY_SUPPORT
#include <android/binder_stability.h>
#endif  // BINDER_STABILITY_SUPPORT

namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace common {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
class OperationContext {
public:
  typedef std::false_type fixed_size;
  static const char* descriptor;

  int32_t id = 0;
  ::aidl::android::hardware::biometrics::common::OperationReason reason = ::aidl::android::hardware::biometrics::common::OperationReason::UNKNOWN;
  bool __attribute__((deprecated("use displayState instead."))) isAod = false;
  bool isCrypto = false;
  ::aidl::android::hardware::biometrics::common::WakeReason wakeReason = ::aidl::android::hardware::biometrics::common::WakeReason::UNKNOWN;
  ::aidl::android::hardware::biometrics::common::DisplayState displayState = ::aidl::android::hardware::biometrics::common::DisplayState::UNKNOWN;
  std::optional<::aidl::android::hardware::biometrics::common::AuthenticateReason> authenticateReason;
  ::aidl::android::hardware::biometrics::common::FoldState foldState = ::aidl::android::hardware::biometrics::common::FoldState::UNKNOWN;
  std::optional<::aidl::android::hardware::biometrics::common::OperationState> operationState;

  binder_status_t readFromParcel(const AParcel* parcel);
  binder_status_t writeToParcel(AParcel* parcel) const;

  inline bool operator==(const OperationContext& _rhs) const {
    return std::tie(id, reason, isAod, isCrypto, wakeReason, displayState, authenticateReason, foldState, operationState) == std::tie(_rhs.id, _rhs.reason, _rhs.isAod, _rhs.isCrypto, _rhs.wakeReason, _rhs.displayState, _rhs.authenticateReason, _rhs.foldState, _rhs.operationState);
  }
  inline bool operator<(const OperationContext& _rhs) const {
    return std::tie(id, reason, isAod, isCrypto, wakeReason, displayState, authenticateReason, foldState, operationState) < std::tie(_rhs.id, _rhs.reason, _rhs.isAod, _rhs.isCrypto, _rhs.wakeReason, _rhs.displayState, _rhs.authenticateReason, _rhs.foldState, _rhs.operationState);
  }
  inline bool operator!=(const OperationContext& _rhs) const {
    return !(*this == _rhs);
  }
  inline bool operator>(const OperationContext& _rhs) const {
    return _rhs < *this;
  }
  inline bool operator>=(const OperationContext& _rhs) const {
    return !(*this < _rhs);
  }
  inline bool operator<=(const OperationContext& _rhs) const {
    return !(_rhs < *this);
  }

  static const ::ndk::parcelable_stability_t _aidl_stability = ::ndk::STABILITY_VINTF;
  inline std::string toString() const {
    std::ostringstream _aidl_os;
    _aidl_os << "OperationContext{";
    _aidl_os << "id: " << ::android::internal::ToString(id);
    _aidl_os << ", reason: " << ::android::internal::ToString(reason);
    _aidl_os << ", isAod: " << ::android::internal::ToString(isAod);
    _aidl_os << ", isCrypto: " << ::android::internal::ToString(isCrypto);
    _aidl_os << ", wakeReason: " << ::android::internal::ToString(wakeReason);
    _aidl_os << ", displayState: " << ::android::internal::ToString(displayState);
    _aidl_os << ", authenticateReason: " << ::android::internal::ToString(authenticateReason);
    _aidl_os << ", foldState: " << ::android::internal::ToString(foldState);
    _aidl_os << ", operationState: " << ::android::internal::ToString(operationState);
    _aidl_os << "}";
    return _aidl_os.str();
  }
};
#pragma clang diagnostic pop
}  // namespace common
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
