#!/usr/bin/env bash
#
# Deploy helper for the KM6 FOD module.
#
#   ./install.sh --test       build, push and run the HAL now, without rebooting
#   ./install.sh --install    build and install the module for persistence
#   ./install.sh --uninstall  stop the HAL/supervisor and remove the module
#   ./install.sh --status     report what the device currently sees
#   ./install.sh --diag       dump HBM, touch, AVC and logcat diagnostics
#   ./install.sh --zip        build the flashable module only
#
# Safety: nothing here erases a partition or writes a raw block device. Every
# step is reversible; --uninstall plus a reboot returns the device to whatever
# state it was in before.
#
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
MODULE="$ROOT/magisk_module"
TARGET=/data/adb/modules/km6fod
ADB="${ADB:-adb}"
VERSION="$(sed -n 's/^version=//p' "$MODULE/module.prop")"
ZIP="$ROOT/dist/km6fod-${VERSION:-dev}.zip"

# Android paths must survive Git Bash path translation.
export MSYS_NO_PATHCONV=1

say() { printf '\033[1m%s\033[0m\n' "$*"; }

require_device() {
    if ! "$ADB" get-state >/dev/null 2>&1; then
        echo "error: no adb device" >&2
        exit 1
    fi
    if [ "$("$ADB" shell id -u | tr -d '\r')" != "0" ]; then
        echo "error: root required (run 'adb root')" >&2
        exit 1
    fi
}

build_zip() {
    say "== building module"
    bash "$ROOT/build_module.sh"
}

stage_binary() {
    say "== building HAL"
    bash "$ROOT/build.sh"
    mkdir -p "$MODULE/files"
    cp -f "$ROOT/build/fp_hal" "$MODULE/files/fp_hal"
    chmod 0755 "$MODULE/files/fp_hal"
}

push_tree() {
    say "== pushing module tree to $TARGET"
    "$ADB" shell "rm -rf $TARGET && mkdir -p $TARGET"
    "$ADB" push "$MODULE/." "$TARGET/" >/dev/null
    "$ADB" shell "chmod 0755 $TARGET/post-fs-data.sh $TARGET/service.sh $TARGET/uninstall.sh 2>/dev/null; \
                  chmod 0755 $TARGET/files/fp_hal 2>/dev/null; \
                  mkdir -p $TARGET/run; \
                  rm -f $TARGET/disable $TARGET/remove $TARGET/run/*.pid"
}

# Stop this project's previous instance and the stock vendor HIDL provider.
# This script deliberately knows nothing about other Magisk modules: if another
# module also publishes IFingerprint/default, its fingerprint part must be
# disabled by its owner before this one is activated.
stop_conflicts() {
    say "== stopping this project's previous HAL/supervisor and vendor HIDL"
    "$ADB" shell 'sh -s' <<'EOF'
for pat in "/data/adb/modules/km6fod/service.sh"; do
    for pid in $(pgrep -f "$pat" 2>/dev/null); do kill "$pid" 2>/dev/null; done
done
for pat in "/data/adb/modules/km6fod/files/fp_hal" "/data/local/tmp/fp_hal_new"; do
    for pid in $(pgrep -f "$pat" 2>/dev/null); do kill "$pid" 2>/dev/null; done
done
stop vendor.tran_fps_hal 2>/dev/null
stop vendor.fps_hal 2>/dev/null
true
EOF
    sleep 1
}

reset_hbm() {
    "$ADB" shell "if [ -e /sys/kernel/tran_display/lcm_hbm_state ]; then \
                    printf '0' > /sys/kernel/tran_display/lcm_hbm_state 2>/dev/null; \
                    cat /sys/kernel/tran_display/lcm_hbm_state 2>/dev/null; \
                  fi" || true
}

