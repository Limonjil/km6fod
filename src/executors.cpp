// SPDX-License-Identifier: GPL-3.0-only

#include "executors.h"

#include <utility>

#include "logging.h"

namespace km6fod {

// ---------------------------------------------------------------- dispatcher

CallbackDispatcher::~CallbackDispatcher() {
    stop();
}

void CallbackDispatcher::start() {
    // The lifecycle lock is the queue mutex: start()/stop() only touch the
    // thread and the running flag, both of which are published under it.
    std::unique_lock<std::mutex> lock(mutex_);
    if (running_ || thread_.joinable()) {
        return;
    }
    running_ = true;
    thread_ = std::thread(&CallbackDispatcher::loop, this);
}

void CallbackDispatcher::stop() {
    std::thread local;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
        // Queued work still runs: the loop drains before it returns.
        local = std::move(thread_);
    }
    cv_.notify_all();
    if (!local.joinable()) {
        return;
    }
    if (local.get_id() == std::this_thread::get_id()) {
        LOGW("dispatcher: stop() called from its own thread, detaching");
        local.detach();
        return;
    }
    local.join();
}

bool CallbackDispatcher::post(std::function<void()> task) {
    if (!task) {
        return false;
    }
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!running_) {
            return false;
        }
        if (queue_.size() >= kMaxQueue) {
            // Keep the newest (terminal) events, drop the oldest stale sample.
            queue_.pop_front();
            LOGW("dispatcher: queue full, dropped oldest task");
        }
        queue_.push_back(std::move(task));
    }
    cv_.notify_one();
    return true;
}

void CallbackDispatcher::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.clear();
}

std::size_t CallbackDispatcher::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

void CallbackDispatcher::loop() {
    for (;;) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] { return !running_ || !queue_.empty(); });
            if (queue_.empty()) {
                return;
            }
            task = std::move(queue_.front());
            queue_.pop_front();
        }
        // Outside the lock: a callback may enqueue further work.
        task();
    }
}

// ------------------------------------------------------------------- worker

SerialWorker::~SerialWorker() {
    stop();
}

void SerialWorker::start() {
    std::unique_lock<std::mutex> lock(mutex_);
    if (running_ || thread_.joinable()) {
        return;
    }
    running_ = true;
    thread_ = std::thread(&SerialWorker::loop, this);
}

void SerialWorker::stop() {
    std::thread local;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
        // Queued work still runs: the loop drains before it returns.
        local = std::move(thread_);
    }
    cv_.notify_all();
    if (!local.joinable()) {
        return;
    }
    if (local.get_id() == std::this_thread::get_id()) {
        LOGW("worker: stop() called from its own thread, detaching");
        local.detach();
        return;
    }
    local.join();
}

bool SerialWorker::post(std::function<void()> task) {
    if (!task) {
        return false;
    }
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!running_) {
            return false;
        }
        if (queue_.size() >= kMaxQueue) {
            queue_.pop_front();
            LOGW("worker: queue full, dropped oldest task");
        }
        queue_.push_back(std::move(task));
    }
    cv_.notify_one();
    return true;
}

std::size_t SerialWorker::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

void SerialWorker::loop() {
    for (;;) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] { return !running_ || !queue_.empty(); });
            if (queue_.empty()) {
                return;
            }
            task = std::move(queue_.front());
            queue_.pop_front();
        }
        task();
    }
}

}  // namespace km6fod
