// SPDX-License-Identifier: GPL-2.0-only
//
// IFingerprint service root.
//
// Owns the single vendor-engine proxy, the callback dispatcher, the blocking
// worker, the deadline scheduler and the touch reader, and routes engine
// notifications to whichever session the framework most recently created.

#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>

#include <aidl/android/hardware/biometrics/fingerprint/BnFingerprint.h>

#include "deadline.h"
#include "engine_message.h"
#include "executors.h"
#include "session.h"
#include "touch_reader.h"

namespace km6fod {

class FingerprintHal : public aidl::android::hardware::biometrics::fingerprint::BnFingerprint {
  public:
    FingerprintHal() = default;
    ~FingerprintHal() override;

    void start();
    void stop();

    // Engine -> HAL bridge, installed into the vendor device on first open.
    static void engineNotifyTrampoline(const EngineMessage* message);

    // ---- IFingerprint ---------------------------------------------------
    ::ndk::ScopedAStatus getSensorProps(
        std::vector<aidl::android::hardware::biometrics::fingerprint::SensorProps>* _aidl_return)
        override;
    ::ndk::ScopedAStatus createSession(
        int32_t in_sensorId, int32_t in_userId,
        const std::shared_ptr<
            aidl::android::hardware::biometrics::fingerprint::ISessionCallback>& in_cb,
        std::shared_ptr<aidl::android::hardware::biometrics::fingerprint::ISession>* _aidl_return)
        override;

  private:
    void dispatchEngineMessage(const EngineMessage& message);
    void onTouch(int x, int y, bool down);
    void onFodKey(bool down);

    static std::atomic<FingerprintHal*> g_instance;

    std::mutex session_mutex_;
    std::shared_ptr<Session> session_;
    // Engine notices can arrive after a session swap. They carry no operation
    // id, so a short mute window keeps the old operation's terminal event from
    // being delivered to the new session.
    std::chrono::steady_clock::time_point engine_mute_until_{};

    CallbackDispatcher dispatcher_;
    SerialWorker worker_;
    DeadlineScheduler timers_;
    TouchReader touch_;
    std::atomic<bool> started_{false};

};

}  // namespace km6fod
