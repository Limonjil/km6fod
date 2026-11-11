// SPDX-License-Identifier: GPL-3.0-only
//
// ISession implementation.
//
// Contract rules that this file deliberately enforces (violating any of them
// either hangs the biometric scheduler or takes down system_server):
//
//   * No ISession method ever returns a service-specific error. Problems are
//     reported through onError() and the method returns ok().
//   * revokeChallenge() always answers with onChallengeRevoked().
//   * getAuthenticatorId() never fails.
//   * close() always answers with onSessionClosed().
//   * The vendor sensor is opened lazily, never on a Binder thread and never in
//     createSession().
//   * Every vendor call runs on the SerialWorker. A Binder thread only updates
//     state, creates the cancellation object and posts work.
//   * HBM is closed on cancel/close/completion. A failed authentication attempt
//     keeps the session active, but the panel window is closed after a short
//     inactivity period and re-armed by the next touch.
//
// Concurrency model:
//   generation_ is the operation identity. Every start increments it; every
//   Cancellation remembers the generation it belongs to. A late cancel from a
//   superseded operation therefore cannot kill the current one. Engine events
//   are accepted only while a matching operation is active.

#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

#include <aidl/android/hardware/biometrics/common/BnCancellationSignal.h>
#include <aidl/android/hardware/biometrics/common/OperationContext.h>
#include <aidl/android/hardware/biometrics/fingerprint/AcquiredInfo.h>
#include <aidl/android/hardware/biometrics/fingerprint/BnSession.h>
#include <aidl/android/hardware/biometrics/fingerprint/Error.h>
#include <aidl/android/hardware/biometrics/fingerprint/ISessionCallback.h>
#include <aidl/android/hardware/biometrics/fingerprint/PointerContext.h>
#include <aidl/android/hardware/keymaster/HardwareAuthToken.h>

#include "deadline.h"
#include "engine_message.h"
#include "executors.h"

namespace km6fod {

// Creates the HAL's virtual KEY_WAKEUP device ahead of the first press, because
// inputflinger opens new devices asynchronously and drops early events.
void wakeKeyInit();

class Session : public aidl::android::hardware::biometrics::fingerprint::BnSession {
  public:
    using AuthToken = aidl::android::hardware::keymaster::HardwareAuthToken;
    using CancellationSignal =
        aidl::android::hardware::biometrics::common::ICancellationSignal;
    using Context = aidl::android::hardware::biometrics::common::OperationContext;
    using FingerprintError = aidl::android::hardware::biometrics::fingerprint::Error;
    using PointerContext =
        aidl::android::hardware::biometrics::fingerprint::PointerContext;

    Session(std::shared_ptr<
                aidl::android::hardware::biometrics::fingerprint::ISessionCallback>
                callback,
            int32_t userId, CallbackDispatcher& dispatcher, SerialWorker& worker,
            DeadlineScheduler& timers);
    ~Session() override;

    // ---- called from the HAL --------------------------------------------
    // Registers a weak self-reference so queued async work and late cancels can
    // verify that the session is still alive.
    void attachSelf(const std::shared_ptr<Session>& self);

    void onEngineMessage(const EngineMessage& message);

    // FOD wake gesture (EV_KEY 0xbe or 0xb7) from the touch controller. This is the only
    // notification available while the panel is off, so it drives both the
    // display wake-up and the illumination window.
    void onFodKey(bool down);
    void onTouchEvent(int x, int y, bool down);
    bool isClosed() const { return closed_.load(std::memory_order_acquire); }
    bool hasActiveOperation() const {
        return state_.load(std::memory_order_acquire) != State::kIdle;
    }

    // Called by FingerprintHal::stop(). Marks the session dead, cancels the
    // engine operation and closes HBM. Does not emit callbacks.
    void shutdown();

