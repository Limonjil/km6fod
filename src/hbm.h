// SPDX-License-Identifier: GPL-2.0-only
//
// Panel high-brightness (HBM) control for the under-display sensor.
//
// The optical sensor underneath the panel can only capture a frame while the
// panel emits maximum light, so every capture is bracketed by an HBM window.
// Four hard requirements drive the design:
//
//   * Binder threads must never perform the sysfs round-trip plus the mandatory
//     settle delay. All node I/O happens on a dedicated worker; callers only
//     wait for a bounded completion.
//
//   * A write is not proof of effect. On this panel the store handler silently
//     skips the command when the display is off or idle ("display state is
//     slept or idle, skip set hbm node") and the node keeps reading 0xFA. The
//     driver is therefore verified by reading back 0xOK before the FULL_HBM_SET
//     gate is injected. A false success here means a capture in the dark.
//
//   * The window must never be left open. Normal completion closes it; a
//     separate guard thread force-offs the panel after inactivity or after an
//     absolute cap, even if the worker is stuck inside a sysfs write.
//
//   * The worker sleeps indefinitely when no window is open. A 100 ms poll
//     loop would keep the SoC out of deep sleep.

#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>

#include "uevent.h"

namespace km6fod {


// Called when the HAL itself woke the panel. The very first HBM write after a
// wake lands while the panel is still finishing its wake sequence, which shows
// up as a horizontal band across the display, so the controller waits one panel
// frame before driving the node the first time.

// Panel backlight level: > 0 means the display is actually lit, 0 means asleep or
// dozing, negative means the node could not be read.
int panelBacklightLevel();
class HbmController {
  public:
    // Delay between driving the sysfs node and telling the engine to capture.
    // Empirically the panel needs ~90 ms to reach full brightness.
    static constexpr int kSettleMs = 90;

    // A window with no engine events and no touches is closed after this.
    // The operation itself stays alive; the next touch re-arms the window.
    // Inactivity window. Long enough that the illumination does not drop out
    // between the touches of an enrollment (the user lifts the finger between
    // captures), short enough not to cook the panel if the framework vanishes.
    static constexpr int kInactivityMs = 15000;

    // Absolute upper bound for a single window, even with constant activity.
    static constexpr int kAbsoluteMaxMs = 30000;

    // If the panel has not acknowledged the ON transition by then, the guard
    // force-offs it. This also covers a worker stuck in a blocking sysfs write.
    static constexpr int kApplyHardLimitMs = 2000;

    // How long a caller may block waiting for the window to be applied.
    static constexpr int kRequestTimeoutMs = 800;

    // Grace period for the normal OFF path before the guard intervenes.
    static constexpr int kOffGraceMs = 400;

    static HbmController& instance();

    void start();
    void stop();

    // Applies the window (verified node write, settle, uevent) and returns once
    // the panel is ready for capture. False when the panel could not be driven
    // or the display is off/idle.
    bool requestOn();

    // Closes the window asynchronously. Safe from any thread.
    void requestOff();

    // Extends the inactivity deadline (engine message or touch sample).
    void keepAlive();

    // Synchronous, best-effort emergency close for the shutdown path. Never
    // waits for the worker; uses a fresh fd so a stuck worker cannot block it.
    void forceOffNow();

    bool isOn() const { return on_.load(std::memory_order_acquire); }

  private:
    using Clock = std::chrono::steady_clock;

    HbmController() = default;
    ~HbmController() = default;
    HbmController(const HbmController&) = delete;
    HbmController& operator=(const HbmController&) = delete;

    void workerLoop();
    void guardLoop();
    bool applyOn(uint64_t epoch);
    void applyOffNormal();
    bool writeOffIndependent(const char* reason);

    // True when the panel node still reports HBM. Used to catch a window the
    // driver dropped on its own (suspend) while our flag still says on.
    bool panelStillInHbm();

    // Both are called with io_mutex_ held.
    bool ensureNodeLocked();
    bool writeAllLocked(int fd, const char* value);
    bool readBackLocked(int fd, std::string* value);
    void closeNodeLocked();

    void armDeadlineLocked(Clock::time_point deadline);

    std::thread worker_;
    std::thread guard_;

    mutable std::mutex state_mutex_;
    std::condition_variable cv_;
    std::condition_variable guard_cv_;

    mutable std::mutex io_mutex_;
    int fd_ = -1;
    bool node_readable_ = false;
    std::string node_path_;

    std::atomic<bool> on_{false};
    std::atomic<bool> running_{false};
    std::atomic<bool> shutting_down_{false};
    std::atomic<bool> writing_{false};
    std::atomic<uint64_t> epoch_{0};

    bool want_on_ = false;
    uint64_t requested_seq_ = 0;
    uint64_t applied_seq_ = 0;
    Clock::time_point opened_at_{};
    Clock::time_point last_activity_{};
    Clock::time_point deadline_ = Clock::time_point::max();

    UeventInjector uevent_;
};

}  // namespace km6fod
