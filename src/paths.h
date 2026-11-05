// SPDX-License-Identifier: GPL-2.0-only
#pragma once

// One runtime directory for everything the HAL and the hold-mode helper share.
// They used to disagree - the HAL wrote hold_off under
// /data/adb/modules/km6fod/run while its own state lived in /data/adb/km6fod/run -
// so the helper never saw the signal (review H-32).
namespace km6fod {

constexpr const char* kRunDir = "/data/adb/km6fod/run";
constexpr const char* kLockPath = "/data/adb/km6fod/run/fp_hal.lock";
constexpr const char* kHoldOffPath = "/data/adb/km6fod/run/hold_off";
constexpr const char* kTemplatesMarkerPath = "/data/adb/km6fod/run/templates_seen";

}  // namespace km6fod
