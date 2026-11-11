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
# One runtime directory, owned by the HAL; every script must use it (review H-32).
RUN="/data/adb/km6fod/run"
PIDFILE="$RUN/fp_hal.pid"
SUPERVISOR_LOCK="$RUN/supervisor.lock"

# The HAL must run from the vendor path: only a process started there gets the
# vendor linker namespace, which dlopen of the vendor engine requires. If the
# bind mount from post-fs-data.sh is missing, starting $VENDOR_HAL would run the
# STOCK HIDL HAL instead - the exact process this module disables. Refuse.
log() { echo "$(date '+%Y-%m-%d %H:%M:%S') $*" >> "$LOGFILE" 2>/dev/null; }
HAL="$VENDOR_HAL"
# Leave a working provider behind instead of exiting into a device where nobody
# owns the sensor (F-007).
restore_stock() {
    setprop ctl.start vendor.tran_fps_hal 2>/dev/null
    setprop ctl.start vendor.fps_hal 2>/dev/null
    exit 1
}

# post-fs-data.sh writes $RUN/ready only after every mount and the RRO gate
# passed; its absence means the stock provider was never stopped and this HAL
# must not register either.
if [ ! -f "$RUN/ready" ]; then
    log "FATAL: post-fs-data не подтвердил подмену (нет $RUN/ready); старт отклонён"
    restore_stock
fi

if ! grep -q " $VENDOR_HAL " /proc/mounts 2>/dev/null; then
    log "FATAL: $VENDOR_HAL не смонтирован на наш бинарь; старт отклонён"
    restore_stock
fi

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

# Recover the panel if a previous run left the backlight at zero: the write only
# lands on an interactive display, which is the state we start in.

# ------------------------------------------------------------ supervision
# A pid file only tells us a process exists, not that it is our HAL. After a
# reboot the stored pid may belong to anything at all, and trusting it would
# leave the sensor permanently down. Verify what the process actually runs.
# The pid file survives a reboot and may then name the stock HIDL provider on
# the same path or an unrelated process: identity is the executed file (F-005),
# not the command line, which is identical for both binaries behind $VENDOR_HAL.
is_hal_alive() {
    [ -n "$1" ] && [ -r "/proc/$1/exe" ] || return 1
    exe=$(readlink "/proc/$1/exe" 2>/dev/null)
    [ "$exe" = "$HAL" ] || [ "$exe" = "$MODDIR/files/fp_hal" ]
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
        delay=2
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
