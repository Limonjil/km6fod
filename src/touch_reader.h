// SPDX-License-Identifier: GPL-3.0-only
//
// Touch reporting for the under-display sensor.
//
// AOSP on this GSI configures the HAL with halHandlesDisplayTouches=false and
// never forwards UDFPS pointer events, so the engine would never learn where the
// finger is. We therefore read the mtk-tpd evdev node directly and push scaled
// pixel coordinates into the sensor ourselves.
//
// Ranges are read from the device via EVIOCGABS rather than hardcoded, so a
// panel or digitizer change does not silently skew the coordinates.
//
// The reader blocks in poll(); it does not busy-poll. An idle reader must not
// generate hundreds of wakeups per second, otherwise the SoC never reaches
// deep sleep.

#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

#include "sensor_geometry.h"

namespace km6fod {

class TouchReader {
  public:
    // x, y in display pixels, down = finger present.
    using Callback = std::function<void(int x, int y, bool down)>;

    // FOD wake gesture from the touch controller: the driver reports EV_KEY with
    // code 0xbe (KEY_F20, gesture id 37) or 0xb7 (KEY_F13, gesture id 36 -> 0xcc)
    // when the sensor zone is pressed while the panel is off. This is the only way
    // to learn about the touch before the display wakes.
    using FodCallback = std::function<void(bool down)>;

    static constexpr bool isFodKeyCode(int code) { return code == 0xbe || code == 0xb7; }

    TouchReader() = default;
    ~TouchReader();

    TouchReader(const TouchReader&) = delete;
    TouchReader& operator=(const TouchReader&) = delete;

    bool start(Callback callback, FodCallback fod_callback = nullptr);
    void stop();
    bool isRunning() const { return running_.load(std::memory_order_acquire); }
    std::string devicePath() const;

    // Display geometry used to map digitizer units to pixels.
    static constexpr int kDisplayWidth = 1080;
    static constexpr int kDisplayHeight = 2400;

    // Sensor centre and acceptance radius in display pixels.
    static constexpr int kSensorCenterX = 560;
    static constexpr int kSensorCenterY = 2205;
    static constexpr int kTouchTolerancePx = 250;

  private:
    struct DeviceInfo {
        int fd = -1;
        std::string path;
        int raw_min_x = 0;
        int raw_max_x = 17279;
        int raw_min_y = 0;
        int raw_max_y = 38399;
    };

    void loop();
    bool openDevice(DeviceInfo* out);

    std::mutex lifecycle_;
    std::thread thread_;
    std::atomic<bool> running_{false};
    std::atomic<bool> stop_{false};
    std::atomic<int> fd_{-1};

    mutable std::mutex path_mutex_;
    std::string device_path_;

    Callback callback_;
    FodCallback fod_callback_;

    // Written by start() before the thread exists, then only by the reader
    // thread during a reconnect. Never read by another thread.
    int raw_min_x_ = 0;
    int raw_max_x_ = 17279;
    int raw_min_y_ = 0;
    int raw_max_y_ = 38399;
};

}  // namespace km6fod
