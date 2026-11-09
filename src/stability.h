// SPDX-License-Identifier: GPL-3.0-only
//
// Minimal VINTF-stability helper.
//
// The NDK ships libbinder_ndk with AIBinder_markVintfStability, but its public
// headers guard the declaration behind BINDER_STABILITY_SUPPORT (which the NDK
// sysroot does not define). AIDL HALs must mark every binder they hand to the
// framework, including the ISession and ICancellationSignal sub-objects, not
// just the root IFingerprint instance. Declaring the stable entry point here
// keeps the generated stubs untouched.

#pragma once

#include <android/binder_ibinder.h>
#include <android/log.h>

extern "C" binder_status_t AIBinder_markVintfStability(AIBinder* binder);

namespace km6fod {

inline void markStable(AIBinder* binder, const char* what) {
    if (binder == nullptr) {
        return;
    }
    const binder_status_t status = AIBinder_markVintfStability(binder);
    // On this libbinder the helper returns the resulting stability category
    // (3 == VINTF), not a STATUS_* code. Treat 0 and 3 as success; everything
    // else is a best-effort warning, never fatal: the root instance and the
    // VINTF manifest are what make the service discoverable.
    if (status != STATUS_OK && status != 3) {
        __android_log_print(ANDROID_LOG_DEBUG, "km6fod",
                            "stability: markVintfStability(%s) -> %d", what, status);
    }
}

}  // namespace km6fod
