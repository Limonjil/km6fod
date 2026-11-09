// SPDX-License-Identifier: GPL-3.0-only

#include "hbm.h"

#include <fcntl.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "logging.h"

namespace km6fod {
namespace {

// Same node, two well-known routes. The aliased class path is the stable one;
// the platform path is the fallback used when tran_display is not linked.
constexpr const char* kCandidateNodes[] = {
    "/sys/kernel/tran_display/lcm_hbm_state",
    "/sys/devices/platform/soc/14013000.dsi/lcm_hbm_state",
};

// Panel driver semantics: 1 turns full HBM on, 0 restores normal brightness.
// Values 2 and 3 are accepted by the store handler but never reach the panel.
constexpr const char* kValueOn = "1";
constexpr const char* kValueOff = "0";

// Panel readback values. A successful ON is reported as 0xOK; OFF as 0xFA.
constexpr const char* kReadbackOn = "0xOK";

bool writeValue(int fd, const char* value) {
    if (fd < 0 || value == nullptr) {
        return false;
    }
    const size_t length = std::strlen(value);
    size_t written = 0;
    while (written < length) {
        const ssize_t rc = ::write(fd, value + written, length - written);
        if (rc < 0) {
            if (errno == EINTR) {
                continue;
            }
            return false;
        }
        if (rc == 0) {
            return false;
        }
        written += static_cast<size_t>(rc);
    }
    return true;
}

bool readValue(int fd, std::string* out) {
    if (fd < 0 || out == nullptr) {
        return false;
    }
    if (::lseek(fd, 0, SEEK_SET) < 0) {
        return false;
    }
    char buffer[64] = {};
    const ssize_t rc = ::read(fd, buffer, sizeof(buffer) - 1);
    if (rc <= 0) {
        return false;
    }
    std::string value(buffer, static_cast<size_t>(rc));
    while (!value.empty() &&
           (value.back() == '\n' || value.back() == '\r' || value.back() == ' ')) {
        value.pop_back();
    }
    *out = std::move(value);
    return true;
}

}  // namespace

// Wall-clock of the last wake the HAL injected, consumed once by applyOn().

// The panel needs roughly one frame plus its DSI command queue to settle after
// a wake; driving HBM earlier leaves a horizontal band until the next frame.
constexpr int64_t kFirstArmSettleMs = 180;

// Panel wake gate: the backlight node is zero while the display sleeps or dozes.
// Reading it is the only trustworthy "is the panel really up" signal the HAL has,
// and it is readable from this domain after all (system:system, 0664).
constexpr const char* kBacklightNode = "/sys/class/leds/lcd-backlight/brightness";

// Positive: the panel is lit. Zero: asleep or dozing. Negative: unreadable.


int readBacklightLevel() {
    const int fd = ::open(kBacklightNode, O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        return -1;
    }
    char buf[32] = {};
    const ssize_t n = ::read(fd, buf, sizeof(buf) - 1);
    ::close(fd);
    if (n <= 0) {
        return -1;
    }
    return std::atoi(buf);
}

int64_t steadyNowMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

int panelBacklightLevel() {
    return readBacklightLevel();
}

HbmController& HbmController::instance() {
    // Intentionally leaked: the HBM thread may outlive static destructors, and
    // the vendor library must never be torn down while its threads are alive.
    static HbmController* controller = new HbmController();
    return *controller;
}

void HbmController::start() {
    if (running_.exchange(true)) {
        return;
    }
    shutting_down_.store(false, std::memory_order_release);
    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        want_on_ = false;
        ++epoch_;
        armDeadlineLocked(Clock::time_point::max());
    }
    {
        std::lock_guard<std::mutex> lock(io_mutex_);
        ensureNodeLocked();
    }
    worker_ = std::thread(&HbmController::workerLoop, this);
    guard_ = std::thread(&HbmController::guardLoop, this);
    LOGI("hbm: controller started");
}

void HbmController::stop() {
    const bool was_running = running_.exchange(false);
    shutting_down_.store(true, std::memory_order_release);
    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        want_on_ = false;
        on_.store(false, std::memory_order_release);
        ++epoch_;
        armDeadlineLocked(Clock::time_point::max());
    }
    cv_.notify_all();
    guard_cv_.notify_all();

    auto join = [](std::thread& thread, const char* name) {
        if (!thread.joinable()) {
            return;
        }
        if (thread.get_id() == std::this_thread::get_id()) {
            LOGW("hbm: %s thread cannot join itself, detaching", name);
            thread.detach();
            return;
        }
        thread.join();
    };
    join(worker_, "worker");
    join(guard_, "guard");

    // Never leave the panel in HBM because we are shutting down.
    writeOffIndependent("stop");
    {
        std::lock_guard<std::mutex> lock(io_mutex_);
        closeNodeLocked();
    }
    if (was_running) {
        LOGI("hbm: controller stopped");
    }
}

