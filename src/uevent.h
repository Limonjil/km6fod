// SPDX-License-Identifier: GPL-3.0-only
//
// Kernel-object uevent injector.
//
// Why this exists
// ---------------
// The vendor engine refuses to start a capture until it observes the uevent
// "TRAN_FULL_HBM_EVENT=FULL_HBM_SET" on NETLINK_KOBJECT_UEVENT. On stock
// firmware the panel driver raises it through tran_fp.ko
// (tran_fasync_for_lcm_full_HBM). Under a GSI the panel driver never does,
// because the notification is only wired up by Transsion's display stack.
//
// The engine replaces embedded NULs with spaces and performs a substring
// search, so a datagram injected from userspace into multicast group 1 is
// indistinguishable from a genuine kernel uevent. Verified on device:
//   [AUTH_TIME] TRAN_FULL_HBM_EVENT=true, screen status: ..., is blind: 0
//
// The injector is called from the HBM worker, the HBM shutdown path and the
// signal thread; fd creation/close is therefore guarded by a mutex.

#pragma once

#include <mutex>

namespace km6fod {

class UeventInjector {
  public:
    UeventInjector() = default;
    ~UeventInjector();

    UeventInjector(const UeventInjector&) = delete;
    UeventInjector& operator=(const UeventInjector&) = delete;

    // Signals that the panel is now in full-HBM mode and the sensor may capture.
    bool sendFullHbmSet();

    // Signals that full-HBM mode ended. Best effort: the engine does not block
    // on this event, but sending it keeps the engine state machine balanced.
    bool sendFullHbmClear();

  private:
    bool send(const char* event_line);
    void closeLocked();

    std::mutex mutex_;
    int fd_ = -1;
};

}  // namespace km6fod
