// SPDX-License-Identifier: GPL-3.0-only

#include "touch_reader.h"

#include <fcntl.h>
#include <linux/input.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cerrno>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>

#include "logging.h"

namespace km6fod {
namespace {

constexpr const char* kInputDevices = "/proc/bus/input/devices";

int clampToRange(int value, int low, int high) {
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

int scaleAxis(int raw, int raw_min, int raw_max, int pixels) {
    const int span = raw_max - raw_min;
    if (span <= 0 || pixels <= 0) {
        return 0;
    }
    const int offset = clampToRange(raw, raw_min, raw_max) - raw_min;
    int scaled = static_cast<int>(static_cast<long long>(offset) * pixels / span);
    // raw_max maps to exactly `pixels`; the last valid pixel is pixels - 1.
    if (scaled >= pixels) {
        scaled = pixels - 1;
    }
    if (scaled < 0) {
        scaled = 0;
    }
    return scaled;
}

bool testAbsBit(int fd, int code) {
    uint8_t bits[(ABS_MAX + 7) / 8] = {};
    if (::ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(bits)), bits) < 0) {
        return false;
    }
    if (code < 0 || code > ABS_MAX) {
        return false;
    }
    return (bits[code / 8] & (1u << (code % 8))) != 0;
}

bool nameLooksLikeTouch(const std::string& name) {
    return name.find("mtk-tpd") != std::string::npos ||
           name.find("tpd") != std::string::npos ||
           name.find("touch") != std::string::npos ||
           name.find("Touch") != std::string::npos;
}

// Returns the best evdev path. Candidates are scored, not just picked first:
// a device that actually advertises multitouch axes wins over a keyboard whose
// name happens to contain "touch".
std::string findEventNode() {
    std::ifstream devices(kInputDevices);
    if (!devices.is_open()) {
        LOGE("touch: cannot read %s", kInputDevices);
        return {};
    }

    std::string best_path;
    int best_score = -1;
    std::string block;
    std::string line;

    auto harvest = [&](const std::string& candidate) {
        std::istringstream stream(candidate);
        std::string current;
        std::string event_name;
        std::string name;
        while (std::getline(stream, current)) {
            if (current.rfind("N: Name=", 0) == 0) {
                name = current.substr(8);
            } else if (current.rfind("H: Handlers=", 0) == 0) {
                std::istringstream handlers(current.substr(current.find('=') + 1));
                std::string token;
                while (handlers >> token) {
                    if (token.rfind("event", 0) == 0) {
                        event_name = token;
                    }
                }
            }
        }
        if (event_name.empty()) {
            return;
        }

        const std::string path = "/dev/input/" + event_name;
        int score = nameLooksLikeTouch(name) ? 2 : 0;

        // Do not trust the name alone: verify that the node actually exposes
        // multitouch coordinates. A single open/ioctl/close is cheap and happens
        // once per boot.
        const int fd = ::open(path.c_str(), O_RDONLY | O_CLOEXEC);
        if (fd >= 0) {
            if (testAbsBit(fd, ABS_MT_POSITION_X) && testAbsBit(fd, ABS_MT_POSITION_Y)) {
                score += 4;
            } else if (testAbsBit(fd, ABS_X) && testAbsBit(fd, ABS_Y)) {
                score += 2;
            }
            ::close(fd);
        }

        if (score > best_score) {
            best_score = score;
            best_path = path;
        }
    };

    while (std::getline(devices, line)) {
        if (line.empty()) {
            harvest(block);
            block.clear();
        } else {
            block += line;
            block += '\n';
        }
    }
    if (!block.empty()) {
        harvest(block);
    }

    if (best_score <= 0) {
        return {};
    }
    return best_path;
}

}  // namespace

TouchReader::~TouchReader() {
    stop();
}

std::string TouchReader::devicePath() const {
    std::lock_guard<std::mutex> lock(path_mutex_);
    return device_path_;
}

