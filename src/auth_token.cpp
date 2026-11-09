// SPDX-License-Identifier: GPL-3.0-only

#include "auth_token.h"

#include <algorithm>
#include <cstring>

#include "logging.h"

namespace km6fod {

void packAuthToken(const aidl::android::hardware::keymaster::HardwareAuthToken& token,
                   uint8_t out[kAuthTokenSize]) {
    AuthTokenRaw raw{};
    raw.version = 0;
    raw.challenge = static_cast<uint64_t>(token.challenge);
    raw.userId = static_cast<uint64_t>(token.userId);
    raw.authenticatorId = static_cast<uint64_t>(token.authenticatorId);
    raw.authenticatorType = static_cast<uint32_t>(token.authenticatorType);
    raw.timestamp = static_cast<uint64_t>(token.timestamp.milliSeconds);

    const size_t mac_length = std::min(token.mac.size(), sizeof(raw.hmac));
    std::memcpy(raw.hmac, token.mac.data(), mac_length);
    if (mac_length < sizeof(raw.hmac)) {
        std::memset(raw.hmac + mac_length, 0, sizeof(raw.hmac) - mac_length);
    }

    std::memcpy(out, &raw, kAuthTokenSize);

    // Full structural dump: if the TA rejects the challenge, the reason is in
    // one of these fields (wrong authenticator type, zeroed MAC, mismatched
    // user id), and neither the framework nor the vendor library logs them.
    bool mac_zero = true;
    for (uint8_t byte : raw.hmac) {
        if (byte != 0) {
            mac_zero = false;
            break;
        }
    }
    // Never log the challenge, the authenticator id or the MAC: the first two
    // identify the transaction and the third is the proof of authentication.
    LOGI("auth: hat v=%u uid=%llu type=%u ts=%llu mac_zero=%d",
         raw.version, static_cast<unsigned long long>(raw.userId),
         raw.authenticatorType, static_cast<unsigned long long>(raw.timestamp),
         mac_zero ? 1 : 0);
}

aidl::android::hardware::keymaster::HardwareAuthToken unpackAuthToken(
    const uint8_t in[kAuthTokenSize]) {
    AuthTokenRaw raw{};
    std::memcpy(&raw, in, kAuthTokenSize);

    aidl::android::hardware::keymaster::HardwareAuthToken token{};
    token.challenge = static_cast<int64_t>(raw.challenge);
    token.userId = static_cast<int64_t>(raw.userId);
    token.authenticatorId = static_cast<int64_t>(raw.authenticatorId);
    token.authenticatorType =
        static_cast<aidl::android::hardware::keymaster::HardwareAuthenticatorType>(
            raw.authenticatorType);
    token.timestamp.milliSeconds = static_cast<int64_t>(raw.timestamp);
    token.mac.assign(raw.hmac, raw.hmac + sizeof(raw.hmac));
    return token;
}

bool isAuthTokenPlausible(const uint8_t in[kAuthTokenSize]) {
    AuthTokenRaw raw{};
    std::memcpy(&raw, in, kAuthTokenSize);
    if (raw.version != 0) {
        return false;
    }
    // Cheap sanity gate only: a token with a zero MAC or a non-zero version is
    // certainly garbage and must not be forwarded to the framework as an unlock.
    // This is NOT a cryptographic check - the HMAC is verified by the TA that
    // produced it; here we only reject the all-zero "engine did nothing" case.
    for (uint8_t byte : raw.hmac) {
        if (byte != 0) {
            return true;
        }
    }
    return false;
}

}  // namespace km6fod
