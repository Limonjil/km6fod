#!/system/bin/sh
#
# Hold mode: the display stays interactive but dark while the device is locked,
# so the touch controller stays in TRAN_ACTIVE_MODE and a resting finger produces
# touches (DEBUG.md F17/F20/F25).
#
# How it is done - and why not otherwise:
#   * `input keyevent` and `cmd power wakeup` do not work from a Magisk child;
#   * `settings`/`svc power stayon` cannot stop an explicit power-key sleep;
#   * a PowerManager display wakelock CAN: it wakes the panel (ACQUIRE_CAUSES_WAKEUP)
#     and keeps it awake. It is acquired by a tiny Java class run through
#     app_process (files/hold.dex), the same mechanism the platform's own shell
#     tools use. Killing that process releases the lock.
#   * brightness is changed through the framework (`cmd display set-brightness`),
#     never through the panel node, so the two can never be desynced.
#
# Exit: $RUN/hold_off (the HAL creates it on a touch outside the sensor zone or on
# a successful fingerprint unlock), or the safety timeout.

MODDIR=/data/adb/modules/km6fod
# The HAL owns /data/adb/km6fod/run (lock, hold_off, templates marker). Both sides
# must agree on it: writing hold_off to the module directory meant the HAL's signal
# went into a directory the helper never read (review H-32).
RUN="/data/adb/km6fod/run"
OFF_REQ="$RUN/hold_off"
LOCK_PID="$RUN/hold.lock.pid"
SAVED="$RUN/hold.saved"
LOGFILE=/data/adb/km6fod.log
MAX_ACTIVE_S=1800
COOLDOWN_S=5

STATE=idle
elapsed=0
cooldown=0

log() { echo "$(date '+%Y-%m-%d %H:%M:%S') hold.sh: $*" >> "$LOGFILE" 2>/dev/null; }
num() { case "$1" in ''|*[!0-9]*) echo "$2" ;; *) echo "$1" ;; esac; }
brightness() { num "$(cat /sys/class/leds/lcd-backlight/brightness 2>/dev/null | tr -d ' ')" -1; }
setting() { num "$(settings get system screen_brightness 2>/dev/null | tr -d ' ')" 45; }

keyguard_up() { dumpsys window 2>/dev/null | grep -q "mDreamingLockscreen=true"; }

# "The panel is off" comes from the driver through the HAL: 1 = off, 2 = on. The
# framework's DisplayState cannot answer this - LOCKSCREEN and NO_UI describe the
# UI, not the power state - and building entry on it darkened the unlocked home
# screen (DEBUG.md F29). The dumpsys probe stays as the fallback for the window
# between boot and the first panel transition.
screen_off() {
    dumpsys window policy 2>/dev/null | grep -q "screenState=SCREEN_STATE_OFF"
}

enter() {
    b=$(setting)
    # Never remember a bottom-of-range value: a leftover "1" restored the panel at
    # the dimmest step, which looks exactly like a light that never went out.
    [ "$b" -lt 20 ] && b=45
    echo "$b" > "$SAVED" 2>/dev/null
    setsid sh -c "CLASSPATH=$MODDIR/files/hold.dex exec app_process /system/bin HoldWakelock acquire" \
        </dev/null >> "$LOGFILE" 2>&1 &
    echo $! > "$LOCK_PID" 2>/dev/null
    # True darkness, and it is reversible: while the display is interactive the
    # panel node accepts 0 and any later value (measured 723 -> 0 -> 300 -> 723).
    # The display must stay interactive for writes to land, which is exactly what
    # the wakelock above guarantees.
    # No fixed sleep here: the write is repeated instead. The driver ignores it while
    # the panel is still asleep, so the second write is the one that lands once the
    # wakelock has brought the display up. A sleep cost a full second of every lock.
    echo 0 > /sys/class/leds/lcd-backlight/brightness 2>/dev/null
    sleep 0.4
    echo 0 > /sys/class/leds/lcd-backlight/brightness 2>/dev/null
    rm -f "$OFF_REQ" 2>/dev/null
    STATE=active
    elapsed=0
    entered_at=$(date +%s)
    seen_on=0
    off_polls=0
    off_stable=0
    log "ENTER (saved brightness $b, lock pid $(cat "$LOCK_PID" 2>/dev/null))"
}

