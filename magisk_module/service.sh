#!/system/bin/sh
#
# late_start service for the KM6 FOD module.
#
# Starts the HAL and keeps it alive. The binary is executed from
# /vendor/bin/hw/... (bind-mounted there by post-fs-data.sh) because only a
# process started from a vendor path gets the vendor linker namespace, which is
# required for dlopen of /vendor/lib64/hw/fingerprint.silead.default.so.
#
# The vendor HIDL init service is stopped, so only one process ever owns the
# sensor and only one AIDL instance is ever registered.

MODDIR=${0%/*}
LOGFILE=/data/adb/km6fod.log
VENDOR_HAL=/vendor/bin/hw/android.hardware.biometrics.fingerprint@2.1-service-mediatek
# The HAL owns /data/adb/km6fod/run (lock, hold_off, templates marker). Both sides
# must agree on it: writing hold_off to the module directory meant the HAL's signal
# went into a directory the helper never read (review H-32).
RUN="/data/adb/km6fod/run"
PIDFILE="$RUN/fp_hal.pid"
SUPERVISOR_LOCK="$RUN/supervisor.lock"

# The HAL must run from the vendor path: only a process started there gets the
# vendor linker namespace, which dlopen of the vendor engine requires. If the
# bind mount from post-fs-data.sh is missing, starting $VENDOR_HAL would run the
# STOCK HIDL HAL instead - the exact process this module disables. Refuse.
HAL="$VENDOR_HAL"
if ! grep -q " $VENDOR_HAL " /proc/mounts 2>/dev/null; then
    log "FATAL: $VENDOR_HAL не смонтирован на наш бинарь; старт отклонён (иначе запустится стоковый HIDL HAL)"
    stop vendor.tran_fps_hal 2>/dev/null
    exit 1
fi
log() { echo "$(date '+%Y-%m-%d %H:%M:%S') $*" >> "$LOGFILE" 2>/dev/null; }

log "=== km6fod: service.sh ==="

mkdir -p "$RUN" 2>/dev/null

# ------------------------------------------------------- single supervisor
# The pid alone is not proof: after a reboot /data keeps the lock directory and
# the stored pid may now belong to an unrelated process. Verify that the process
# is actually our script before trusting it.
is_ours() {
    [ -n "$1" ] && [ -r "/proc/$1/cmdline" ] || return 1
    tr '\0' ' ' < "/proc/$1/cmdline" 2>/dev/null | grep -q 'km6fod/service.sh'
}

if ! mkdir "$SUPERVISOR_LOCK" 2>/dev/null; then
    old=$(cat "$SUPERVISOR_LOCK/pid" 2>/dev/null)
    if is_ours "$old"; then
        log "supervisor already running (pid $old), exiting"
        exit 0
    fi
    log "clearing stale supervisor lock (pid '${old:-none}')"
    rm -rf "$SUPERVISOR_LOCK" 2>/dev/null
    mkdir "$SUPERVISOR_LOCK" 2>/dev/null || exit 1
fi
echo $$ > "$SUPERVISOR_LOCK/pid"
trap 'rm -rf "$SUPERVISOR_LOCK" 2>/dev/null' EXIT INT TERM

# --------------------------------------------------- free the sensor
# The vendor HIDL provider opens /dev/silead_fp exclusively, and if it registers
# with hwservicemanager the framework builds a HIDL fingerprint sensor and hangs
# the biometric scheduler. It is already stopped in post-fs-data; this is belt
# and braces for a manual restart.
stop vendor.tran_fps_hal 2>/dev/null
stop vendor.fps_hal 2>/dev/null
sleep 1

# --------------------------------------------------- FOD wake gesture (screen off)
# The touch controller reports a finger on the sensor zone even with the panel
# off, but the driver drops it until the gesture is enabled. On stock the vendor
# framework asks the touch HAL to do it; on a GSI nobody does. The helper talks
# to vendor.hardware.trantp.ITrantp/default and calls opration(3, "fp1"), which
# writes "fp1" to /proc/gesture_state exactly like the stock framework does.
# After that the controller emits EV_KEY 0xbe on /dev/input/event5 for every press.
enable_fod_gesture() {
    # Two independent switches, both required:
    #   1. `/proc/gesture_state` via the vendor service - puts the controller into
    #      gesture mode, so it keeps scanning while the panel is off.
    #   2. `/proc/gesture_function` - sets the per-gesture enable bit inside the
    #      driver's `gesture_maps` table. Without it the driver recognises the
    #      touch, translates it to 0xbe and then discards it with
    #      "Sorry, this gesture has been disabled" (GestureHandle).
    if [ -x "$MODDIR/files/trantp_probe" ]; then
        if "$MODDIR/files/trantp_probe" 2 3 fp1 >/dev/null 2>&1; then
            log "FOD wake gesture enabled (ITrantp opration 3)"
        else
            log "FOD wake gesture NOT enabled (ITrantp)"
        fi
    fi
    if [ -w /proc/gesture_function ]; then
        if echo fp1 > /proc/gesture_function 2>/dev/null; then
            log "FOD wake gesture enabled (gesture_function fp1)"
        else
            log "FOD wake gesture NOT enabled (gesture_function)"
        fi
    fi
}

# The display wake is injected by the HAL itself through /dev/uinput: that is
# the only path that works here. `cmd input` returns "Failed transaction" for
# a child of the HAL, and a helper started from this script runs in the Magisk
# domain, where the same command blocks instead of waking anything.

# --------------------------------------------------------------- hold mode
# Recover the panel if a previous run died while hold mode had it at zero: the
# write only lands on an interactive display, which is the state we start in.
echo 723 > /sys/class/leds/lcd-backlight/brightness 2>/dev/null
settings put system screen_brightness 45 2>/dev/null

# Keeps the display interactive with zero backlight while locked (one process).
HOLD_PID="$RUN/hold.pid"
if [ -f "$HOLD_PID" ] && kill -0 "$(cat "$HOLD_PID" 2>/dev/null)" 2>/dev/null; then
    :
else
    if [ -x "$MODDIR/hold.sh" ]; then
        nohup sh "$MODDIR/hold.sh" >> "$LOGFILE" 2>&1 &
        echo $! > "$HOLD_PID"
        log "hold: helper started (pid $!)"
    fi
fi

# ------------------------------------------------------------ supervision
# A pid file only tells us a process exists, not that it is our HAL. After a
# reboot the stored pid may belong to anything at all, and trusting it would
# leave the sensor permanently down. Verify the command line.
is_hal_alive() {
    [ -n "$1" ] && [ -r "/proc/$1/cmdline" ] || return 1
    tr '\0' ' ' < "/proc/$1/cmdline" 2>/dev/null |
        grep -qE 'fingerprint@2\.1-service-mediatek|km6fod/files/fp_hal'
}

delay=2
while true; do
    pid=$(cat "$PIDFILE" 2>/dev/null)
    if is_hal_alive "$pid"; then
        sleep 10
        continue
    fi

    # Never leave the panel in HBM because the previous instance died.
    echo 0 > /sys/kernel/tran_display/lcm_hbm_state 2>/dev/null

    log "starting $HAL"
    "$HAL" >> "$LOGFILE" 2>&1 &
    pid=$!
    echo "$pid" > "$PIDFILE" 2>/dev/null
    log "HAL pid $pid"

    started=$(date +%s)
    sleep 5

    if kill -0 "$pid" 2>/dev/null; then
        # It survived the first seconds: reset the backoff and (re)assert the
        # wake gesture, which the driver does not keep across a restart.
        delay=2
        enable_fod_gesture
    else
        # Report how it ended, not just that it ended (review F-011): without the
        # wait status every crash looks identical in the log.
        wait "$pid" 2>/dev/null
        status=$?
        ended=$(date +%s)
        ran=$((ended - started))
        log "HAL exited after ${ran}s status=$status"
        [ "$ran" -ge 30 ] && delay=2
        [ "$ran" -lt 5 ] && delay=30
        sleep "$delay"
        delay=$((delay * 2))
        [ "$delay" -gt 30 ] && delay=30
    fi
done
