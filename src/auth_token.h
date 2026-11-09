// SPDX-License-Identifier: GPL-3.0-only
//
// Conversion between the AIDL HardwareAuthToken and the 69-byte packed
// hw_auth_token_t the vendor engine expects.
//
// A layout mistake here is invisible: the sensor still returns 0 and no error is
// reported, but the capture never starts. The offsets are therefore asserted at
// compile time.

#pragma once

#include <cstddef>
#include <cstdint>

#include <aidl/android/hardware/keymaster/HardwareAuthToken.h>

namespace km6fod {

constexpr size_t kAuthTokenSize = 69;

// Packed keymaster hw_auth_token_t, little-endian.
struct __attribute__((packed)) AuthTokenRaw {
    uint8_t version;          // 0
    uint64_t challenge;       // 1
    uint64_t userId;          // 9
    uint64_t authenticatorId; // 17
    uint32_t authenticatorType;  // 25
    uint64_t timestamp;       // 29
    uint8_t hmac[32];         // 37
};

static_assert(offsetof(AuthTokenRaw, version) == 0, "version offset");
static_assert(offsetof(AuthTokenRaw, challenge) == 1, "challenge offset");
static_assert(offsetof(AuthTokenRaw, userId) == 9, "userId offset");
static_assert(offsetof(AuthTokenRaw, authenticatorId) == 17, "authenticatorId offset");
static_assert(offsetof(AuthTokenRaw, authenticatorType) == 25, "authenticatorType offset");
static_assert(offsetof(AuthTokenRaw, timestamp) == 29, "timestamp offset");
static_assert(offsetof(AuthTokenRaw, hmac) == 37, "hmac offset");
static_assert(sizeof(AuthTokenRaw) == 69, "hw_auth_token_t must be 69 bytes");

// Serialises the AIDL token into the packed vendor representation.
void packAuthToken(const aidl::android::hardware::keymaster::HardwareAuthToken& token,
                   uint8_t out[kAuthTokenSize]);

// Parses the engine's 69-byte token into the AIDL representation.
aidl::android::hardware::keymaster::HardwareAuthToken unpackAuthToken(
    const uint8_t in[kAuthTokenSize]);

// True when the buffer carries a plausible token (non-degenerate challenge or
// MAC). Used to avoid forwarding an all-zero HAT as a success signal.
bool isAuthTokenPlausible(const uint8_t in[kAuthTokenSize]);

}  // namespace km6fod