    // ---- IFingerprint.ISession ------------------------------------------
    ::ndk::ScopedAStatus generateChallenge() override;
    // The worker half of generateChallenge(), after the calibration settle.
    void finishChallenge();
    ::ndk::ScopedAStatus revokeChallenge(int64_t in_challenge) override;
    ::ndk::ScopedAStatus enroll(
        const AuthToken& in_hat,
        std::shared_ptr<CancellationSignal>* _aidl_return) override;
    ::ndk::ScopedAStatus authenticate(
        int64_t in_operationId,
        std::shared_ptr<CancellationSignal>* _aidl_return) override;
    ::ndk::ScopedAStatus detectInteraction(
        std::shared_ptr<CancellationSignal>* _aidl_return) override;
    ::ndk::ScopedAStatus enumerateEnrollments() override;
    ::ndk::ScopedAStatus removeEnrollments(const std::vector<int32_t>& in_enrollmentIds) override;
    ::ndk::ScopedAStatus getAuthenticatorId() override;
    ::ndk::ScopedAStatus invalidateAuthenticatorId() override;
    ::ndk::ScopedAStatus resetLockout(const AuthToken& in_hat) override;
    ::ndk::ScopedAStatus close() override;
    ::ndk::ScopedAStatus onPointerDown(int32_t in_pointerId, int32_t in_x, int32_t in_y,
                                       float in_minor, float in_major) override;
    ::ndk::ScopedAStatus onPointerUp(int32_t in_pointerId) override;
    ::ndk::ScopedAStatus onUiReady() override;
    ::ndk::ScopedAStatus authenticateWithContext(
        int64_t in_operationId, const Context& in_context,
        std::shared_ptr<CancellationSignal>* _aidl_return) override;
    ::ndk::ScopedAStatus enrollWithContext(
        const AuthToken& in_hat, const Context& in_context,
        std::shared_ptr<CancellationSignal>* _aidl_return) override;
    ::ndk::ScopedAStatus detectInteractionWithContext(
        const Context& in_context,
        std::shared_ptr<CancellationSignal>* _aidl_return) override;
    ::ndk::ScopedAStatus onPointerDownWithContext(const PointerContext& in_context) override;
    ::ndk::ScopedAStatus onPointerUpWithContext(const PointerContext& in_context) override;
    ::ndk::ScopedAStatus onPointerCancelWithContext(const PointerContext& in_context) override;
    ::ndk::ScopedAStatus onContextChanged(const Context& in_context) override;
    ::ndk::ScopedAStatus setIgnoreDisplayTouches(bool in_shouldIgnore) override;

  private:
    enum class State { kIdle, kEnrolling, kAuthenticating, kDetecting };

    class Cancellation;

    static constexpr uint32_t kEnrollTimeoutSeconds = 60;
    static constexpr int kOperationTimeoutMs = 60000;
    // Enrollment is twenty separate captures with the finger lifted in between,
    // and AOSP itself gives the enroll client no deadline (timeoutSec = 0). A
    // 60 s budget fired in the middle of a normal enrollment and closed the
    // operation with TIMEOUT, which the user sees as the sensor giving up.
    static constexpr int kEnrollOperationTimeoutMs = 180000;
    static constexpr int kCancelEscalationMs = 3000;
    static constexpr int kRearmThrottleMs = 500;
    static constexpr int kEnumerateQuietMs = 400;
    static constexpr int kEnumerateMaxMs = 2500;

    // How long to wait after the first set_active_group() before pre_enroll().
    // The vendor engine runs optic calibration (TA command 0x40) as a side effect
    // of the group switch, and that command replaces the TA's enroll challenge.
    // Measured on hardware: calibration completes ~200 ms after the switch, but
    // the margin is kept generous because a failed enroll costs a full retry.
    // times, on the timer queue instead of on the serial worker (review H-12).
    static constexpr int kCalibrationSettleMs = 2500;

    // Opening the HBM window right after the display wakes is racy: the panel
    // driver ignores the write while it still considers the display idle, the
    // readback stays 0xFA and the capture would be aborted. Retrying briefly
    // covers the wake-up ramp instead of failing an authentication attempt.
    // One attempt plus one deferred retry covers the ramp without ever sleeping
    // on the callback dispatcher (review H-12/H-33).
    static constexpr int kHbmArmRetryMs = 80;