bool TouchReader::openDevice(DeviceInfo* out) {
    if (out == nullptr) {
        return false;
    }
    const std::string path = findEventNode();
    if (path.empty()) {
        LOGE("touch: no evdev node with position axes found");
        return false;
    }

    const int fd = ::open(path.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        LOGE("touch: open(%s) failed: %s", path.c_str(), std::strerror(errno));
        return false;
    }

    DeviceInfo info;
    info.fd = fd;
    info.path = path;

    struct input_absinfo abs_x {};
    struct input_absinfo abs_y {};
    if (::ioctl(fd, EVIOCGABS(ABS_MT_POSITION_X), &abs_x) == 0 && abs_x.maximum > abs_x.minimum) {
        info.raw_min_x = abs_x.minimum;
        info.raw_max_x = abs_x.maximum;
    } else if (::ioctl(fd, EVIOCGABS(ABS_X), &abs_x) == 0 && abs_x.maximum > abs_x.minimum) {
        info.raw_min_x = abs_x.minimum;
        info.raw_max_x = abs_x.maximum;
    }
    if (::ioctl(fd, EVIOCGABS(ABS_MT_POSITION_Y), &abs_y) == 0 && abs_y.maximum > abs_y.minimum) {
        info.raw_min_y = abs_y.minimum;
        info.raw_max_y = abs_y.maximum;
    } else if (::ioctl(fd, EVIOCGABS(ABS_Y), &abs_y) == 0 && abs_y.maximum > abs_y.minimum) {
        info.raw_min_y = abs_y.minimum;
        info.raw_max_y = abs_y.maximum;
    }

    LOGI("touch: using %s (x %d..%d, y %d..%d)", info.path.c_str(), info.raw_min_x,
         info.raw_max_x, info.raw_min_y, info.raw_max_y);
    *out = std::move(info);
    return true;
}

bool TouchReader::start(Callback callback, FodCallback fod_callback) {
    std::lock_guard<std::mutex> lifecycle(lifecycle_);
    if (running_.load(std::memory_order_acquire)) {
        return true;
    }
    fod_callback_ = std::move(fod_callback);
    DeviceInfo info;
    if (!openDevice(&info)) {
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(path_mutex_);
        device_path_ = info.path;
    }
    fd_.store(info.fd, std::memory_order_release);
    raw_min_x_ = info.raw_min_x;
    raw_max_x_ = info.raw_max_x;
    raw_min_y_ = info.raw_min_y;
    raw_max_y_ = info.raw_max_y;
    callback_ = std::move(callback);
    stop_.store(false, std::memory_order_release);
    running_.store(true, std::memory_order_release);
    thread_ = std::thread(&TouchReader::loop, this);
    return true;
}

void TouchReader::stop() {
    std::thread local;
    {
        std::lock_guard<std::mutex> lifecycle(lifecycle_);
        if (!running_.exchange(false, std::memory_order_acq_rel)) {
            const int fd = fd_.exchange(-1, std::memory_order_acq_rel);
            if (fd >= 0) {
                ::close(fd);
            }
            return;
        }
        stop_.store(true, std::memory_order_release);
        local = std::move(thread_);
    }
    if (!local.joinable()) {
        return;
    }
    if (local.get_id() == std::this_thread::get_id()) {
        LOGW("touch: stop() called from its own thread, detaching");
        local.detach();
        return;
    }
    local.join();

    const int fd = fd_.exchange(-1, std::memory_order_acq_rel);
    if (fd >= 0) {
        ::close(fd);
    }
    LOGI("touch: reader stopped");
}

