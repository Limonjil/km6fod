// SPDX-License-Identifier: GPL-2.0-only
//
// Unified logging for the KM6 FOD HAL.
//
// Every record goes to logcat (tag "km6fod") and to stderr, because the HAL is
// started by a Magisk service script whose stdout is captured into a file. Both
// sinks are useful: logcat for correlation with the vendor engine, the file for
// post-mortem when the process dies before logd is reachable.

#pragma once

#include <android/log.h>

#include <cstdarg>
#include <cstdio>

namespace km6fod {

enum class Level : int {
    kDebug = ANDROID_LOG_DEBUG,
    kInfo = ANDROID_LOG_INFO,
    kWarn = ANDROID_LOG_WARN,
    kError = ANDROID_LOG_ERROR,
};

namespace detail {

inline void logWrite(Level level, const char* fmt, va_list ap) {
    va_list copy;
    va_copy(copy, ap);
    __android_log_vprint(static_cast<int>(level), "km6fod", fmt, ap);
    std::vfprintf(stderr, fmt, copy);
    std::fputc('\n', stderr);
    std::fflush(stderr);
    va_end(copy);
}

}  // namespace detail

inline void logf(Level level, const char* fmt, ...) __attribute__((format(printf, 2, 3)));

inline void logf(Level level, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    detail::logWrite(level, fmt, ap);
    va_end(ap);
}

#define LOGD(...) ::km6fod::logf(::km6fod::Level::kDebug, __VA_ARGS__)
#define LOGI(...) ::km6fod::logf(::km6fod::Level::kInfo, __VA_ARGS__)
#define LOGW(...) ::km6fod::logf(::km6fod::Level::kWarn, __VA_ARGS__)
#define LOGE(...) ::km6fod::logf(::km6fod::Level::kError, __VA_ARGS__)

}  // namespace km6fod
