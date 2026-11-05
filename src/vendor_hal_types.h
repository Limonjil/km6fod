// SPDX-License-Identifier: GPL-2.0-only
//
// Minimal, ABI-checked re-declaration of the Android HAL module ABI.
//
// The NDK does not ship <hardware/hardware.h> (it belongs to the platform
// libhardware), and the vendor engine only ever needs three things from it: the
// module handle exported as "HMI", its methods table, and the open() entry point.
// Declaring the structs locally keeps the dependency list empty while the
// static_asserts pin the layout to what the vendor library actually exports.
//
//   HMI is an hw_module_t: 8 bytes of header + 5 pointers + 25 reserved quads
//                         = 0xF8 on arm64.
//   hw_device_t is 0x70, which is also where fingerprint_device_t's first
//   function pointer (close) lives -- confirmed against the vendor vtable.

#pragma once

#include <cstdint>

namespace km6fod::vendorhw {

struct hw_module_t;
struct hw_device_t;

struct hw_module_methods_t {
    int (*open)(const hw_module_t* module, const char* id, hw_device_t** device);
};

struct hw_device_t {
    uint32_t tag;
    uint32_t version;
    hw_module_t* module;
    uint64_t reserved[12];
};

struct hw_module_t {
    uint32_t tag;
    uint16_t module_api_version;
    uint16_t hal_api_version;
    const char* id;
    const char* name;
    const char* author;
    hw_module_methods_t* methods;
    void* dso;
    uint64_t reserved[25];
};

static_assert(sizeof(hw_device_t) == 0x70, "hw_device_t must be 0x70 bytes");
static_assert(sizeof(hw_module_t) == 0xF8, "hw_module_t must be 0xF8 bytes");
static_assert(offsetof(hw_module_t, methods) == 0x20, "methods must sit at 0x20");

}  // namespace km6fod::vendorhw