    // How long to wait for the fingerprint overlay after waking a dark screen,
    // and the polling step. SystemUI needs roughly 200-400 ms to draw it; waiting
    // here is what lets a single press work rather than requiring a second touch.
    static constexpr int kUiReadyWaitMs = 380;
    static constexpr int kUiReadyPollMs = 30;

    // ---- AIDL entry points shared by the legacy and context variants ----
    ::ndk::ScopedAStatus startEnroll(const AuthToken& hat,
                                     std::shared_ptr<CancellationSignal>* out);
    ::ndk::ScopedAStatus startAuthenticate(int64_t operation_id,
                                           std::shared_ptr<CancellationSignal>* out);

    // ---- operation lifecycle --------------------------------------------
    bool isCurrent(uint64_t generation, State state) const;
    uint64_t beginOperation(State state);
    bool finishOperation(uint64_t generation, bool close_hbm);
    void failOperation(uint64_t generation, FingerprintError error);
    void postTerminalError(FingerprintError error, uint64_t epoch);
    void cancelOperation(uint64_t generation);
    void timeoutOperation(uint64_t generation);

    // ---- worker-side actions --------------------------------------------
    void runEnroll(uint64_t generation, AuthToken hat);
    void runAuthenticate(uint64_t generation, int64_t operation_id);
    bool armCapture(uint64_t generation, State expected);

    // Opens the panel illumination window on demand (first touch of an
    // authentication). Returns false when the panel refused to become bright.
    bool armIllumination(bool allow_wake);

    // The single requestOn() attempt and the guards around it; no retry here.
    bool tryArmIllumination();
    // Puts that one retry on the timer queue (review H-12/H-33).
    void scheduleIlluminationRetry();

    // Schedules armIllumination() on the dispatcher. The touch reader must never
    // block on the panel: it is the only thread reading the finger, and a stalled
    // reader means no samples for the sensor and a release that is never seen.
    void requestIllumination(bool allow_wake);
    bool illuminationQuarantined() const;
    // Tells the vendor the glass is empty, on the serial worker (review H-07/H-03).
    void releaseFingerOnEngine();
    // Clears the finger flag and reconciles the vendor. True if it was set.
    bool clearFingerState();
    // Every dispatcher post is checked: a silently dropped callback leaves the
    // framework waiting until its own timeout (review H-06).
    bool postToUi(std::function<void()> task);
    // Vendor work goes here, never onto a Binder, timer or reader thread
    // (review H-02/H-03/H-04).
    bool postToWorker(std::function<void()> action);
    // The FOD press body: vendor work plus a bounded wait for the overlay.
    void armFodPress();
    // The part of the press that needs the display up, after the timer wait.
    void continueFodPress();
    // Ends a deferred gesture release once the hold window expired (timer-driven).
    void finishGestureRelease(uint64_t token);
    // One step of the "wait for the framework overlay" poll (timer-driven).
    void overlayTick(int waited);

    bool ensureUserGroup();
    void requestRearm(uint64_t generation);
    void startEnumerate(std::vector<int32_t> verify_removal = {});
    void runEnumerate(uint64_t generation);
    // Timer-driven flush of the enumeration buffer (review H-12).
    void enumerateSettle(uint64_t generation);
    void finishEnumerate(uint64_t generation, std::vector<int32_t> ids,
                         bool authoritative);

    // ---- engine message handlers ----------------------------------------
    void handleEnumerateMessage(const EngineMessage& message);

