// SPDX-License-Identifier: GPL-3.0-only
//
// Two small execution helpers shared by the HAL.
//
//   CallbackDispatcher -- serialises vendor-engine notifications and every
//     HAL -> framework callback onto a single thread. The engine posts from
//     arbitrary internal threads while holding its own locks; calling Binder
//     from there risks re-entering the engine and deadlocking the capture loop.
//
//   SerialWorker -- a single background thread for every call into the vendor
//     engine (open, set_active_group, enroll, authenticate, enumerate, remove,
//     pre/post enroll, authenticator id). A Binder thread must never block on
//     vendor I/O: the pool has only four threads and a hung vendor call would
//     exhaust it, stalling the whole biometric scheduler.
//
// Lifecycle contract: Running -> Stopping -> Stopped. start()/stop() are
// serialised, stop() is idempotent, and a thread may not join itself. stop()
// refuses new work, runs what is already queued, then joins: a dropped task is
// a callback the framework waits for until its own timeout (review H-31).

#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>

namespace km6fod {

class CallbackDispatcher {
  public:
    // Engine notifications are rare, but a burst during enumerate must not be
    // lost. On overflow the OLDEST entry is dropped: it is almost always a
    // stale onAcquired sample, while terminal events are the newest.
    static constexpr std::size_t kMaxQueue = 1024;

    CallbackDispatcher() = default;
    ~CallbackDispatcher();

    CallbackDispatcher(const CallbackDispatcher&) = delete;
    CallbackDispatcher& operator=(const CallbackDispatcher&) = delete;

    void start();
    void stop();

    // Queues a task. Never blocks the producer. Returns false when the
    // dispatcher is stopped (the caller may then decide to drop).
    bool post(std::function<void()> task);

    // Drops everything queued (used when a session is torn down).
    void clear();

    std::size_t size() const;

  private:
    void loop();

    std::thread thread_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::deque<std::function<void()>> queue_;
    bool running_ = false;
};

class SerialWorker {
  public:
    static constexpr std::size_t kMaxQueue = 256;

    SerialWorker() = default;
    ~SerialWorker();

    SerialWorker(const SerialWorker&) = delete;
    SerialWorker& operator=(const SerialWorker&) = delete;

    void start();
    void stop();
    bool post(std::function<void()> task);
    std::size_t size() const;

  private:
    void loop();

    std::thread thread_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::deque<std::function<void()>> queue_;
    bool running_ = false;
};

}  // namespace km6fod
