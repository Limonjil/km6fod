// SPDX-License-Identifier: GPL-3.0-only
//
// Wire format of the Silead GSL7002 vendor engine notifications.
//
// The engine (libsl_fp_impl.so) posts an 88-byte message to the callback that
// was installed through fingerprint_device_t::set_notify. The layout below was
// recovered from the engine's own emitters and is byte-exact:
//
//   silfp_notify_send_acquired_notice(int info)        type=1, a=info
//   silfp_notify_send_enroll_notice(int fid, int rem, int x)
//                                                      type=3, a=rem, b=fid, c=x
//   silfp_notify_send_removed_notice(int fid, int a2, int x)
//                                                      type=4, a=a2, b=fid, c=x
//   silfp_notify_send_authenticated_notice(int fid, int a2, const void* hat)
//                                                      type=5, a=a2, b=fid, hat[69]
//   silfp_notify_send_enumerate_notice(int fid, int a2, int x)  type=6
//   silfp_notify_send_error_notice(int code)           type=-1, a=code
//
// NOTE: type -1 is a genuine engine *error*, not a vendor "raw" event. Treating
// it as progress (as an earlier revision of this module did) hides real sensor
// failures and produces an enroll loop that never finishes.

#pragma once

#include <cstdint>
#include <cstring>

namespace km6fod {

constexpr int32_t kEngineMsgError = -1;
constexpr int32_t kEngineMsgAcquired = 1;
constexpr int32_t kEngineMsgEnroll = 3;
constexpr int32_t kEngineMsgRemoved = 4;
constexpr int32_t kEngineMsgAuthenticated = 5;
constexpr int32_t kEngineMsgEnumerate = 6;

constexpr size_t kEngineHatSize = 69;

struct EngineMessage {
    int32_t type;
    int32_t reserved;
    int32_t a;
    int32_t b;
    // Overloaded by message type: 4-byte "extra" for enroll/removed/enumerate,
    // and the 69-byte hw_auth_token_t for authenticated.
    uint8_t hat[kEngineHatSize];
    uint8_t tail[3];

    // The third integer argument for the non-auth messages lives at offset 16,
    // which aliases the head of the hat buffer.
    int32_t extra() const {
        int32_t value = 0;
        std::memcpy(&value, hat, sizeof(value));
        return value;
    }
};

static_assert(sizeof(EngineMessage) == 88, "vendor engine message must stay 88 bytes");
static_assert(offsetof(EngineMessage, a) == 8, "field a must sit at offset 8");
static_assert(offsetof(EngineMessage, b) == 12, "field b must sit at offset 12");
static_assert(offsetof(EngineMessage, hat) == 16, "hat must sit at offset 16");

using EngineNotifyFn = void (*)(const EngineMessage* msg);

}  // namespace km6fod