bool HbmController::ensureNodeLocked() {
    if (fd_ >= 0) {
        return true;
    }
    for (const char* candidate : kCandidateNodes) {
        int fd = ::open(candidate, O_RDWR | O_NONBLOCK | O_CLOEXEC);
        bool readable = fd >= 0;
        if (fd < 0) {
            fd = ::open(candidate, O_WRONLY | O_NONBLOCK | O_CLOEXEC);
        }
        if (fd < 0) {
            continue;
        }
        fd_ = fd;
        node_readable_ = readable;
        node_path_ = candidate;
        LOGI("hbm: using node %s%s", candidate, readable ? " (readback)" : " (write-only)");
        return true;
    }
    LOGE("hbm: no writable panel node found (checked %zu candidates)",
         sizeof(kCandidateNodes) / sizeof(kCandidateNodes[0]));
    return false;
}

void HbmController::closeNodeLocked() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
    node_readable_ = false;
    node_path_.clear();
}

bool HbmController::writeAllLocked(int fd, const char* value) {
    return writeValue(fd, value);
}

bool HbmController::readBackLocked(int fd, std::string* value) {
    return readValue(fd, value);
}

void HbmController::armDeadlineLocked(Clock::time_point deadline) {
    deadline_ = deadline;
    guard_cv_.notify_all();
}

namespace {

// A write during the panel's wake sequence leaves a bright rolling band across
// the display; the same write on a settled panel is clean (measured on the
// device). Zero backlight means asleep or dozing, so wait for the panel to light
// and then let a few frames pass. Aborts on shutdown or a newer epoch.
constexpr int64_t kPanelSettleMs = 180;
constexpr int64_t kPanelWaitMaxMs = 800;
constexpr int64_t kPanelPollMs = 10;

bool waitForSettledPanel(const std::atomic<bool>& shutting_down,
                         const std::atomic<uint64_t>& epoch_now, uint64_t epoch) {
    if (readBacklightLevel() > 0) {
        return true;  // already lit: this is not the wake path
    }
    int64_t lit_at = 0;
    const int64_t started = steadyNowMs();
    while (steadyNowMs() - started < kPanelWaitMaxMs) {
        if (shutting_down.load(std::memory_order_acquire) ||
            epoch_now.load(std::memory_order_acquire) != epoch) {
            return false;
        }
        const int64_t now = steadyNowMs();
        if (lit_at == 0 && readBacklightLevel() > 0) {
            lit_at = now;
        }
        if (lit_at != 0 && now - lit_at >= kPanelSettleMs) {
            return true;
        }
        ::usleep(kPanelPollMs * 1000);
    }
    return true;  // never lit: let the write run, the readback still rejects it
}

}  // namespace

