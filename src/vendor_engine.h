// SPDX-License-Identifier: GPL-3.0-only
//
// Thin, strongly-typed proxy over the Transsion/Silead vendor fingerprint
// engine.
//
// The vendor stack exposes a HIDL-2.1-style `fingerprint_device_t` whose vtable
// is an array of function pointers inside the heap object returned by
// fingerprint.silead.default.so:HMI. The layout was recovered from the module's
// own fingerprint_command() initialiser and is verified slot by slot:
//
//   0x70 close/free          0xa8 cancel
//   0x80 set_notify           0xb0 enumerate
//   0x88 pre_enroll           0xb8 remove
//   0x90 enroll               0xc0 set_active_group
//   0x98 post_enroll          0xc8 authenticate
//   0xa0 get_auth_id
//
// Locking contract:
//   * device_mutex_ is a shared_mutex. open()/close() take it exclusively and
//     therefore wait for every in-flight vendor call to finish.
//   * Ordinary vtable calls take it shared and additionally serialize on
//     call_mutex_, so the sensor never receives two commands at once.
//   * notify*(), sendTouchInfo() and probeSpi() take it shared but NOT
//     call_mutex_: touch coordinates must be deliverable while a capture is
//     running.
//   * cancel() takes it shared but deliberately bypasses call_mutex_. AOSP
//     requires cancel to preempt an in-flight authenticate/enroll; serializing
//     it behind the very call it must abort defeats the purpose.
//
// The vendor libraries are never dlclose()d. They own threads and static state;
// unloading them while those threads run is a use-after-free inside the vendor
// code. One mapping per process is the correct trade-off.

#pragma once

#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <shared_mutex>
#include <string>

#include "engine_message.h"

namespace km6fod {

class VendorEngine {
  public:
    static VendorEngine& instance();

    // Opens the sensor once. Safe to call from any thread; returns false when
    // the vendor library or the sensor node is unavailable. Installs the
    // notification callback and applies the default template group on success.
    // A caller that arrives while an attempt is in flight waits for its outcome
    // instead of opening the module a second time (review H-23).
    bool open();

    // Releases the sensor. Nested calls are tolerated. Does not dlclose.
    void close();

    bool isOpen() const;

    // Runs get_chipid() over SPI. Returns true when the sensor reports success
    // (the vendor helper returns 0, not a boolean).
    bool probeSpi(int* status_out);

    // Emits the "UI is ready" flag the engine's capture gate expects. Must be
    // called after the panel has entered HBM.
    void notifyUiReady(int ready);

    // Screen state: 2 == STATE_ON. The engine consults this before capturing.
    void notifyScreenState(int state);

    // Asks the vendor engine to wake the system through the kernel module
    // (SIFP_IOC_WAKELOCK on /dev/silead_fp -> tran_fp -> do_wakeup_set). This is
    // the stock path: the panel is woken by the kernel, not by an injected key.
    bool wakeSystem();

    // Pushes touch coordinates into the sensor (silfp_impl_set_touch_info).
    // Returns the vendor status: 0 means accepted.
    int sendTouchInfo(int x, int y);

    // Tells the engine that the finger is down/up. Without it the enroll loop
    // blocks in silfp_dev_wait_finger_status(), which waits for a driver message
    // the optical sensor never raises, and dies with err(1030).
    // Maps to silfp_dev_sync_finger_down()/up().
    void syncFingerStatusOptic(bool down);

    // Number of capture steps one enrollment needs (vendor GET_ENROLL_NUM).
    // The vendor enroll() takes this count as its "remaining" seed; passing the
    // user id instead makes the very first progress event report remaining=0 and
    // the framework considers the fingerprint enrolled after one touch.
    int enrollSteps();

    static constexpr int kDefaultEnrollSteps = 20;

    // ---- fingerprint_device_t slots -------------------------------------
    void setNotify(EngineNotifyFn callback);
    uint64_t preEnroll();
    int enroll(const uint8_t* hat_69, uint32_t gid, uint32_t timeout_seconds);
    int postEnroll();
    uint64_t getAuthenticatorId();
    int cancel();
    int enumerate();
    int removeEnrollment(uint32_t gid, uint32_t fid);
    int setActiveGroup(uint32_t gid, const char* path);
    int authenticate(uint64_t operation_id, uint32_t gid);

    static constexpr const char* kEngineLibrary = "libsl_fp_impl.so";
    static constexpr const char* kHalLibrary = "fingerprint.silead.default.so";
    // The HAL module lives in the vendor hw directory, which is not part of the
    // default linker search path for a process started from /data. The absolute
    // path resolves through the default namespace on this GSI (verified on
    // device).
    static constexpr const char* kHalLibraryAbsolute =
        "/vendor/lib64/hw/fingerprint.silead.default.so";
    static constexpr const char* kDefaultStorePath = "/data/vendor_de/0/fpdata";

    // Screen state value the engine treats as "awake".
    static constexpr int kScreenStateOn = 2;

    // Store path for a given Android user.
    static std::string storePathForUser(int32_t user_id);

  private:
    VendorEngine() = default;
    ~VendorEngine() = default;
    VendorEngine(const VendorEngine&) = delete;
    VendorEngine& operator=(const VendorEngine&) = delete;

    using Slot = void*;
    Slot slot(size_t offset) const;

    template <typename Fn>
    Fn slotAs(size_t offset) const {
        void* raw = slot(offset);
        Fn fn = nullptr;
        static_assert(sizeof(fn) == sizeof(raw), "function pointer size");
        std::memcpy(&fn, &raw, sizeof(fn));
        return fn;
    }

    bool loadEngineSymbolsLocked();

    // The slow half of open(): dlopen, the vendor open() and the two setup
    // slots. Runs without open_mutex_ so that isOpen() and the notify paths are
    // not parked behind the vendor call.
    bool openDevice();

    enum class OpenState { kClosed, kOpening, kOpen };
    std::mutex open_mutex_;
    std::condition_variable open_cv_;
    OpenState open_state_ = OpenState::kClosed;
    uint64_t open_epoch_ = 0;

    mutable std::shared_mutex device_mutex_;
    std::mutex call_mutex_;

    void* device_ = nullptr;
    void* hal_handle_ = nullptr;
    void* engine_handle_ = nullptr;
    bool symbols_loaded_ = false;

    EngineNotifyFn notify_fn_ = nullptr;

    // Engine helpers resolved from libsl_fp_impl.so. Signatures verified by
    // disassembly: set_touch_info/get_chipid return int, 0 == success.
    void (*fn_notify_ui_ready_)(int) = nullptr;
    void (*fn_screen_state_)(int) = nullptr;
    int (*fn_set_touch_info_)(const void*, int) = nullptr;
    int (*fn_get_chipid_)(void*, void*) = nullptr;
    void (*fn_sync_optic_)(int) = nullptr;
    int (*fn_wakelock_)(int) = nullptr;
};

}  // namespace km6fod
