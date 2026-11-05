/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: C:\Users\Limonjil\Desktop\android\android-sdk\build-tools\35.0.0\aidl.exe --lang=ndk --structured --stability=vintf --min_sdk_version=31 -v 5 -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\fingerprint\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl -I C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\keymaster\aidl -h C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\include -o C:\Users\Limonjil\AppData\Local\Temp\aidl17\gen3\out C:\Users\Limonjil\AppData\Local\Temp\aidl17\src\biometrics\common\aidl\android\hardware\biometrics\common\ICancellationSignal.aidl
 */
#include "aidl/android/hardware/biometrics/common/ICancellationSignal.h"

#include <android/binder_parcel_utils.h>
#include <aidl/android/hardware/biometrics/common/BnCancellationSignal.h>
#include <aidl/android/hardware/biometrics/common/BpCancellationSignal.h>

namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace common {
static binder_status_t _aidl_android_hardware_biometrics_common_ICancellationSignal_onTransact(AIBinder* _aidl_binder, transaction_code_t _aidl_code, const AParcel* _aidl_in, AParcel* _aidl_out) {
  (void)_aidl_in;
  (void)_aidl_out;
  binder_status_t _aidl_ret_status = STATUS_UNKNOWN_TRANSACTION;
  std::shared_ptr<BnCancellationSignal> _aidl_impl = std::static_pointer_cast<BnCancellationSignal>(::ndk::ICInterface::asInterface(_aidl_binder));
  switch (_aidl_code) {
    case (FIRST_CALL_TRANSACTION + 0 /*cancel*/): {

      ::ndk::ScopedAStatus _aidl_status = _aidl_impl->cancel();
      _aidl_ret_status = STATUS_OK;
      break;
    }
    case (FIRST_CALL_TRANSACTION + 16777214 /*getInterfaceVersion*/): {
      int32_t _aidl_return;

      ::ndk::ScopedAStatus _aidl_status = _aidl_impl->getInterfaceVersion(&_aidl_return);
      _aidl_ret_status = AParcel_writeStatusHeader(_aidl_out, _aidl_status.get());
      if (_aidl_ret_status != STATUS_OK) break;

      if (!AStatus_isOk(_aidl_status.get())) break;

      _aidl_ret_status = ::ndk::AParcel_writeData(_aidl_out, _aidl_return);
      if (_aidl_ret_status != STATUS_OK) break;

      break;
    }
  }
  return _aidl_ret_status;
}

static AIBinder_Class* _g_aidl_android_hardware_biometrics_common_ICancellationSignal_clazz = ::ndk::ICInterface::defineClass(ICancellationSignal::descriptor, _aidl_android_hardware_biometrics_common_ICancellationSignal_onTransact);

BpCancellationSignal::BpCancellationSignal(const ::ndk::SpAIBinder& binder) : BpCInterface(binder) {}
BpCancellationSignal::~BpCancellationSignal() {}

::ndk::ScopedAStatus BpCancellationSignal::cancel() {
  binder_status_t _aidl_ret_status = STATUS_OK;
  ::ndk::ScopedAStatus _aidl_status;
  ::ndk::ScopedAParcel _aidl_in;
  ::ndk::ScopedAParcel _aidl_out;

  _aidl_ret_status = AIBinder_prepareTransaction(asBinder().get(), _aidl_in.getR());
  if (_aidl_ret_status != STATUS_OK) goto _aidl_error;

  _aidl_ret_status = AIBinder_transact(
    asBinder().get(),
    (FIRST_CALL_TRANSACTION + 0 /*cancel*/),
    _aidl_in.getR(),
    _aidl_out.getR(),
    FLAG_ONEWAY
    #ifdef BINDER_STABILITY_SUPPORT
    | FLAG_PRIVATE_LOCAL
    #endif  // BINDER_STABILITY_SUPPORT
    );
  if (_aidl_ret_status == STATUS_UNKNOWN_TRANSACTION && ICancellationSignal::getDefaultImpl()) {
    _aidl_status = ICancellationSignal::getDefaultImpl()->cancel();
    goto _aidl_status_return;
  }
  if (_aidl_ret_status != STATUS_OK) goto _aidl_error;

  _aidl_error:
  _aidl_status.set(AStatus_fromStatus(_aidl_ret_status));
  _aidl_status_return:
  return _aidl_status;
}
::ndk::ScopedAStatus BpCancellationSignal::getInterfaceVersion(int32_t* _aidl_return) {
  binder_status_t _aidl_ret_status = STATUS_OK;
  ::ndk::ScopedAStatus _aidl_status;
  if (_aidl_cached_version != -1) {
    *_aidl_return = _aidl_cached_version;
    _aidl_status.set(AStatus_fromStatus(_aidl_ret_status));
    return _aidl_status;
  }
  ::ndk::ScopedAParcel _aidl_in;
  ::ndk::ScopedAParcel _aidl_out;

  _aidl_ret_status = AIBinder_prepareTransaction(asBinder().get(), _aidl_in.getR());
  if (_aidl_ret_status != STATUS_OK) goto _aidl_error;

  _aidl_ret_status = AIBinder_transact(
    asBinder().get(),
    (FIRST_CALL_TRANSACTION + 16777214 /*getInterfaceVersion*/),
    _aidl_in.getR(),
    _aidl_out.getR(),
    0
    #ifdef BINDER_STABILITY_SUPPORT
    | FLAG_PRIVATE_LOCAL
    #endif  // BINDER_STABILITY_SUPPORT
    );
  if (_aidl_ret_status == STATUS_UNKNOWN_TRANSACTION && ICancellationSignal::getDefaultImpl()) {
    _aidl_status = ICancellationSignal::getDefaultImpl()->getInterfaceVersion(_aidl_return);
    goto _aidl_status_return;
  }
  if (_aidl_ret_status != STATUS_OK) goto _aidl_error;

  _aidl_ret_status = AParcel_readStatusHeader(_aidl_out.get(), _aidl_status.getR());
  if (_aidl_ret_status != STATUS_OK) goto _aidl_error;

  if (!AStatus_isOk(_aidl_status.get())) goto _aidl_status_return;
  _aidl_ret_status = ::ndk::AParcel_readData(_aidl_out.get(), _aidl_return);
  if (_aidl_ret_status != STATUS_OK) goto _aidl_error;

  _aidl_cached_version = *_aidl_return;
  _aidl_error:
  _aidl_status.set(AStatus_fromStatus(_aidl_ret_status));
  _aidl_status_return:
  return _aidl_status;
}
// Source for BnCancellationSignal
BnCancellationSignal::BnCancellationSignal() {}
BnCancellationSignal::~BnCancellationSignal() {}
::ndk::SpAIBinder BnCancellationSignal::createBinder() {
  AIBinder* binder = AIBinder_new(_g_aidl_android_hardware_biometrics_common_ICancellationSignal_clazz, static_cast<void*>(this));
  #ifdef BINDER_STABILITY_SUPPORT
  AIBinder_markVintfStability(binder);
  #endif  // BINDER_STABILITY_SUPPORT
  return ::ndk::SpAIBinder(binder);
}
::ndk::ScopedAStatus BnCancellationSignal::getInterfaceVersion(int32_t* _aidl_return) {
  *_aidl_return = ICancellationSignal::version;
  return ::ndk::ScopedAStatus(AStatus_newOk());
}
// Source for ICancellationSignal
const char* ICancellationSignal::descriptor = "android.hardware.biometrics.common.ICancellationSignal";
ICancellationSignal::ICancellationSignal() {}
ICancellationSignal::~ICancellationSignal() {}


std::shared_ptr<ICancellationSignal> ICancellationSignal::fromBinder(const ::ndk::SpAIBinder& binder) {
  if (!AIBinder_associateClass(binder.get(), _g_aidl_android_hardware_biometrics_common_ICancellationSignal_clazz)) {
    #if __ANDROID_API__ >= 31
    const AIBinder_Class* originalClass = AIBinder_getClass(binder.get());
    if (originalClass == nullptr) return nullptr;
    if (0 == strcmp(AIBinder_Class_getDescriptor(originalClass), descriptor)) {
      return ::ndk::SharedRefBase::make<BpCancellationSignal>(binder);
    }
    #endif
    return nullptr;
  }
  std::shared_ptr<::ndk::ICInterface> interface = ::ndk::ICInterface::asInterface(binder.get());
  if (interface) {
    return std::static_pointer_cast<ICancellationSignal>(interface);
  }
  return ::ndk::SharedRefBase::make<BpCancellationSignal>(binder);
}

binder_status_t ICancellationSignal::writeToParcel(AParcel* parcel, const std::shared_ptr<ICancellationSignal>& instance) {
  return AParcel_writeStrongBinder(parcel, instance ? instance->asBinder().get() : nullptr);
}
binder_status_t ICancellationSignal::readFromParcel(const AParcel* parcel, std::shared_ptr<ICancellationSignal>* instance) {
  ::ndk::SpAIBinder binder;
  binder_status_t status = AParcel_readStrongBinder(parcel, binder.getR());
  if (status != STATUS_OK) return status;
  *instance = ICancellationSignal::fromBinder(binder);
  return STATUS_OK;
}
bool ICancellationSignal::setDefaultImpl(const std::shared_ptr<ICancellationSignal>& impl) {
  // Only one user of this interface can use this function
  // at a time. This is a heuristic to detect if two different
  // users in the same process use this function.
  assert(!ICancellationSignal::default_impl);
  if (impl) {
    ICancellationSignal::default_impl = impl;
    return true;
  }
  return false;
}
const std::shared_ptr<ICancellationSignal>& ICancellationSignal::getDefaultImpl() {
  return ICancellationSignal::default_impl;
}
std::shared_ptr<ICancellationSignal> ICancellationSignal::default_impl = nullptr;
::ndk::ScopedAStatus ICancellationSignalDefault::cancel() {
  ::ndk::ScopedAStatus _aidl_status;
  _aidl_status.set(AStatus_fromStatus(STATUS_UNKNOWN_TRANSACTION));
  return _aidl_status;
}
::ndk::ScopedAStatus ICancellationSignalDefault::getInterfaceVersion(int32_t* _aidl_return) {
  *_aidl_return = 0;
  return ::ndk::ScopedAStatus(AStatus_newOk());
}
::ndk::SpAIBinder ICancellationSignalDefault::asBinder() {
  return ::ndk::SpAIBinder();
}
bool ICancellationSignalDefault::isRemote() {
  return false;
}
}  // namespace common
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