void TouchReader::loop() {
    int raw_x = 0;
    int raw_y = 0;
    bool tracking = false;
    bool reported_down = false;
    int fd = fd_.load(std::memory_order_acquire);
    int syn_total = 0;
    int syn_inside = 0;

    struct input_event events[64];

    auto report_up = [&]() {
        if (reported_down && callback_) {
            callback_(0, 0, false);
        }
        reported_down = false;
    };

    while (!stop_.load(std::memory_order_acquire)) {
        if (fd < 0) {
            // The node disappeared. Try to re-resolve it, but stay off the CPU:
            // 200 ms between attempts is enough for a device re-enumeration.
            // Keep trying for as long as the process lives: the old code gave up after 5 s
            // and left running_ true, so the HAL believed it had a reader while no
            // touch ever arrived again (review H-05).
            for (int attempt = 0; !stop_.load(); ++attempt) {
                ::usleep(200 * 1000);
                DeviceInfo info;
                if (openDevice(&info)) {
                    fd = info.fd;
                    fd_.store(fd, std::memory_order_release);
                    {
                        std::lock_guard<std::mutex> lock(path_mutex_);
                        device_path_ = info.path;
                    }
                    raw_min_x_ = info.raw_min_x;
                    raw_max_x_ = info.raw_max_x;
                    raw_min_y_ = info.raw_min_y;
                    raw_max_y_ = info.raw_max_y;
                    tracking = false;
                    report_up();
                    if (attempt > 0) {
                        LOGW("touch: device re-opened after %d attempts", attempt);
                    }
                    break;
                }
                if (attempt > 0 && attempt % 25 == 0) {
                    LOGE("touch: still no evdev node after %d attempts (%.0f s)",
                         attempt, attempt * 0.2);
                }
            }
            if (fd < 0) {
                continue;
            }
            continue;
        }

        struct pollfd pfd {};
        pfd.fd = fd;
        pfd.events = POLLIN;
        // 100 ms slices instead of a full second: stop() used to wait up to a second
        // for this thread, which delayed every HAL shutdown (review H-48).
        int ready = 0;
        while (!stop_.load(std::memory_order_acquire)) {
            ready = ::poll(&pfd, 1, 100);
            if (ready != 0) {
                break;
            }
        }
        if (stop_.load(std::memory_order_acquire)) {
            break;
        }
        if (ready < 0) {
            if (errno == EINTR) {
                continue;
            }
            LOGE("touch: poll failed: %s", std::strerror(errno));
            report_up();
            ::close(fd);
            fd = -1;
            fd_.store(-1, std::memory_order_release);
            continue;
        }
        if (ready == 0) {
            continue;
        }
        if ((pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
            LOGW("touch: evdev hangup (revents=0x%x), reopening", pfd.revents);
            report_up();
            ::close(fd);
            fd = -1;
            fd_.store(-1, std::memory_order_release);
            continue;
        }
        if ((pfd.revents & POLLIN) == 0) {
            continue;
        }

        const ssize_t bytes = ::read(fd, events, sizeof(events));
        if (bytes < 0) {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            LOGE("touch: read failed: %s", std::strerror(errno));
            report_up();
            ::close(fd);
            fd = -1;
            fd_.store(-1, std::memory_order_release);
            continue;
        }
        if (bytes == 0) {
            continue;
        }

        const size_t count = static_cast<size_t>(bytes) / sizeof(struct input_event);
        for (size_t i = 0; i < count; ++i) {
            const struct input_event& ev = events[i];

            if (ev.type == EV_ABS) {
                switch (ev.code) {
                    case ABS_MT_POSITION_X:
                    case ABS_X:
                        raw_x = ev.value;
                        break;
                    case ABS_MT_POSITION_Y:
                    case ABS_Y:
                        raw_y = ev.value;
                        break;
                    case ABS_MT_TRACKING_ID:
                        tracking = ev.value >= 0;
                        break;
                    default:
                        break;
                }
            } else if (ev.type == EV_KEY && isFodKeyCode(ev.code)) {
                // FOD wake gesture from the touch controller (see the header).
                if (fod_callback_) {
                    fod_callback_(ev.value != 0);
                }
            } else if (ev.type == EV_KEY && ev.code == BTN_TOUCH) {
                tracking = ev.value != 0;
            } else if (ev.type == EV_SYN && ev.code == SYN_DROPPED) {
                // The kernel dropped events; the current tracking state is no
                // longer trustworthy. Release the finger and resynchronise.
                tracking = false;
                report_up();
            } else if (ev.type == EV_SYN && ev.code == SYN_REPORT) {
                const int px = scaleAxis(raw_x, raw_min_x_, raw_max_x_, kDisplayWidth);
                const int py = scaleAxis(raw_y, raw_min_y_, raw_max_y_, kDisplayHeight);

                const int dx = px - kSensorCenterX;
                const int dy = py - kSensorCenterY;
                const bool inside = tracking && dx > -kTouchTolerancePx &&
                                    dx < kTouchTolerancePx && dy > -kTouchTolerancePx &&
                                    dy < kTouchTolerancePx;

                // Instrumentation for bring-up: proves whether the digitizer node
                // is delivering samples at all and where the touch lands relative
                // to the sensor window. Rate-limited so it cannot flood logcat.
                ++syn_total;
                if (inside) {
                    ++syn_inside;
                }
                if ((syn_total % 50) == 0) {
                    LOGD("touch: syn=%d inside=%d raw=(%d,%d) px=(%d,%d) range=(%d..%d,%d..%d)",
                         syn_total, syn_inside, raw_x, raw_y, px, py, raw_min_x_, raw_max_x_,
                         raw_min_y_, raw_max_y_);
                }

                if (inside) {
                    // Stream every sample while the finger stays inside the
                    // sensor zone: the engine refines its estimate from
                    // successive coordinates.
                    if (callback_) {
                        callback_(px, py, true);
                    }
                    reported_down = true;
                } else if (reported_down) {
                    report_up();
                }
            }
        }
    }

    report_up();
    LOGI("touch: reader loop exited");
}

}  // namespace km6fod