bool HbmController::applyOn(uint64_t epoch) {

    // The panel's own wake sequence must finish before HBM is armed, otherwise
    // the flash leaves a bright rolling band until the next frame. This waits
    // outside io_mutex_, so requestOff and shutdown are never blocked by it.
    if (!waitForSettledPanel(shutting_down_, epoch_, epoch)) {
        return false;
    }
    {
        std::lock_guard<std::mutex> lock(io_mutex_);
        if (shutting_down_.load(std::memory_order_acquire) ||
            epoch_.load(std::memory_order_acquire) != epoch) {
            return false;
        }
        if (!ensureNodeLocked()) {
            return false;
        }
        writing_.store(true, std::memory_order_release);
        if (!writeAllLocked(fd_, kValueOn)) {
            writing_.store(false, std::memory_order_release);
            LOGE("hbm: write(%s) failed: %s", node_path_.c_str(), std::strerror(errno));
            closeNodeLocked();
            return false;
        }
        // Close the shutdown race: if forceOffNow() ran while this write was in
        // flight, the final state must be OFF, not the "1" that just landed.
        if (shutting_down_.load(std::memory_order_acquire) ||
            epoch_.load(std::memory_order_acquire) != epoch) {
            writeAllLocked(fd_, kValueOff);
            writing_.store(false, std::memory_order_release);
            return false;
        }

        // A write is not proof of effect: the panel store handler silently
        // ignores the command when the display is off or idle. Verify 0xOK.
        if (node_readable_) {
            bool engaged = false;
            for (int attempt = 0; attempt < 3; ++attempt) {
                std::string value;
                if (!readBackLocked(fd_, &value)) {
                    // Some kernels expose the attribute write-only despite
                    // accepting O_RDWR; fall back to trusting the write.
                    LOGW("hbm: readback unavailable, trusting write");
                    node_readable_ = false;
                    engaged = true;
                    break;
                }
                if (value.find(kReadbackOn) != std::string::npos) {
                    engaged = true;
                    break;
                }
                LOGW("hbm: panel not in HBM yet (read '%s'), retry %d", value.c_str(),
                     attempt + 1);
                ::usleep(20 * 1000);
                if (shutting_down_.load(std::memory_order_acquire) ||
                    epoch_.load(std::memory_order_acquire) != epoch) {
                    break;
                }
                if (!writeAllLocked(fd_, kValueOn)) {
                    LOGE("hbm: retry write failed: %s", std::strerror(errno));
                    closeNodeLocked();
                    return false;
                }
            }
            if (!engaged) {
                LOGW("hbm: display is off/idle, refusing to open the capture gate");
                // Best effort: leave no half-open state behind.
                writeAllLocked(fd_, kValueOff);
                writing_.store(false, std::memory_order_release);
                return false;
            }
        }
        writing_.store(false, std::memory_order_release);
    }

    // The panel driver needs time to actually ramp the backlight before the
    // sensor is asked for a frame. Sleep in slices so requestOff()/shutdown can
    // abort the window almost immediately.
    for (int elapsed = 0; elapsed < kSettleMs; elapsed += 10) {
        if (shutting_down_.load(std::memory_order_acquire) ||
            epoch_.load(std::memory_order_acquire) != epoch) {
            return false;
        }
        ::usleep(10 * 1000);
    }
    if (shutting_down_.load(std::memory_order_acquire) ||
        epoch_.load(std::memory_order_acquire) != epoch) {
        return false;
    }

    // Tell the engine that full-HBM is in effect; this is the gate it blocks on.
    const bool notified = uevent_.sendFullHbmSet();
    if (!notified) {
        LOGW("hbm: FULL_HBM_SET injection failed; engine gate may stay closed");
    }
    if (shutting_down_.load(std::memory_order_acquire) ||
        epoch_.load(std::memory_order_acquire) != epoch) {
        uevent_.sendFullHbmClear();
        return false;
    }
    LOGI("hbm: on (settle %d ms, uevent %s)", kSettleMs, notified ? "sent" : "FAILED");
    return true;
}

void HbmController::applyOffNormal() {
    // Clear first so the engine stops treating the panel as illuminated even if
    // the brightness write fails below.
    uevent_.sendFullHbmClear();

    // Always write. Skipping the write when the backlight was zero ("the driver reset
    // the node itself") left the panel in HBM with the sensor area glowing after an
    // unlock (measured: node 0xOK, log "hbm: off by panel sleep"). A write on a
    // sleeping panel is ignored by the driver, so it costs nothing.

    std::lock_guard<std::mutex> lock(io_mutex_);
    if (!ensureNodeLocked()) {
        return;
    }
    bool off = false;
    writing_.store(true, std::memory_order_release);
    for (int attempt = 0; attempt < 6; ++attempt) {
        if (!writeAllLocked(fd_, kValueOff)) {
            writing_.store(false, std::memory_order_release);
            LOGE("hbm: off write failed: %s", std::strerror(errno));
            closeNodeLocked();
            return;
        }
        if (!node_readable_) {
            off = true;
            break;
        }
        std::string value;
        if (!readBackLocked(fd_, &value)) {
            node_readable_ = false;
            off = true;
            break;
        }
        if (value.find(kReadbackOn) == std::string::npos) {
            off = true;
            break;
        }
        LOGW("hbm: panel still reports HBM on (read '%s'), retry %d", value.c_str(),
             attempt + 1);
        ::usleep(10 * 1000);
    }
    writing_.store(false, std::memory_order_release);
    if (off) {
        LOGI("hbm: off");
    } else {
        LOGE("hbm: could not verify HBM off");
    }
}

