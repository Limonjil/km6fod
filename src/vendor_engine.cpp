// SPDX-License-Identifier: GPL-2.0-only

#include "vendor_engine.h"

#include <dlfcn.h>
#include <unistd.h>

#include <array>
#include <cerrno>
#include <cstdio>
#include <cstring>

#include "logging.h"
#include "vendor_hal_types.h"

namespace km6fod {
namespace {

using vendorhw::hw_device_t;
using vendorhw::hw_module_t;

// Byte offsets of the vendor vtable entries inside fingerprint_device_t.
constexpr size_t kSlotClose = 0x70;
constexpr size_t kSlotSetNotify = 0x80;
constexpr size_t kSlotPreEnroll = 0x88;
constexpr size_t kSlotEnroll = 0x90;
constexpr size_t kSlotPostEnroll = 0x98;
constexpr size_t kSlotGetAuthId = 0xA0;
constexpr size_t kSlotCancel = 0xA8;
constexpr size_t kSlotEnumerate = 0xB0;
constexpr size_t kSlotRemove = 0xB8;
constexpr size_t kSlotSetActiveGroup = 0xC0;
constexpr size_t kSlotAuthenticate = 0xC8;

using SetNotifyFn = int (*)(void*, EngineNotifyFn);
using PreEnrollFn = uint64_t (*)(void*);
using EnrollFn = int (*)(void*, const uint8_t*, uint32_t, uint32_t);
using PostEnrollFn = int (*)(void*);
using GetAuthIdFn = uint64_t (*)(void*);
using CancelFn = int (*)(void*);
using EnumerateFn = int (*)(void*);
using RemoveFn = int (*)(void*, uint32_t, uint32_t);
using SetActiveGroupFn = int (*)(void*, uint32_t, const char*);
using AuthenticateFn = int (*)(void*, uint64_t, uint32_t);
using CloseFn = int (*)(void*);

struct TouchInfo {
    int32_t x;
    int32_t y;
    int32_t minor;
    int32_t reserved;
};
static_assert(sizeof(TouchInfo) == 16, "touch payload must be 16 bytes");

}  // namespace

std::string VendorEngine::storePathForUser(int32_t user_id) {
    // Matches the vendor layout: one directory per Android user.
    return std::string("/data/vendor_de/") + std::to_string(user_id) + "/fpdata";
}

VendorEngine& VendorEngine::instance() {
    // Intentionally leaked: the vendor libraries own threads and must never be
    // destroyed by static destructor ordering at process exit.
    static VendorEngine* engine = new VendorEngine();
    return *engine;
}

VendorEngine::Slot VendorEngine::slot(size_t offset) const {
    // Only the offsets this code actually reads are allowed. The old bound (<= 0x200)
    // assumed the vendor object is at least that big, which nothing guarantees
    // (review H-25).
    static constexpr size_t kKnownSlots[] = {
        kSlotClose,          kSlotSetNotify, kSlotPreEnroll, kSlotEnroll,
        kSlotPostEnroll,     kSlotGetAuthId, kSlotCancel,    kSlotEnumerate,
        kSlotRemove,         kSlotSetActiveGroup,            kSlotAuthenticate};
    bool known = false;
    for (size_t candidate : kKnownSlots) {
        if (candidate == offset) {
            known = true;
            break;
        }
    }
    if (device_ == nullptr || !known) {
        return nullptr;
    }
    return reinterpret_cast<void**>(device_)[offset / sizeof(void*)];
}

bool VendorEngine::loadEngineSymbolsLocked() {
    if (engine_handle_ == nullptr) {
        // libsl_fp_impl.so is normally already loaded as a dependency of the
        // HAL module; RTLD_NOLOAD promotes it to a handle without remapping.
        engine_handle_ = ::dlopen(kEngineLibrary, RTLD_NOW | RTLD_NOLOAD);
        if (engine_handle_ == nullptr) {
            engine_handle_ = ::dlopen(kEngineLibrary, RTLD_NOW | RTLD_NODELETE);
        }
    }
    if (engine_handle_ == nullptr) {
        LOGE("engine: cannot open %s: %s", kEngineLibrary, ::dlerror());
        return false;
    }

    fn_notify_ui_ready_ =
        reinterpret_cast<void (*)(int)>(::dlsym(engine_handle_, "silfp_cust_notify_ui_ready"));
    fn_screen_state_ = reinterpret_cast<void (*)(int)>(
        ::dlsym(engine_handle_, "silfp_worker_screen_state_callback"));
    fn_set_touch_info_ = reinterpret_cast<int (*)(const void*, int)>(
        ::dlsym(engine_handle_, "silfp_impl_set_touch_info"));
    fn_get_chipid_ =
        reinterpret_cast<int (*)(void*, void*)>(::dlsym(engine_handle_, "silfp_cmd_get_chipid"));
    fn_sync_optic_ = reinterpret_cast<void (*)(int)>(
        ::dlsym(engine_handle_, "silfp_impl_sync_finger_status_optic"));
    fn_wakelock_ = reinterpret_cast<int (*)(int)>(::dlsym(engine_handle_, "silfp_dev_wakelock"));
    if (fn_wakelock_ == nullptr) {
        fn_wakelock_ =
            reinterpret_cast<int (*)(int)>(::dlsym(engine_handle_, "silfp_impl_wakelock"));
    }

    symbols_loaded_ = fn_notify_ui_ready_ != nullptr && fn_set_touch_info_ != nullptr;
    LOGI("engine: symbols ui_ready=%p screen_state=%p touch_info=%p chipid=%p sync_optic=%p",
         reinterpret_cast<void*>(fn_notify_ui_ready_), reinterpret_cast<void*>(fn_screen_state_),
         reinterpret_cast<void*>(fn_set_touch_info_), reinterpret_cast<void*>(fn_get_chipid_),
         reinterpret_cast<void*>(fn_sync_optic_));
    return symbols_loaded_;
}

bool VendorEngine::open() {
    std::unique_lock<std::shared_mutex> lock(device_mutex_);
    if (device_ != nullptr) {
        return true;
    }

    if (hal_handle_ == nullptr) {
        hal_handle_ = ::dlopen(kHalLibrary, RTLD_NOW | RTLD_NODELETE);
        if (hal_handle_ == nullptr) {
            // Fall back to the explicit vendor path; the short name only resolves
            // for processes that already sit in the vendor namespace.
            const char* short_error = ::dlerror();
            const std::string saved_error = short_error != nullptr ? short_error : "n/a";
            hal_handle_ = ::dlopen(kHalLibraryAbsolute, RTLD_NOW | RTLD_NODELETE);
            if (hal_handle_ == nullptr) {
                LOGE("engine: dlopen(%s) failed: %s (short name: %s)", kHalLibraryAbsolute,
                     ::dlerror(), saved_error.c_str());
                return false;
            }
        }
        LOGI("engine: vendor HAL module loaded");
    }

    auto* module = reinterpret_cast<hw_module_t*>(::dlsym(hal_handle_, "HMI"));
    if (module == nullptr) {
        LOGE("engine: dlsym(HMI) failed: %s", ::dlerror());
        return false;
    }
    if (module->methods == nullptr || module->methods->open == nullptr) {
        LOGE("engine: HAL module has no open() method");
        return false;
    }

    hw_device_t* raw_device = nullptr;
    const int rc = module->methods->open(module, "default", &raw_device);
    if (rc != 0 || raw_device == nullptr) {
        LOGE("engine: open(default) failed rc=%d (%s)", rc, std::strerror(-rc));
        // Retry on a later operation; the sensor may simply be busy right now.
        return false;
    }
    device_ = raw_device;

    if (!loadEngineSymbolsLocked()) {
        LOGW("engine: helper symbols incomplete, capability is reduced");
    }

    // The engine's own token file must exist before set_active_group, otherwise
    // it silently keeps an empty group and enrollment never persists.
    if (::access(kDefaultStorePath, F_OK) != 0) {
        LOGW("engine: store path %s missing (errno=%d)", kDefaultStorePath, errno);
    }

    // Slot 0x80: install the notification callback. Installing it here (not in
    // a separate startup task) closes the race where the first operation opens
    // the sensor before set_notify has run.
    if (notify_fn_ != nullptr) {
        auto fn = slotAs<SetNotifyFn>(kSlotSetNotify);
        if (fn != nullptr) {
            const int notify_rc = fn(device_, notify_fn_);
            LOGI("engine: set_notify rc=%d", notify_rc);
        } else {
            LOGE("engine: set_notify slot is null");
        }
    }

    // Slot 0xc0: the mandatory default group. A session may immediately switch
    // to a per-user path, but the engine must have a valid group before any
    // pre_enroll/enroll.
    auto group_fn = slotAs<SetActiveGroupFn>(kSlotSetActiveGroup);
    if (group_fn != nullptr) {
        const int group_rc = group_fn(device_, 0, kDefaultStorePath);
        LOGI("engine: default set_active_group rc=%d", group_rc);
    } else {
        LOGE("engine: set_active_group slot is null");
    }

    lock.unlock();

    // Diagnostic only; never gate a capture on it.
    int status = 0;
    probeSpi(&status);
    return true;
}

void VendorEngine::close() {
    std::unique_lock<std::shared_mutex> lock(device_mutex_);
    if (device_ == nullptr) {
        return;
    }
    // Slot 0x70 is the vendor close()/free() entry; calling it also de-inits the
    // engine and releases /dev/silead_fp. Do not dlclose the libraries.
    auto close_fn = slotAs<CloseFn>(kSlotClose);
    if (close_fn != nullptr) {
        close_fn(device_);
    } else {
        LOGW("engine: no close slot, leaking device handle");
    }
    device_ = nullptr;
}

bool VendorEngine::isOpen() const {
    std::shared_lock<std::shared_mutex> lock(device_mutex_);
    return device_ != nullptr;
}

void VendorEngine::setNotify(EngineNotifyFn callback) {
    std::unique_lock<std::shared_mutex> lock(device_mutex_);
    notify_fn_ = callback;
    if (device_ == nullptr) {
        return;
    }
    auto fn = slotAs<SetNotifyFn>(kSlotSetNotify);
    if (fn == nullptr) {
        LOGE("engine: set_notify slot is null");
        return;
    }
    const int rc = fn(device_, callback);
    LOGI("engine: set_notify rc=%d", rc);
}

uint64_t VendorEngine::preEnroll() {
    std::shared_lock<std::shared_mutex> device(device_mutex_);
    if (device_ == nullptr) {
        return 0;
    }
    std::lock_guard<std::mutex> call(call_mutex_);
    auto fn = slotAs<PreEnrollFn>(kSlotPreEnroll);
    if (fn == nullptr) {
        return 0;
    }
    return fn(device_);
}

int VendorEngine::enroll(const uint8_t* hat_69, uint32_t gid, uint32_t timeout_seconds) {
    std::shared_lock<std::shared_mutex> device(device_mutex_);
    if (device_ == nullptr) {
        return -1;
    }
    std::lock_guard<std::mutex> call(call_mutex_);
    auto fn = slotAs<EnrollFn>(kSlotEnroll);
    if (fn == nullptr) {
        return -1;
    }
    return fn(device_, hat_69, gid, timeout_seconds);
}

int VendorEngine::postEnroll() {
    std::shared_lock<std::shared_mutex> device(device_mutex_);
    if (device_ == nullptr) {
        return -1;
    }
    std::lock_guard<std::mutex> call(call_mutex_);
    auto fn = slotAs<PostEnrollFn>(kSlotPostEnroll);
    if (fn == nullptr) {
        return -1;
    }
    return fn(device_);
}

uint64_t VendorEngine::getAuthenticatorId() {
    std::shared_lock<std::shared_mutex> device(device_mutex_);
    if (device_ == nullptr) {
        return 0;
    }
    std::lock_guard<std::mutex> call(call_mutex_);
    auto fn = slotAs<GetAuthIdFn>(kSlotGetAuthId);
    if (fn == nullptr) {
        return 0;
    }
    return fn(device_);
}

int VendorEngine::cancel() {
    std::shared_lock<std::shared_mutex> device(device_mutex_);
    if (device_ == nullptr) {
        return -1;
    }
    // Deliberately NOT call_mutex_: cancel must be able to interrupt a running
    // authenticate/enroll. The vendor HAL is a HIDL-style reentrant device and
    // AOSP calls cancel from a different thread for exactly this reason.
    auto fn = slotAs<CancelFn>(kSlotCancel);
    if (fn == nullptr) {
        return -1;
    }
    return fn(device_);
}

int VendorEngine::enumerate() {
    std::shared_lock<std::shared_mutex> device(device_mutex_);
    if (device_ == nullptr) {
        return -1;
    }
    std::lock_guard<std::mutex> call(call_mutex_);
    auto fn = slotAs<EnumerateFn>(kSlotEnumerate);
    if (fn == nullptr) {
        return -1;
    }
    return fn(device_);
}

int VendorEngine::removeEnrollment(uint32_t gid, uint32_t fid) {
    std::shared_lock<std::shared_mutex> device(device_mutex_);
    if (device_ == nullptr) {
        return -1;
    }
    std::lock_guard<std::mutex> call(call_mutex_);
    auto fn = slotAs<RemoveFn>(kSlotRemove);
    if (fn == nullptr) {
        return -1;
    }
    return fn(device_, gid, fid);
}

int VendorEngine::setActiveGroup(uint32_t gid, const char* path) {
    if (path == nullptr) {
        return -1;
    }
    // Deliberately does NOT open the sensor. createSession() must stay lazy and
    // must not touch the vendor device on a Binder thread. Session::armCapture()
    // (on the SerialWorker) opens the sensor and then applies the user group
    // before the first capture command.
    std::shared_lock<std::shared_mutex> device(device_mutex_);
    if (device_ == nullptr) {
        return -1;
    }
    std::lock_guard<std::mutex> call(call_mutex_);
    auto fn = slotAs<SetActiveGroupFn>(kSlotSetActiveGroup);
    if (fn == nullptr) {
        return -1;
    }
    return fn(device_, gid, path);
}

int VendorEngine::authenticate(uint64_t operation_id, uint32_t gid) {
    std::shared_lock<std::shared_mutex> device(device_mutex_);
    if (device_ == nullptr) {
        return -1;
    }
    std::lock_guard<std::mutex> call(call_mutex_);
    auto fn = slotAs<AuthenticateFn>(kSlotAuthenticate);
    if (fn == nullptr) {
        return -1;
    }
    return fn(device_, operation_id, gid);
}

void VendorEngine::notifyUiReady(int ready) {
    std::shared_lock<std::shared_mutex> device(device_mutex_);
    if (device_ == nullptr || fn_notify_ui_ready_ == nullptr) {
        return;
    }
    fn_notify_ui_ready_(ready);
    LOGI("engine: ui_ready(%d)", ready);
}

bool VendorEngine::wakeSystem() {
    if (fn_wakelock_ == nullptr) {
        LOGW("engine: wakelock symbol missing, cannot wake through the kernel");
        return false;
    }
    const int rc = fn_wakelock_(1);
    LOGI("engine: wake via kernel wakelock rc=%d", rc);
    return rc == 0;
}

void VendorEngine::notifyScreenState(int state) {
    std::shared_lock<std::shared_mutex> device(device_mutex_);
    if (device_ == nullptr || fn_screen_state_ == nullptr) {
        return;
    }
    fn_screen_state_(state);
    LOGD("engine: screen_state(%d)", state);
}

int VendorEngine::sendTouchInfo(int x, int y) {
    std::shared_lock<std::shared_mutex> device(device_mutex_);
    if (device_ == nullptr || fn_set_touch_info_ == nullptr) {
        return -1;
    }
    TouchInfo info{x, y, 0, 0};
    const int rc = fn_set_touch_info_(&info, static_cast<int>(sizeof(info)));
    if (rc != 0) {
        LOGD("engine: touch_info(%d,%d) rc=%d", x, y, rc);
    }
    return rc;
}

void VendorEngine::syncFingerStatusOptic(bool down) {
    std::shared_lock<std::shared_mutex> device(device_mutex_);
    if (device_ == nullptr || fn_sync_optic_ == nullptr) {
        return;
    }
    fn_sync_optic_(down ? 1 : 0);
    LOGD("engine: sync_finger_status_optic(%d)", down ? 1 : 0);
}

int VendorEngine::enrollSteps() {
    // Deliberately NOT calling silfp_impl_get_enroll_num(): that symbol routes to
    // a different TA command than the engine's own GET_ENROLL_NUM (0x24) and,
    // issued outside the state machine right before enroll(), it left the sensor
    // unresponsive - enroll() then never reached the vendor code at all.
    // The engine reports 20 steps for this part, which is what stock used too.
    return kDefaultEnrollSteps;
}

bool VendorEngine::probeSpi(int* status_out) {
    std::shared_lock<std::shared_mutex> device(device_mutex_);
    if (device_ == nullptr || fn_get_chipid_ == nullptr) {
        if (status_out != nullptr) {
            *status_out = -1;
        }
        return false;
    }
    std::array<uint8_t, 16> chip{};
    const int rc = fn_get_chipid_(chip.data(), nullptr);
    if (status_out != nullptr) {
        *status_out = rc;
    }
    LOGI("engine: chipid probe rc=%d data=%02x%02x%02x%02x", rc, chip[0], chip[1], chip[2],
         chip[3]);
    return rc == 0;
}

}  // namespace km6fod
