#!/system/bin/sh
#
# Module removal: stop the supervisor and the HAL, then force the panel back to
# normal brightness. The VINTF bind mount disappears on the next boot by itself.
#
# The supervisor's wait(1) is blocked on the HAL. Killing it first is essential:
# otherwise it would restart fp_hal immediately after this script kills it.

MODDIR=${0%/*}
# One runtime directory, owned by the HAL; every script must use it (review H-32).
RUN="/data/adb/km6fod/run"
PIDFILE="$RUN/fp_hal.pid"
SUPERVISOR_LOCK="$RUN/supervisor.lock"
HBM=/sys/kernel/tran_display/lcm_hbm_state
VENDOR_HAL=/vendor/bin/hw/android.hardware.biometrics.fingerprint@2.1-service-mediatek
LOGFILE=/data/adb/km6fod.log

log() { echo "$(date '+%Y-%m-%d %H:%M:%S') $*" >> "$LOGFILE" 2>/dev/null; }

kill_pid() {
    [ -n "$1" ] || return 0
    kill "$1" 2>/dev/null
    sleep 0.2
    kill -9 "$1" 2>/dev/null
    return 0
}

reset_hbm() {
    [ -e "$HBM" ] || return 0
    local i=0
    local value=""
    while [ "$i" -lt 5 ]; do
        printf '0' > "$HBM" 2>/dev/null
        value=$(cat "$HBM" 2>/dev/null)
        case "$value" in
            *OK*) sleep 0.05 ;;
            *) return 0 ;;
        esac
        i=$((i + 1))
    done
    log "uninstall: HBM reset unverified (read '$value')"
}

# A pid stored before the reboot may now belong to an unrelated process (pid
# reuse), so identity is verified before anything is killed (F-012). The HAL is
# executed from $VENDOR_HAL, a bind mount of files/fp_hal, so its command line
# never names this module: compare the executable instead (F-005).
is_supervisor() {
    [ -n "$1" ] && [ -r "/proc/$1/cmdline" ] || return 1
    tr '\0' ' ' < "/proc/$1/cmdline" 2>/dev/null | grep -q 'km6fod/service.sh'
}

is_our_hal() {
    [ -n "$1" ] && [ -r "/proc/$1/exe" ] || return 1
    exe=$(readlink "/proc/$1/exe" 2>/dev/null)
    case "$exe" in
        "$MODDIR"/files/*|/data/local/tmp/fp_hal*) return 0 ;;
    esac
    # Started from the vendor path: ours only while that path is our mount,
    # otherwise the stock HIDL provider stands behind it.
    [ "$exe" = "$VENDOR_HAL" ] && grep -q " $VENDOR_HAL " /proc/mounts 2>/dev/null
}

# 1. Supervisor first.
supervisor=$(cat "$SUPERVISOR_LOCK/pid" 2>/dev/null)
if is_supervisor "$supervisor"; then
    log "uninstall: stopping supervisor $supervisor"
    kill_pid "$supervisor"
fi
for pid in $(pgrep -f "$MODDIR/service.sh" 2>/dev/null); do
    is_supervisor "$pid" && kill_pid "$pid"
done

# 2. HAL.
hal=$(cat "$PIDFILE" 2>/dev/null)
if is_our_hal "$hal"; then
    log "uninstall: stopping HAL $hal"
    kill_pid "$hal"
fi
for pid in $(pgrep -f 'fingerprint@2.1-service-mediatek' 2>/dev/null); do
    is_our_hal "$pid" && kill_pid "$pid"
done

# 3. Give the vendor tree back. The module's bind mounts survive until the next
#    reboot otherwise, and the stock HIDL provider stays disabled for that time.
for target in "$VENDOR_HAL" \
              /vendor/lib64/libsl_fp_impl.so \
              /vendor/etc/vintf/manifest \
              /system/product/overlay; do
    if grep -q " $target " /proc/mounts 2>/dev/null; then
        if umount "$target" 2>/dev/null; then
            log "uninstall: umounted $target"
        else
            log "uninstall: не удалось отмонтировать $target (исчезнет после ребута)"
        fi
    fi
done
if [ -e "$VENDOR_HAL" ]; then
    setprop ctl.start vendor.fps_hal 2>/dev/null
    setprop ctl.start vendor.tran_fps_hal 2>/dev/null
    log "uninstall: стоковые fingerprint-сервисы запущены заново"
fi

# 4. Panel and runtime state.
reset_hbm
rm -f "$MODDIR/run/hbm_owned" 2>/dev/null
rm -rf "$RUN" 2>/dev/null
log "uninstall: stopped and HBM reset"