    // ---- callback helpers ----------------------------------------------
    bool checkCallback(::ndk::ScopedAStatus status, const char* what);
    void handleDeadCallback();
    bool emitChallengeGenerated(int64_t challenge);
    bool emitError(FingerprintError error);
    bool emitAcquired(int32_t engine_code);
    bool emitEnrollmentProgress(int32_t id, int32_t remaining);
    bool emitAuthenticationSucceeded(int32_t id, const AuthToken& token);
    bool emitAuthenticationFailed();
    bool emitInteractionDetected();
    bool emitEnrollmentsEnumerated(const std::vector<int32_t>& ids);
    bool emitEnrollmentsRemoved(const std::vector<int32_t>& ids);
    bool emitAuthenticatorId(int64_t id, bool invalidated);
    bool emitSessionClosed();

    std::shared_ptr<
        aidl::android::hardware::biometrics::fingerprint::ISessionCallback>
        callback_;
    CallbackDispatcher& dispatcher_;
    SerialWorker& worker_;
    DeadlineScheduler& timers_;
    const int32_t user_id_;

    std::atomic<State> state_{State::kIdle};
    std::atomic<bool> closed_{false};
    std::atomic<bool> ignore_touches_{false};
    std::atomic<int32_t> display_state_{0};

    // Operation identity. Written under op_mutex_, read atomically elsewhere.
    std::atomic<uint64_t> generation_{0};
    std::atomic<uint64_t> cancel_epoch_{0};
    std::mutex op_mutex_;
    uint64_t timeout_id_ = 0;

    std::atomic<bool> rearm_pending_{false};
    std::atomic<int64_t> last_rearm_ms_{0};

    // Tracks whether the engine has been told the finger is down, so the
    // corresponding "up" is always delivered exactly once.
    std::atomic<bool> finger_down_{false};

    // Set when the engine reports a GOOD frame (acquired code 0). A non-match is
    // reported to the framework only after such a frame: partial touches also
    // produce "no match", and counting them as failed attempts locked the sensor.
    std::atomic<bool> frame_good_{false};

    std::atomic<int64_t> last_touch_ms_{0};

    // When the last press started on a dark panel and woke it: the overlay icon is
    // drawn by SystemUI ~1.4 s after the wake, far outside the capture budget, so
    // such a press is captured as soon as the panel is actually lit instead of
    // waiting for the icon.
    std::atomic<int64_t> dark_wake_ms_{0};

    // Authoritative template id: from the last enumerate() when exactly one
    // template exists, otherwise from the last enrollment step. Always a value the
    // framework accepts (never negative); -1 means "nothing known in this boot".
    std::atomic<int32_t> single_template_id_{-1};

    // Authentication defers the illumination window until a finger actually
    // arrives, so the panel does not flash every time the keyguard appears.
    std::atomic<bool> illumination_pending_{false};

    // Set by onUiReady(): the framework only calls it when the fingerprint
    // overlay is actually on screen, and it re-asserts it periodically while the
    // overlay stays up. A plain flag is not enough - the framework never tells us
    // when the overlay goes away (the PIN pad, the emergency dialer), so the flag
    // would stay set and the panel would light up over a keypad. A short validity
    // window expires on its own once those callbacks stop.
    static constexpr int64_t kUiReadyWindowMs = 2500;
    std::atomic<int64_t> ui_ready_until_ms_{0};

    // How long a finger is assumed to stay on the glass after a wake gesture.
    // The touch controller sends one momentary event per gesture, so without
    // this hold a finger that never moved would be treated as lifted long before
    // the display is back and the sensor can capture.
    static constexpr int64_t kGestureHoldMs = 1500;
    std::atomic<uint64_t> gesture_token_{0};

