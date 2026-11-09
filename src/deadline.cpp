// SPDX-License-Identifier: GPL-3.0-only

#include "deadline.h"

#include <algorithm>
#include <utility>

#include "logging.h"

namespace km6fod {

DeadlineScheduler::~DeadlineScheduler() {
    stop();
}

void DeadlineScheduler::start() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (running_ || thread_.joinable()) {
        return;
    }
    running_ = true;
    thread_ = std::thread(&DeadlineScheduler::loop, this);
}

void DeadlineScheduler::stop() {
    std::thread thread;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
        entries_.clear();
        thread = std::move(thread_);
    }
    cv_.notify_all();
    if (!thread.joinable()) {
        return;
    }
    if (thread.get_id() == std::this_thread::get_id()) {
        // A task must never deadlock the process by joining its own scheduler.
        thread.detach();
        return;
    }
    thread.join();
}

uint64_t DeadlineScheduler::scheduleAfter(std::chrono::milliseconds delay, Task task) {
    if (!task) {
        return 0;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!running_) {
        return 0;
    }
    if (entries_.size() >= kMaxPending) {
        LOGW("timers: queue full (%zu), dropping deadline", entries_.size());
        return 0;
    }
    const uint64_t id = next_id_++;
    entries_.push_back(Entry{Clock::now() + delay, id, std::move(task)});
    cv_.notify_all();
    return id;
}

void DeadlineScheduler::cancel(uint64_t id) {
    if (id == 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = entries_.begin(); it != entries_.end(); ++it) {
        if (it->id == id) {
            entries_.erase(it);
            return;
        }
    }
}

std::size_t DeadlineScheduler::pending() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return entries_.size();
}

void DeadlineScheduler::loop() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (running_) {
        if (entries_.empty()) {
            cv_.wait(lock);
            continue;
        }

        auto next = std::min_element(
            entries_.begin(), entries_.end(),
            [](const Entry& a, const Entry& b) { return a.when < b.when; });
        if (next == entries_.end()) {
            continue;
        }

        if (cv_.wait_until(lock, next->when) != std::cv_status::timeout) {
            continue;
        }

        const auto now = Clock::now();
        std::vector<Task> due;
        for (auto it = entries_.begin(); it != entries_.end();) {
            if (it->when <= now) {
                due.push_back(std::move(it->task));
                it = entries_.erase(it);
            } else {
                ++it;
            }
        }

        // Run outside the lock: a task may schedule or cancel further deadlines.
        lock.unlock();
        for (auto& task : due) {
            if (task) {
                task();
            }
        }
        lock.lock();
    }
}

}  // namespace km6fod
