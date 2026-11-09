// SPDX-License-Identifier: GPL-3.0-only
#pragma once

// One runtime directory for everything the module writes: the HAL, the boot scripts
// and the uninstaller must agree on it (review H-32).
namespace km6fod {

constexpr const char* kRunDir = "/data/adb/km6fod/run";
constexpr const char* kLockPath = "/data/adb/km6fod/run/fp_hal.lock";

}  // namespace km6fod
