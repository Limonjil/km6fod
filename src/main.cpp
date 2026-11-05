// SPDX-License-Identifier: GPL-2.0-only
//
// Service entry point for the KM6 AIDL fingerprint HAL.
//
// Deliberately minimal: acquire a single-instance lock, block termination
// signals in every thread except one sigwait thread, register the instance, then
// park in the Binder thread pool. Everything else is driven by framework calls
// or by the vendor engine's own threads.

#include <android/binder_ibinder.h>
#include <android/binder_interface_utils.h>
#include <android/binder_status.h>

#include "paths.h"

#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <cstring>
#include <thread>

#include "fingerprint_hal.h"
#include "hbm.h"
#include "logging.h"
#include "stability.h"

// libbinder_ndk exports these C entry points; the NDK sysroot does not ship the
// <android/binder_manager.h> / <android/binder_process.h> wrappers, so they are
// declared here against the stable LIBBINDER_NDK symbol version.
extern "C" {
binder_status_t AServiceManager_addService(AIBinder* binder, const char* instance);
void ABinderProcess_setThreadPoolMaxThreadCount(uint32_t num_threads);
void ABinderProcess_startThreadPool(void);
void ABinderProcess_joinThreadPool(void);
}

namespace {

constexpr const char* kInstanceName =
    "android.hardware.biometrics.fingerprint.IFingerprint/default";

// The engine keeps its templates and calibration here. It will not create the
// directory itself, and enrollment silently fails when it is missing.
constexpr const char* kStorePath = "/data/vendor_de/0/fpdata";

constexpr const char* kRunDir = km6fod::kRunDir;
constexpr const char* kLockPath = km6fod::kLockPath;
constexpr const char* kFallbackLockPath = "/data/local/tmp/km6fod.lock";

void makeDir(const char* path, mode_t mode) {
    if (::mkdir(path, mode) == 0) {
        LOGI("main: created %s", path);
        return;
    }
    if (errno != EEXIST) {
        LOGW("main: mkdir(%s) failed: %s", path, std::strerror(errno));
    }
}

void ensureStorePath() {
    makeDir("/data/vendor_de", 0771);
    makeDir("/data/vendor_de/0", 0771);
    makeDir(kStorePath, 0700);
    // A permissive umask must not widen the template store.
    ::chmod(kStorePath, 0700);
}

// Returns an open fd holding the exclusive lock, -1 when another instance owns
// it, or -2 when no lock file can be created.
int acquireInstanceLock() {
    makeDir("/data/adb", 0755);
    makeDir("/data/adb/km6fod", 0755);
    makeDir(kRunDir, 0755);

    const char* candidates[] = {kLockPath, kFallbackLockPath};
    for (const char* path : candidates) {
        const int fd = ::open(path, O_RDWR | O_CREAT | O_CLOEXEC, 0600);
        if (fd < 0) {
            continue;
        }
        if (::flock(fd, LOCK_EX | LOCK_NB) == 0) {
            LOGI("main: instance lock acquired: %s", path);
            return fd;
        }
        const int saved = errno;
        ::close(fd);
        if (saved == EWOULDBLOCK || saved == EAGAIN) {
            return -1;
        }
    }
    return -2;
}

// All threads inherit the blocked signal mask. Only this sigwait thread ever
// observes SIGTERM/SIGINT, so no signal handler can race a worker's sysfs write.
// forceOffNow() uses a fresh fd and an epoch invalidation, then _exit().
void installSignalHandling() {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGHUP);
    if (::pthread_sigmask(SIG_BLOCK, &set, nullptr) != 0) {
        LOGE("main: pthread_sigmask failed: %s", std::strerror(errno));
    }

    struct sigaction ignore {};
    ignore.sa_handler = SIG_IGN;
    sigemptyset(&ignore.sa_mask);
    ::sigaction(SIGPIPE, &ignore, nullptr);

    // Construct the HBM singleton before the thread starts: forceOffNow() must
    // never allocate or race a lazy static initialisation in the signal path.
    km6fod::HbmController::instance();

    std::thread([set] {
        int signal_number = 0;
        for (;;) {
            if (::sigwait(&set, &signal_number) != 0) {
                continue;
            }
            LOGW("main: received signal %d, forcing HBM off", signal_number);
            km6fod::HbmController::instance().forceOffNow();
            ::_exit(0);
        }
    }).detach();
}

}  // namespace

int main() {
    ::umask(0077);
    ensureStorePath();

    const int lock_fd = acquireInstanceLock();
    if (lock_fd == -1) {
        LOGW("main: another fp_hal instance holds the lock, exiting");
        return 0;
    }
    if (lock_fd == -2) {
        // Running as the unprivileged `system` user under init, /data/adb and
        // /data/local/tmp are not writable. Not having a lock file is not a
        // reason to leave the device without a fingerprint sensor; init already
        // guarantees a single instance for a service definition.
        LOGW("main: no instance lock available, continuing without one");
    }
    (void)lock_fd;  // held for the process lifetime

    installSignalHandling();

    LOGI("main: starting KM6 FOD HAL");

    // Four worker threads: the framework issues a few concurrent queries during
    // boot (props, authenticator id, session setup). Every long operation is
    // offloaded to SerialWorker, so these threads never block on vendor I/O.
    ABinderProcess_setThreadPoolMaxThreadCount(4);
    ABinderProcess_startThreadPool();

    auto hal = ::ndk::SharedRefBase::make<km6fod::FingerprintHal>();
    hal->start();

    // Mark VINTF stability before publishing: this is what makes the instance
    // discoverable through the VINTF manifest by the framework HAL loader.
    km6fod::markStable(hal->asBinder().get(), "IFingerprint/default");

    // The module starts as soon as servicemanager is up, which may still be a
    // moment before it accepts registrations. Retry instead of exiting: an early
    // registration is what keeps the AIDL sensor in the framework's list from
    // the very beginning of the boot.
    binder_status_t status = STATUS_UNKNOWN_ERROR;
    for (int attempt = 0; attempt < 300; ++attempt) {
        status = AServiceManager_addService(hal->asBinder().get(), kInstanceName);
        if (status == STATUS_OK) {
            break;
        }
        if (attempt == 0 || (attempt % 20) == 0) {
            LOGW("main: addService attempt %d failed: %d", attempt + 1, status);
        }
        ::usleep(200 * 1000);
    }
    if (status != STATUS_OK) {
        LOGE("main: addService(%s) failed: %d", kInstanceName, status);
        hal->stop();
        return 1;
    }

    LOGI("main: registered %s", kInstanceName);

    // Does not return: the process ends through the sigwait path in main(), which
    // forces the illumination off and exits (review H-50).
    ABinderProcess_joinThreadPool();
    _exit(0);
}