bool HbmController::writeOffIndependent(const char* reason) {
    uevent_.sendFullHbmClear();

    for (const char* path : kCandidateNodes) {
        for (int attempt = 0; attempt < 3; ++attempt) {
            int fd = ::open(path, O_RDWR | O_NONBLOCK | O_CLOEXEC);
            const bool readable = fd >= 0;
            if (fd < 0) {
                fd = ::open(path, O_WRONLY | O_NONBLOCK | O_CLOEXEC);
            }
            if (fd < 0) {
                break;
            }
            bool off = writeValue(fd, kValueOff);
            if (off && readable) {
                std::string value;
                if (readValue(fd, &value)) {
                    off = value.find(kReadbackOn) == std::string::npos;
                }
            }
            ::close(fd);
            if (off) {
                LOGW("hbm: forced off (%s) by %s", path, reason);
                return true;
            }
            ::usleep(10 * 1000);
        }
    }
    LOGE("hbm: force-off by %s could not be verified", reason);
    return false;
}

bool HbmController::requestOn() {
    if (shutting_down_.load(std::memory_order_acquire) ||
        !running_.load(std::memory_order_acquire)) {
        return false;
    }

    uint64_t ticket = 0;
    {
        std::unique_lock<std::mutex> lock(state_mutex_);

        // Window already open (a retry inside one operation): re-assert the
        // engine gate, but check the panel first. After a suspend the driver
        // drops HBM while this flag still says on, and trusting the flag is what
        // made captures run in the dark until something happened to reset it.
        if (on_.load(std::memory_order_acquire)) {
            want_on_ = true;
            const auto now = Clock::now();
            last_activity_ = now;
            armDeadlineLocked(std::min(now + std::chrono::milliseconds(kInactivityMs),
                                       opened_at_ + std::chrono::milliseconds(kAbsoluteMaxMs)));
            lock.unlock();
            if (shutting_down_.load(std::memory_order_acquire)) {
                return false;
            }
            if (panelStillInHbm()) {
                uevent_.sendFullHbmSet();
                return true;
            }
            LOGW("hbm: flag says open but the panel is not in HBM, re-applying");
            lock.lock();
            on_.store(false, std::memory_order_release);
        }

        want_on_ = true;
        ticket = ++requested_seq_;
        const auto now = Clock::now();
        last_activity_ = now;
        armDeadlineLocked(now + std::chrono::milliseconds(kApplyHardLimitMs));
        cv_.notify_all();

        const auto deadline = now + std::chrono::milliseconds(kRequestTimeoutMs);
        if (!cv_.wait_until(lock, deadline, [&] { return applied_seq_ >= ticket; })) {
            LOGW("hbm: request timed out after %d ms", kRequestTimeoutMs);
            // Roll the request back: the worker may still be inside a blocked
            // write. The guard will force the panel off.
            want_on_ = false;
            ++epoch_;
            armDeadlineLocked(Clock::now());
            cv_.notify_all();
            return false;
        }
        return on_.load(std::memory_order_acquire);
    }
}

bool HbmController::panelStillInHbm() {
    std::lock_guard<std::mutex> lock(io_mutex_);
    if (!node_readable_ || fd_ < 0) {
        // The node cannot be read back on this kernel; the write is the only
        // evidence available.
        return true;
    }
    std::string value;
    if (!readBackLocked(fd_, &value)) {
        return true;
    }
    return value.find(kReadbackOn) != std::string::npos;
}

void HbmController::requestOff() {
    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        if (!want_on_ && !on_.load(std::memory_order_acquire)) {
            return;
        }
        want_on_ = false;
        ++epoch_;
        // Give the normal worker a short grace period; the guard is the
        // backstop if the worker is stuck.
        armDeadlineLocked(Clock::now() + std::chrono::milliseconds(kOffGraceMs));
    }
    cv_.notify_all();
}

void HbmController::keepAlive() {
    if (!on_.load(std::memory_order_acquire) ||
        shutting_down_.load(std::memory_order_acquire)) {
        return;
    }
    std::lock_guard<std::mutex> lock(state_mutex_);
    if (!want_on_) {
        return;
    }
    const auto now = Clock::now();
    last_activity_ = now;
    armDeadlineLocked(std::min(now + std::chrono::milliseconds(kInactivityMs),
                               opened_at_ + std::chrono::milliseconds(kAbsoluteMaxMs)));
}

void HbmController::forceOffNow() {
    shutting_down_.store(true, std::memory_order_release);
    running_.store(false, std::memory_order_release);
    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        want_on_ = false;
        on_.store(false, std::memory_order_release);
        ++epoch_;
        armDeadlineLocked(Clock::time_point::max());
    }
    cv_.notify_all();
    guard_cv_.notify_all();

    // The worker may be inside a sysfs write. Wait for it to leave the critical
    // section, force off, then repeat if it managed to write "1" after us.
    for (int pass = 0; pass < 3; ++pass) {
        for (int i = 0; i < 40 && writing_.load(std::memory_order_acquire); ++i) {
            ::usleep(5 * 1000);
        }
        writeOffIndependent(pass == 0 ? "shutdown" : "shutdown-final");
        if (!writing_.load(std::memory_order_acquire)) {
            break;
        }
    }
}

