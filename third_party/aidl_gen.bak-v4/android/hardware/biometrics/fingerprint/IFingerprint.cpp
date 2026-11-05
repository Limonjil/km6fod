/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/limonjil/android/sdk/build-tools/35.0.0/aidl --lang=ndk --min_sdk_version=31 --structured --stability vintf -o gen -Iflat --header_out=gen/include flat/android/hardware/biometrics/fingerprint/IFingerprint.aidl
 */
#include "aidl/android/hardware/biometrics/fingerprint/IFingerprint.h"

#include <android/binder_parcel_utils.h>
#include <aidl/android/hardware/biometrics/common/BnCancellationSignal.h>
#include <aidl/android/hardware/biometrics/common/BpCancellationSignal.h>
#include <aidl/android/hardware/biometrics/common/ICancellationSignal.h>
#include <aidl/android/hardware/biometrics/fingerprint/BnFingerprint.h>
#include <aidl/android/hardware/biometrics/fingerprint/BnSession.h>
#include <aidl/android/hardware/biometrics/fingerprint/BnSessionCallback.h>
#include <aidl/android/hardware/biometrics/fingerprint/BpFingerprint.h>
#include <aidl/android/hardware/biometrics/fingerprint/BpSession.h>
#include <aidl/android/hardware/biometrics/fingerprint/BpSessionCallback.h>
#include <aidl/android/hardware/biometrics/fingerprint/ISession.h>
#include <aidl/android/hardware/biometrics/fingerprint/ISessionCallback.h>

namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {
static binder_status_t _aidl_android_hardware_biometrics_fingerprint_IFingerprint_onTransact(AIBinder* _aidl_binder, transaction_code_t _aidl_code, const AParcel* _aidl_in, AParcel* _aidl_out) {
  (void)_aidl_in;
  (void)_aidl_out;
  binder_status_t _aidl_ret_status = STATUS_UNKNOWN_TRANSACTION;
  std::shared_ptr<BnFingerprint> _aidl_impl = std::static_pointer_cast<BnFingerprint>(::ndk::ICInterface::asInterface(_aidl_binder));
  switch (_aidl_code) {
    case (FIRST_CALL_TRANSACTION + 0 /*getSensorProps*/): {
      std::vector<::aidl::android::hardware::biometrics::fingerprint::SensorProps> _aidl_return;

      ::ndk::ScopedAStatus _aidl_status = _aidl_impl->getSensorProps(&_aidl_return);
      _aidl_ret_status = AParcel_writeStatusHeader(_aidl_out, _aidl_status.get());
      if (_aidl_ret_status != STATUS_OK) break;

      if (!AStatus_isOk(_aidl_status.get())) break;

      _aidl_ret_status = ::ndk::AParcel_writeData(_aidl_out, _aidl_return);
      if (_aidl_ret_status != STATUS_OK) break;

      break;
    }
    case (FIRST_CALL_TRANSACTION + 1 /*createSession*/): {
      int32_t in_sensorId;
      int32_t in_userId;
      std::shared_ptr<::aidl::android::hardware::biometrics::fingerprint::ISessionCallback> in_cb;
      std::shared_ptr<::aidl::android::hardware::biometrics::fingerprint::ISession> _aidl_return;

      _aidl_ret_status = ::ndk::AParcel_readData(_aidl_in, &in_sensorId);
      if (_aidl_ret_status != STATUS_OK) break;

      _aidl_ret_status = ::ndk::AParcel_readData(_aidl_in, &in_userId);
      if (_aidl_ret_status != STATUS_OK) break;

      _aidl_ret_status = ::ndk::AParcel_readData(_aidl_in, &in_cb);
      if (_aidl_ret_status != STATUS_OK) break;

      ::ndk::ScopedAStatus _aidl_status = _aidl_impl->createSession(in_sensorId, in_userId, in_cb, &_aidl_return);
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

static AIBinder_Class* _g_aidl_android_hardware_biometrics_fingerprint_IFingerprint_clazz = ::ndk::ICInterface::defineClass(IFingerprint::descriptor, _aidl_android_hardware_biometrics_fingerprint_IFingerprint_onTransact);

BpFingerprint::BpFingerprint(const ::ndk::SpAIBinder& binder) : BpCInterface(binder) {}
BpFingerprint::~BpFingerprint() {}

::ndk::ScopedAStatus BpFingerprint::getSensorProps(std::vector<::aidl::android::hardware::biometrics::fingerprint::SensorProps>* _aidl_return) {
  binder_status_t _aidl_ret_status = STATUS_OK;
  ::ndk::ScopedAStatus _aidl_status;
  ::ndk::ScopedAParcel _aidl_in;
  ::ndk::ScopedAParcel _aidl_out;

  _aidl_ret_status = AIBinder_prepareTransaction(asBinder().get(), _aidl_in.getR());
  if (_aidl_ret_status != STATUS_OK) goto _aidl_error;

  _aidl_ret_status = AIBinder_transact(
    asBinder().get(),
    (FIRST_CALL_TRANSACTION + 0 /*getSensorProps*/),
    _aidl_in.getR(),
    _aidl_out.getR(),
    0
    #ifdef BINDER_STABILITY_SUPPORT
    | FLAG_PRIVATE_LOCAL
    #endif  // BINDER_STABILITY_SUPPORT
    );
  if (_aidl_ret_status == STATUS_UNKNOWN_TRANSACTION && IFingerprint::getDefaultImpl()) {
    _aidl_status = IFingerprint::getDefaultImpl()->getSensorProps(_aidl_return);
    goto _aidl_status_return;
  }
  if (_aidl_ret_status != STATUS_OK) goto _aidl_error;

  _aidl_ret_status = AParcel_readStatusHeader(_aidl_out.get(), _aidl_status.getR());
  if (_aidl_ret_status != STATUS_OK) goto _aidl_error;

  if (!AStatus_isOk(_aidl_status.get())) goto _aidl_status_return;
  _aidl_ret_status = ::ndk::AParcel_readData(_aidl_out.get(), _aidl_return);
  if (_aidl_ret_status != STATUS_OK) goto _aidl_error;

  _aidl_error:
  _aidl_status.set(AStatus_fromStatus(_aidl_ret_status));
  _aidl_status_return:
  return _aidl_status;
}
::ndk::ScopedAStatus BpFingerprint::createSession(int32_t in_sensorId, int32_t in_userId, const std::shared_ptr<::aidl::android::hardware::biometrics::fingerprint::ISessionCallback>& in_cb, std::shared_ptr<::aidl::android::hardware::biometrics::fingerprint::ISession>* _aidl_return) {
  binder_status_t _aidl_ret_status = STATUS_OK;
  ::ndk::ScopedAStatus _aidl_status;
  ::ndk::ScopedAParcel _aidl_in;
  ::ndk::ScopedAParcel _aidl_out;

  _aidl_ret_status = AIBinder_prepareTransaction(asBinder().get(), _aidl_in.getR());
  if (_aidl_ret_status != STATUS_OK) goto _aidl_error;

  _aidl_ret_status = ::ndk::AParcel_writeData(_aidl_in.get(), in_sensorId);
  if (_aidl_ret_status != STATUS_OK) goto _aidl_error;

  _aidl_ret_status = ::ndk::AParcel_writeData(_aidl_in.get(), in_userId);
  if (_aidl_ret_status != STATUS_OK) goto _aidl_error;

  _aidl_ret_status = ::ndk::AParcel_writeData(_aidl_in.get(), in_cb);
  if (_aidl_ret_status != STATUS_OK) goto _aidl_error;

  _aidl_ret_status = AIBinder_transact(
    asBinder().get(),
    (FIRST_CALL_TRANSACTION + 1 /*createSession*/),
    _aidl_in.getR(),
    _aidl_out.getR(),
    0
    #ifdef BINDER_STABILITY_SUPPORT
    | FLAG_PRIVATE_LOCAL
    #endif  // BINDER_STABILITY_SUPPORT
    );
  if (_aidl_ret_status == STATUS_UNKNOWN_TRANSACTION && IFingerprint::getDefaultImpl()) {
    _aidl_status = IFingerprint::getDefaultImpl()->createSession(in_sensorId, in_userId, in_cb, _aidl_return);
    goto _aidl_status_return;
  }
  if (_aidl_ret_status != STATUS_OK) goto _aidl_error;

  _aidl_ret_status = AParcel_readStatusHeader(_aidl_out.get(), _aidl_status.getR());
  if (_aidl_ret_status != STATUS_OK) goto _aidl_error;

  if (!AStatus_isOk(_aidl_status.get())) goto _aidl_status_return;
  _aidl_ret_status = ::ndk::AParcel_readData(_aidl_out.get(), _aidl_return);
  if (_aidl_ret_status != STATUS_OK) goto _aidl_error;

  _aidl_error:
  _aidl_status.set(AStatus_fromStatus(_aidl_ret_status));
  _aidl_status_return:
  return _aidl_status;
}
// Source for BnFingerprint
BnFingerprint::BnFingerprint() {}
BnFingerprint::~BnFingerprint() {}
::ndk::SpAIBinder BnFingerprint::createBinder() {
  AIBinder* binder = AIBinder_new(_g_aidl_android_hardware_biometrics_fingerprint_IFingerprint_clazz, static_cast<void*>(this));
  #ifdef BINDER_STABILITY_SUPPORT
  AIBinder_markVintfStability(binder);
  #endif  // BINDER_STABILITY_SUPPORT
  return ::ndk::SpAIBinder(binder);
}
// Source for IFingerprint
const char* IFingerprint::descriptor = "android.hardware.biometrics.fingerprint.IFingerprint";
IFingerprint::IFingerprint() {}
IFingerprint::~IFingerprint() {}


std::shared_ptr<IFingerprint> IFingerprint::fromBinder(const ::ndk::SpAIBinder& binder) {
  if (!AIBinder_associateClass(binder.get(), _g_aidl_android_hardware_biometrics_fingerprint_IFingerprint_clazz)) {
    #if __ANDROID_API__ >= 31
    const AIBinder_Class* originalClass = AIBinder_getClass(binder.get());
    if (originalClass == nullptr) return nullptr;
    if (0 == strcmp(AIBinder_Class_getDescriptor(originalClass), descriptor)) {
      return ::ndk::SharedRefBase::make<BpFingerprint>(binder);
    }
    #endif
    return nullptr;
  }
  std::shared_ptr<::ndk::ICInterface> interface = ::ndk::ICInterface::asInterface(binder.get());
  if (interface) {
    return std::static_pointer_cast<IFingerprint>(interface);
  }
  return ::ndk::SharedRefBase::make<BpFingerprint>(binder);
}

binder_status_t IFingerprint::writeToParcel(AParcel* parcel, const std::shared_ptr<IFingerprint>& instance) {
  return AParcel_writeStrongBinder(parcel, instance ? instance->asBinder().get() : nullptr);
}
binder_status_t IFingerprint::readFromParcel(const AParcel* parcel, std::shared_ptr<IFingerprint>* instance) {
  ::ndk::SpAIBinder binder;
  binder_status_t status = AParcel_readStrongBinder(parcel, binder.getR());
  if (status != STATUS_OK) return status;
  *instance = IFingerprint::fromBinder(binder);
  return STATUS_OK;
}
bool IFingerprint::setDefaultImpl(const std::shared_ptr<IFingerprint>& impl) {
  // Only one user of this interface can use this function
  // at a time. This is a heuristic to detect if two different
  // users in the same process use this function.
  assert(!IFingerprint::default_impl);
  if (impl) {
    IFingerprint::default_impl = impl;
    return true;
  }
  return false;
}
const std::shared_ptr<IFingerprint>& IFingerprint::getDefaultImpl() {
  return IFingerprint::default_impl;
}
std::shared_ptr<IFingerprint> IFingerprint::default_impl = nullptr;
::ndk::ScopedAStatus IFingerprintDefault::getSensorProps(std::vector<::aidl::android::hardware::biometrics::fingerprint::SensorProps>* /*_aidl_return*/) {
  ::ndk::ScopedAStatus _aidl_status;
  _aidl_status.set(AStatus_fromStatus(STATUS_UNKNOWN_TRANSACTION));
  return _aidl_status;
}
::ndk::ScopedAStatus IFingerprintDefault::createSession(int32_t /*in_sensorId*/, int32_t /*in_userId*/, const std::shared_ptr<::aidl::android::hardware::biometrics::fingerprint::ISessionCallback>& /*in_cb*/, std::shared_ptr<::aidl::android::hardware::biometrics::fingerprint::ISession>* /*_aidl_return*/) {
  ::ndk::ScopedAStatus _aidl_status;
  _aidl_status.set(AStatus_fromStatus(STATUS_UNKNOWN_TRANSACTION));
  return _aidl_status;
}
::ndk::SpAIBinder IFingerprintDefault::asBinder() {
  return ::ndk::SpAIBinder();
}
bool IFingerprintDefault::isRemote() {
  return false;
}
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