    // A touch release is only believed once the glass has been quiet this long;
    // the digitizer reports a release for every frame the finger shifts, and
    // acting on each of them aborted every capture before it could finish.
    // Two different deadlines on purpose. The optic state must be released
    // quickly, otherwise the engine keeps waiting for a frame from a finger that
    // is gone. The light must not be, or an uneven stream of samples closes and
    // re-opens the window over and over, which looks like blinking.
    // One arm in flight at a time; a second request while one runs is dropped
    // because requestOn() is idempotent for an open window anyway.
    std::atomic<bool> arm_in_flight_{false};
    // One deferred illumination retry at a time (review H-12).
    std::atomic<bool> illumination_retry_{false};
    // One overlay wait at a time: the branch that handles touches without an
    // overlay fires for every sample, but only one of them may wait.
    std::atomic<bool> overlay_wait_active_{false};
    // When the finger was last seen going up, so an engine result that arrives
    // long after the glass is empty can be recognised as stale.
    std::atomic<int64_t> finger_up_ms_{0};
    // Set on a successful authentication: no illumination for a moment after it,
    // otherwise the new operation the framework starts re-lights the sensor.
    std::atomic<int64_t> last_success_ms_{0};
    // Anchor for the staleness check. Without it a session that never saw a plain
    // touch kept up_at == 0, which disabled the check entirely (review H-09).
    std::atomic<int64_t> op_started_ms_{0};
    std::atomic<int32_t> last_touch_x_{0};
    std::atomic<int32_t> last_touch_y_{0};

    // A match older than this after the finger left is not applied. 1.2 s was too
    // tight: the vendor engine on a cold press routinely answers 1.5-2 s after the
    // release, and a good match was being dropped as "stale" - the first press
    // unlocked and the next ones silently did not.
    static constexpr int64_t kStaleResultMs = 2600;

    static constexpr int64_t kOpticReleaseMs = 120;
    // How long the HBM window survives a finger-up. At 350 ms the next touch of a
    // quick double tap re-armed the panel while it was still ramping down, and the
    // repeated on/off showed as a bright moving band. The window now outlives a
    // normal second touch and is closed by the same deferred path.
    static constexpr int64_t kWindowHoldMs = 250;
    std::atomic<uint64_t> release_token_{0};

    bool uiReadyValid() const;
    void markUiReady();
    void clearUiReady();

    // Enrollment progress. The vendor engine reports its "remaining" counter
    // verbatim from the value we seed, and never decrements it itself, so the
    // framework would sit at 100% forever. Progress is therefore counted here:
    // one step per accepted enroll notification, clamped to 1 until the engine
    // finally reports completion, so a burst of frames cannot finish the
    // enrollment early.
    std::atomic<int32_t> enroll_steps_total_{0};
    std::atomic<int32_t> enroll_steps_done_{0};

    // set_active_group() must run at most once per session, and only before
    // pre_enroll(). The vendor engine resets its pending challenge whenever the
    // active group is (re)applied, so calling it again between pre_enroll() and
    // enroll() makes silfp_impl_verify_enroll_challenge() fail and the engine
    // reports HW_UNAVAILABLE one millisecond after the enroll command.
    std::atomic<bool> group_applied_{false};

    // Set when this session actually performed set_active_group(). The engine
    // starts optic calibration as a side effect of that call, and calibration
    // issues a TA command that replaces the stored enroll challenge. pre_enroll()
    // must therefore wait for it to settle, or enroll() dies with
    // INVALID_CHALLENGE (-1004).
    std::atomic<bool> group_just_applied_{false};

    // The active group is a property of the vendor engine, not of a session, so
    // it is applied once per process instead of once per session; otherwise the
    // calibration delay would be paid on every operation.
    static std::atomic<int32_t> applied_user_id_;

    // Enumeration is accumulated and flushed once, because AIDL delivers the
    // complete template list in a single callback. The quiet window absorbs
    // notices that the engine emits asynchronously after enumerate() returns;
    // it is driven by the timer queue so the serial worker is never parked for
    // it (review H-12).
    mutable std::mutex enum_mutex_;
    std::vector<int32_t> enumerated_;
    // Non-empty when the enumeration runs to verify a removal: the ids are
    // reported as removed only if they are absent from the result (F-004).
    std::vector<int32_t> enum_remove_pending_;
    bool enum_active_ = false;
    bool enum_error_ = false;
    int64_t enum_started_ms_ = 0;
    int64_t enum_last_notice_ms_ = 0;
    uint64_t enum_generation_ = 0;
    std::weak_ptr<Session> self_;
};

}  // namespace km6fod
