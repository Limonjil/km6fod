// SPDX-License-Identifier: GPL-2.0-only
//
// Single-thread deadline scheduler.
//
// The HAL needs several independent timeouts (operation timeout, cancel
// escalation, HBM watchdog). A thread per timeout is wasteful and a periodic
// tick would keep the CPU (and the whole SoC) out of deep sleep. Instead every
// component posts a one-shot deadline here and the scheduler sleeps in the
// kernel until the earliest one is due. With no pending work the thread is
// blocked indefinitely.
//
// Tasks run on the scheduler thread and must be short. Anything that can block
// on vendor I/O or Binder is posted to the SerialWorker / CallbackDispatcher
// instead.

#pragma once

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace km6fod {

class DeadlineScheduler {
  public:
    using Clock = std::chrono::steady_clock;
    using Task = std::function<void()>;

    // Bounded on purpose: an unbounded timer queue is an OOM vector.
    static constexpr std::size_t kMaxPending = 64;

    DeadlineScheduler() = default;
    ~DeadlineScheduler();

    DeadlineScheduler(const DeadlineScheduler&) = delete;
    DeadlineScheduler& operator=(const DeadlineScheduler&) = delete;

    void start();
    void stop();

    // Returns 0 when the scheduler is stopped or the queue is full.
    uint64_t scheduleAfter(std::chrono::milliseconds delay, Task task);

    // Removes a pending deadline. Safe with an unknown/expired id.
    void cancel(uint64_t id);

    std::size_t pending() const;

  private:
    struct Entry {
        Clock::time_point when;
        uint64_t id;
        Task task;
    };

    void loop();

    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::vector<Entry> entries_;
    std::thread thread_;
    bool running_ = false;
    uint64_t next_id_ = 1;
};

}  // namespace km6fod