do_test() {
    require_device
    stage_binary
    say "== WARNING: this stops this project's previous instance and the vendor HIDL provider"
    stop_conflicts
    push_tree

    say "== running post-fs-data steps (VINTF swap + SELinux + calibration)"
    "$ADB" shell "sh $TARGET/post-fs-data.sh" || true
    reset_hbm

    say "== starting the HAL"
    "$ADB" shell "cd $TARGET && setsid ./files/fp_hal >> /data/adb/km6fod.log 2>&1 </dev/null & \
                  echo \$! > run/fp_hal.pid"
    sleep 2
    do_status
    echo
    echo "The HAL is running outside the supervisor. Reboot to clear the runtime state."
    echo "To remove this test install:  ./install.sh --uninstall"
}

do_install() {
    require_device
    build_zip
    say "== installing via Magisk"
    "$ADB" push "$ZIP" /data/local/tmp/ >/dev/null
    "$ADB" shell "magisk --install-module /data/local/tmp/$(basename "$ZIP")"
    say "Module installed. Reboot to activate."
    echo
    echo "NOTE: if another module also installs a fingerprint HAL and mounts the"
    echo "      same VINTF directory, disable that module's fingerprint part before"
    echo "      rebooting; only one IFingerprint/default instance may exist."
}

do_uninstall() {
    require_device
    say "== stopping supervisor and HAL"
    "$ADB" shell "if [ -f $TARGET/uninstall.sh ]; then sh $TARGET/uninstall.sh; fi" || true
    "$ADB" shell "touch $TARGET/remove 2>/dev/null; sleep 1; rm -rf $TARGET"
    reset_hbm
    say "Removed. Reboot to clear the VINTF bind mount (mounts are not persistent)."
}

do_status() {
    require_device
    say "== HAL process"
    "$ADB" shell "ps -A -o PID,ARGS | grep -E 'fp_hal' | grep -v grep" || echo "  (not running)"
    say "== AIDL service registration"
    "$ADB" shell "service list 2>/dev/null | grep -i 'fingerprint.IFingerprint' || echo '  (absent)'" || true
    say "== fingerprint provider"
    "$ADB" shell "dumpsys fingerprint 2>/dev/null | head -8" || true
    say "== panel HBM state"
    "$ADB" shell "cat /sys/kernel/tran_display/lcm_hbm_state 2>/dev/null" || true
    say "== VINTF manifest entry"
    "$ADB" shell "ls /vendor/etc/vintf/manifest/ 2>/dev/null | grep -i fingerprint || echo '  (none)'" || true
    say "== supervisors / conflicts"
    "$ADB" shell "ps -A -o PID,ARGS | grep -E 'fp_hal|fingerprint@2.1-service' | grep -v grep" || echo "  (none)"
    say "== module log (tail)"
    "$ADB" shell "tail -n 30 /data/adb/km6fod.log 2>/dev/null" || true
    say "== logcat (tail)"
    "$ADB" shell "logcat -d -s km6fod 2>/dev/null | tail -20" || true
}

do_diag() {
    require_device
    say "== HBM kernel log"
    "$ADB" shell "dmesg 2>/dev/null | grep -iE 'tran_lcm_hbm_store|panel_hbm_set_cmdq|skip set hbm' | tail -30" || true
    say "== input devices"
    "$ADB" shell "cat /proc/bus/input/devices 2>/dev/null | grep -E 'Name=|Handlers='" || true
    say "== SELinux denials"
    "$ADB" shell "logcat -d -b all 2>/dev/null | grep -i 'avc:' | tail -30" || true
    say "== fingerprint logcat"
    "$ADB" shell "logcat -d -b all 2>/dev/null | grep -iE 'km6fod|silead|silfp|TRAN_FULL_HBM' | tail -60" || true
}

case "${1:-}" in
    --zip)       build_zip ;;
    --test)      do_test ;;
    --install)   do_install ;;
    --uninstall) do_uninstall ;;
    --status)    do_status ;;
    --diag)      do_diag ;;
    *)
        sed -n '3,16p' "$0" | sed 's/^# \{0,1\}//'
        exit 1
        ;;
esac
