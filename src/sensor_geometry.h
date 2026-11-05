// SPDX-License-Identifier: GPL-2.0-only
#pragma once

#include <cstdint>

// One source of truth for the sensor area. It used to be duplicated: the session
// accepted a 500x500 square while the framework is told a circle of r=100, so a
// press up to 250 px away lit the sensor and started a capture the framework never
// asked for (review H-14, H-42).
namespace km6fod {

constexpr int32_t kSensorCenterX = 560;
constexpr int32_t kSensorCenterY = 2205;
// Slightly larger than the framework's reported radius: the touch controller and
// the optical sensor do not share a frame of reference exactly, and a press that
// the framework considers on-target must never be rejected here.
constexpr int32_t kSensorRadiusPx = 120;
constexpr int32_t kPanelWidthPx = 1080;
constexpr int32_t kPanelHeightPx = 2400;

inline bool inSensorZone(int32_t x, int32_t y) {
    const int32_t dx = x - kSensorCenterX;
    const int32_t dy = y - kSensorCenterY;
    return dx * dx + dy * dy <= kSensorRadiusPx * kSensorRadiusPx;
}

}  // namespace km6fod