void HbmController::workerLoop() {
    std::unique_lock<std::mutex> lock(state_mutex_);

    while (running_.load(std::memory_order_acquire)) {
        const bool want = want_on_;
        const bool is_on = on_.load(std::memory_order_acquire);

        if (!want) {
            if (is_on) {
                lock.unlock();
                applyOffNormal();
                lock.lock();
                on_.store(false, std::memory_order_release);
                applied_seq_ = requested_seq_;
                armDeadlineLocked(Clock::time_point::max());
                cv_.notify_all();
                continue;
            }
            // No window: sleep indefinitely. This is what keeps the SoC out of
            // the 10-wakeups-per-second trap the old 100 ms loop created.
            cv_.wait(lock);
            continue;
        }

        if (!is_on) {
            const uint64_t epoch = epoch_.load(std::memory_order_acquire);
            lock.unlock();
            const bool ok = applyOn(epoch);
            lock.lock();

            const bool still_wanted = want_on_ && running_.load(std::memory_order_acquire) &&
                                      !shutting_down_.load(std::memory_order_acquire) &&
                                      epoch_.load(std::memory_order_acquire) == epoch;
            if (ok && still_wanted) {
                on_.store(true, std::memory_order_release);
                const auto now = Clock::now();
                opened_at_ = now;
                last_activity_ = now;
                armDeadlineLocked(std::min(now + std::chrono::milliseconds(kInactivityMs),
                                           now + std::chrono::milliseconds(kAbsoluteMaxMs)));
            } else {
                on_.store(false, std::memory_order_release);
                want_on_ = false;
                ++epoch_;
                armDeadlineLocked(Clock::time_point::max());
                lock.unlock();
                // Also covers the "applyOn returned false after a partial
                // write" case; an off write is harmless.
                applyOffNormal();
                lock.lock();
            }
            // Satisfy every waiter up to the latest ticket, not just the one
            // captured before applyOn.
            applied_seq_ = requested_seq_;
            cv_.notify_all();
            continue;
        }

        // Window is open. Wait for the inactivity/absolute deadline; the guard
        // thread independently enforces the same deadline.
        if (deadline_ == Clock::time_point::max()) {
            cv_.wait(lock);
            continue;
        }
        if (cv_.wait_until(lock, deadline_) != std::cv_status::timeout) {
            continue;
        }
        if (!running_.load(std::memory_order_acquire) || !want_on_ ||
            !on_.load(std::memory_order_acquire)) {
            continue;
        }
        if (Clock::now() < deadline_) {
            continue;
        }

        want_on_ = false;
        ++epoch_;
        on_.store(false, std::memory_order_release);
        const auto age = std::chrono::duration_cast<std::chrono::milliseconds>(
                             Clock::now() - opened_at_)
                             .count();
        lock.unlock();
        applyOffNormal();
        lock.lock();
        applied_seq_ = requested_seq_;
        armDeadlineLocked(Clock::time_point::max());
        cv_.notify_all();
        LOGW("hbm: watchdog closed a window open for %lld ms", static_cast<long long>(age));
    }
}

void HbmController::guardLoop() {
    std::unique_lock<std::mutex> lock(state_mutex_);

    while (running_.load(std::memory_order_acquire)) {
        if (deadline_ == Clock::time_point::max()) {
            guard_cv_.wait(lock);
            continue;
        }
        if (guard_cv_.wait_until(lock, deadline_) != std::cv_status::timeout) {
            continue;
        }
        if (!running_.load(std::memory_order_acquire)) {
            break;
        }
        if (Clock::now() < deadline_) {
            continue;
        }
        if (!want_on_ && !on_.load(std::memory_order_acquire)) {
            deadline_ = Clock::time_point::max();
            continue;
        }

        // Hard fail-safe: the worker may be blocked in a sysfs write, so use a
        // fresh fd and do not take io_mutex_. The epoch bump makes the worker
        // undo any ON that completes after this point.
        want_on_ = false;
        on_.store(false, std::memory_order_release);
        ++epoch_;
        deadline_ = Clock::time_point::max();
        lock.unlock();
        const bool off = writeOffIndependent("hard watchdog");
        lock.lock();
        applied_seq_ = requested_seq_;
        cv_.notify_all();
        if (!off) {
            LOGE("hbm: hard watchdog could not verify the panel is off");
        }
    }
}

}  // namespace km6fod