leave() {
    reason="$1"
    b=$(num "$(cat "$SAVED" 2>/dev/null)" 45)
    [ "$b" -lt 20 ] && b=45
    # Brightness first, wakelock second: once the display sleeps the driver
    # ignores the node and the screen stays black (that was the bug).
    echo 723 > /sys/class/leds/lcd-backlight/brightness 2>/dev/null
    settings put system screen_brightness "$b" 2>/dev/null
    [ -f "$LOCK_PID" ] && kill -9 "$(cat "$LOCK_PID" 2>/dev/null)" 2>/dev/null
    rm -f "$LOCK_PID" 2>/dev/null
    rm -f "$SAVED" "$OFF_REQ" 2>/dev/null
    STATE=idle
    cooldown=$COOLDOWN_S
    log "LEAVE ($reason): brightness $b"
}

log "started (pid $$)"
off_stable=0

# One instance only. service.sh may run more than once (boot, hot restart, manual
# start), and several copies fought each other: three ENTER lines in a row, one
# darkening the panel while another restored it (DEBUG.md F28).
if ! mkdir "$RUN/hold.instance" 2>/dev/null; then
    other=$(cat "$RUN/hold.instance/pid" 2>/dev/null)
    if [ -n "$other" ] && kill -0 "$other" 2>/dev/null; then
        exit 0
    fi
    rm -rf "$RUN/hold.instance" 2>/dev/null
    mkdir "$RUN/hold.instance" 2>/dev/null || exit 0
fi
echo $$ > "$RUN/hold.instance/pid" 2>/dev/null

while true; do
    sleep 0.3
    if [ "$STATE" = "idle" ]; then
        if [ "$cooldown" -gt 0 ]; then
            cooldown=$((cooldown - 1))
            continue
        fi
        # Enter only on a real screen-off with the keyguard up: anything else and we
        # would fight the user (that is what dimmed an unlocked phone before).
        # The panel node keeps its last value while the display sleeps (measured:
        # 723 with the screen off), so the framework state is the only reliable
        # "dark" signal here. screen_off already means the display is not visible.
        if ! screen_off; then
            off_stable=0
            # Nothing to do while the screen is up: poll slowly, dumpsys is not cheap.
            sleep 0.7
            continue
        fi
        # Stable for ~1 s first: the framework reports NO_UI briefly during its own
        # transitions, and entering on one of those darkened the home screen right
        # after an unlock.
        off_stable=$((off_stable + 1))
        [ "$off_stable" -lt 3 ] && continue
        [ "$(setting)" -gt 0 ] || continue
        # And the keyguard must really be up. Checked once per attempt, not per poll:
        # this is the gate that kept the mode off an unlocked phone.
        if ! keyguard_up; then
            cooldown=10
            continue
        fi
        off_stable=0
        enter
        continue
    fi

    elapsed=$((elapsed + 1))
    if [ -f "$OFF_REQ" ]; then
        leave "user interaction"
        continue
    fi
    if [ "$elapsed" -ge "$MAX_ACTIVE_S" ]; then
        leave "safety timeout"
        continue
    fi

    # The user pressed power while we hold the display awake. The framework reads
    # that as "sleep" (the device was already awake), so the panel would stay dark
    # and the first press would do nothing. Wake it ourselves: one press must land
    # on the lockscreen (target behaviour, item 3).
    # Only a sleep that we actually witnessed counts as "the user pressed power":
    # right after our own wake the framework still reports the old state for a
    # moment, and treating that as a press made the helper leave instantly.
    if screen_off; then
        off_polls=$((off_polls + 1))
        # Ignore everything for the first five seconds: our own wakelock makes the
        # framework flip its state, and reading those flips as user input turned the
        # helper into a one-second ENTER/LEAVE loop that broke the lockscreen.
        if [ "$seen_on" = "1" ] && [ "$off_polls" -ge 3 ] &&
           [ $(( $(date +%s) - entered_at )) -ge 5 ]; then
            leave "user pressed power"
            # A single delayed injection: the earlier loop of three fought the sleep
            # animation and left the lockscreen unreachable.
            ( sleep 1; input keyevent 224 >/dev/null 2>&1 ) &
            continue
        fi
    else
        seen_on=1
        off_polls=0
    fi
    if [ "$(brightness)" != "0" ]; then
        # The framework writes its own brightness when the display comes up. Our own
        # wake does that within the first seconds and must be absorbed, otherwise the
        # helper left one second after entering, over and over. A later write is the
        # user pressing power: they asked for the lockscreen, so leave it alone
        # (target behaviour, item 3).
        if [ $(( $(date +%s) - entered_at )) -lt 3 ]; then
            echo 0 > /sys/class/leds/lcd-backlight/brightness 2>/dev/null
        else
            leave "user woke the display"
        fi
    fi
done
