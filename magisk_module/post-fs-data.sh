#!/system/bin/sh
#
# post-fs-data for the KM6 FOD module.
#
# Runs from the Magisk post-fs-data stage: /vendor is already mounted, the
# package manager has not scanned anything yet, and no framework client has
# looked for a fingerprint HAL. That is exactly the window needed to swap the
# VINTF manifest and apply SELinux rules.
#
# The HAL itself is NOT started here on purpose: Binder registration succeeds
# only from the late_start stage.

MODDIR=${0%/*}
LOGFILE=/data/adb/km6fod.log
# One runtime directory, owned by the HAL; every script must use it (review H-32).
RUN="/data/adb/km6fod/run"
HBM=/sys/kernel/tran_display/lcm_hbm_state

umask 077
mkdir -p "$RUN" 2>/dev/null
chmod 0755 "$RUN" 2>/dev/null

if ! ( : >> "$LOGFILE" ) 2>/dev/null; then
    LOGFILE=/dev/null
fi
if [ "$LOGFILE" != /dev/null ] && [ "$(wc -c < "$LOGFILE" 2>/dev/null || echo 0)" -gt 524288 ]; then
    tail -c 262144 "$LOGFILE" > "$LOGFILE.tmp" 2>/dev/null && mv -f "$LOGFILE.tmp" "$LOGFILE"
fi
[ "$LOGFILE" != /dev/null ] && chmod 0600 "$LOGFILE" 2>/dev/null

log() { echo "$(date '+%Y-%m-%d %H:%M:%S') $*" >> "$LOGFILE" 2>/dev/null; }

# Manager probe. MAGISK_VER_CODE is spoofed on KernelSU and APatch, so it must
# never be used to tell the managers apart (review F-004). The rules below
# address the domain of the shell the manager gives us, not a guessed name.
MANAGER=unknown
if [ -n "${APATCH:-}" ] || [ -e /data/adb/ap ]; then
    MANAGER=apatch
elif [ -n "${KSU:-}" ] || [ -d /data/adb/ksu ]; then
    MANAGER=kernelsu
elif [ -x /data/adb/magisk/magisk ]; then
    MANAGER=magisk
fi
log "manager: $MANAGER"
if [ "$MANAGER" = "kernelsu" ]; then
    log "manager: KernelSU этим модулем не поддержан (см. module.prop): правила и монтирование рассчитаны на Magisk/APatch"
fi

PAGE_SIZE=$(getconf PAGE_SIZE 2>/dev/null || echo 4096)
if [ "$PAGE_SIZE" != "4096" ]; then
    log "vm: PAGE_SIZE=$PAGE_SIZE — заявлен и проверен только 4 KB; поведение на 16 KB не гарантировано"
fi

# Boot-time HBM recovery. The driver does not latch a value written while the
# display is asleep (verified on device), but a previous crash marker means we
# still write 0 once and read it back.
if [ -e "$HBM" ]; then
    [ -f "$RUN/hbm_owned" ] && log "HBM: recovered an unclean shutdown marker"
    printf '0' > "$HBM" 2>/dev/null
    log "HBM: boot state $(cat "$HBM" 2>/dev/null)"
    rm -f "$RUN/hbm_owned" 2>/dev/null
fi

log "=== km6fod: post-fs-data ==="

# ------------------------------------------- 0. Take the vendor service offline
# init declares the vendor fingerprint provider as `class late_start`. If it
# starts, it registers a HIDL fingerprint service, system_server builds a HIDL
# sensor for it and the resulting client wedges the biometric scheduler. The
# module supplies its own AIDL HAL instead, so the init service is disabled here
# (post-fs-data runs before class late_start).
for svc in vendor.tran_fps_hal vendor.fps_hal \
           android.hardware.biometrics.fingerprint@2.1-service-mediatek; do
    stop "$svc" 2>/dev/null
    setprop "ctl.stop" "$svc" 2>/dev/null
done
log "vendor HIDL fp -> $(getprop init.svc.vendor.tran_fps_hal 2>/dev/null)"

# --------------------------------------------------------------- 1. SELinux
# The HAL runs from the Magisk late_start context and therefore inherits the
# `magisk` domain rather than `hal_fingerprint_default`.
#
# GRAMMAR WARNING: this magiskpolicy build accepts only the space separated form
#
#     allow <source> <target> <class> <perm_set>
#
# The `target:class` form used by most SELinux tooling is rejected -- and it is
# rejected *silently*, because --apply/--live still exit with status 0. That is
# why colon-style rules appear to apply yet never take effect. Every rule below
# is therefore written in the accepted form and the output is inspected instead
# of trusting the exit status.
#
# File-class rules (`... file ...`, `... chr_file ...`, `... dir ...`) are not
# used here: this policy build rejects every one of them at parse time. They are
# not needed either -- the `magisk` domain already reaches the sensor node, the
# digitizer and the panel node, which is how the previous vendor HAL ran in the
# same domain without any module rule. Only the socket permissions are genuinely
# missing and are granted below.
POLICY=$(command -v magiskpolicy 2>/dev/null)
[ -n "$POLICY" ] || POLICY=/sbin/magiskpolicy

apply_rule() {
    if [ ! -x "$POLICY" ]; then
        log "sepolicy: magiskpolicy missing, skipped: $1"
        return 0
    fi
    out=$("$POLICY" --live "$1" 2>&1)
    case "$out" in
        *"Syntax error"*|*"does not exist"*|*"Could not"*)
            log "sepolicy: ОТКЛОНЕНО: $1"
            log "sepolicy: причина: $(echo "$out" | head -2 | tr '\n' ' ')"
            ;;
        *)
            log "sepolicy: $1"
            ;;
    esac
    return 0
}

if [ -f "$MODDIR/sepolicy.rule" ]; then
    while IFS= read -r rule; do
        case "$rule" in
            ''|'#'*) continue ;;
        esac
        apply_rule "$rule"
    done < "$MODDIR/sepolicy.rule"
fi

# Optional coordination-channel rules. Kept separate so a rejected one cannot
# stop the rest; a missing permission here only costs the vendor ext channel.
apply_rule "allow magisk hal_fingerprint_default fd use"
apply_rule "allow magisk hal_fingerprint_default unix_stream_socket { read write getattr }"

# ------------------------------------------------------------- 1b. Overlays
# Static RROs must carry the system_file label or PackageManager skips them
# without a word. KM6FpAidl clears config_biometric_sensors, which is what makes
# AuthService take the AIDL branch instead of the legacy HIDL one.
for apk in "$MODDIR"/system/product/overlay/*/*.apk; do
    [ -f "$apk" ] || continue
    chmod 0644 "$apk" 2>/dev/null
    if chcon u:object_r:system_file:s0 "$apk" 2>/dev/null; then
        log "overlay: ${apk##*/} labelled"
    else
        log "overlay: ${apk##*/} НЕ ПОМЕЧЕН"
    fi
done

# --------------------------------------------------- 1c. Vendor namespace
# A process executed from /data gets the "default" linker namespace, whose
# permitted paths exclude /vendor/lib64, so dlopen of the vendor engine fails
# with "is not accessible for the namespace". Bind-mounting the binary over the
# vendor HIDL binary and executing it from there puts it in the vendor
# namespace. The init service that owns this path is stopped in step 0, so the
# mount point is free and nothing else executes it.
VENDOR_FP=/vendor/bin/hw/android.hardware.biometrics.fingerprint@2.1-service-mediatek
# A hot update cannot overwrite files/fp_hal while the previous boot's bind mount
# still holds the inode (ETXTBSY), so an update is staged next to it and swapped in
# here, before the mount is created.
for staged in fp_hal; do
    if [ -f "$MODDIR/files/$staged.new" ]; then
        log "update: installing staged $staged"
        cat "$MODDIR/files/$staged.new" > "$MODDIR/files/$staged" 2>/dev/null &&
            rm -f "$MODDIR/files/$staged.new"
    fi
done
if [ -f "$MODDIR/files/fp_hal" ] && [ -e "$VENDOR_FP" ]; then
    chmod 0755 "$MODDIR/files/fp_hal" 2>/dev/null
    chcon u:object_r:vendor_file:s0 "$MODDIR/files/fp_hal" 2>/dev/null
    if mount --bind "$MODDIR/files/fp_hal" "$VENDOR_FP" 2>/dev/null; then
        log "hal: bound to $VENDOR_FP (vendor namespace)"
    else
        log "hal: НЕ УДАЛОСЬ смонтировать $VENDOR_FP"
    fi
fi

# ------------------------- 1c2. Touch keeps scanning on a dark panel
# Stock focaltech_ft3683g puts the controller into firmware gesture mode when the
# display blanks, and adaptive-ts sends its own suspend command; after that a press
# on a dark screen produces no MT events at all, only a tap gesture. The two
# patched copies in files/ keep the controller in normal scan mode, so the HAL sees
# the finger that is already resting on the sensor when the panel comes up.
# The patches are four bytes each and are valid only for the exact vendor builds
# hashed below: on any mismatch the vendor modules are used unchanged.
TOUCH_VENDOR=/vendor/lib/modules
AD_SHA=e824f57549c25f920ceb6aa7a9e982cc238d375b94273b93bb2872156d1e14d4
FTS_SHA=7098b0b2d2a824f4f06689784ecfb5fc32e2defe8b2c79ffba22ffc294ccaed9
if [ -f "$MODDIR/files/adaptive-ts.ko" ] && [ -f "$MODDIR/files/focaltech_ft3683g.ko" ]; then
    ad_sum="$(sha256sum "$TOUCH_VENDOR/adaptive-ts.ko" 2>/dev/null | cut -d' ' -f1)"
    fts_sum="$(sha256sum "$TOUCH_VENDOR/focaltech_ft3683g.ko" 2>/dev/null | cut -d' ' -f1)"
    if [ "$ad_sum" = "$AD_SHA" ] && [ "$fts_sum" = "$FTS_SHA" ]; then
        for ko in adaptive-ts focaltech_ft3683g; do
            chmod 0644 "$MODDIR/files/$ko.ko" 2>/dev/null
            chcon u:object_r:vendor_file:s0 "$MODDIR/files/$ko.ko" 2>/dev/null
        done
        # focaltech holds adaptive_ts, so it has to go first in both directions.
        rmmod focaltech_ft3683g 2>/dev/null
        rmmod adaptive_ts 2>/dev/null
        if insmod "$MODDIR/files/adaptive-ts.ko" 2>/dev/null; then
            log "touch: adaptive-ts patched build loaded"
        else
            insmod "$TOUCH_VENDOR/adaptive-ts.ko" 2>/dev/null &&
                log "touch: adaptive-ts FAILED, vendor copy loaded"
        fi
        if insmod "$MODDIR/files/focaltech_ft3683g.ko" 2>/dev/null; then
            log "touch: focaltech patched build loaded"
        else
            insmod "$TOUCH_VENDOR/focaltech_ft3683g.ko" 2>/dev/null &&
                log "touch: focaltech FAILED, vendor copy loaded"
        fi
    else
        log "touch: vendor modules changed, stock drivers kept"
    fi
fi


# --------------------------------- 1d. Skip the vendor pre-flight stub
# In this vendor build the challenge pre-flight is NOT implemented: the library
# logs "No implement fp_verify_enroll_challenge" and returns an uninitialised
# code, so its error branch aborts every enroll. The code is not stable either
# (observed -1805537232, -1004, ...) - there is nothing to match on, which is why
# the patch is a plain nop and not a narrow one (NOTICE.md, "The vendor pre-flight
# patch").
#
#   silfp_enroll_enroll, vaddr 0x3645c (file offset 0x2f45c)
#     37f80220  tbnz w0, #0x1f, <error handler>
#     1f2003d5  nop     (what this script writes)
#
# Nothing else in the function changes, and the cave at 0xe008 is no longer
# reachable: it is still written because the guard verifies that region is
# zero-filled, but nothing branches to it.
#
# What the module must NOT do (review F-003) is ship a pre-patched copy of one
# vendor build and mount it blind: on any other vendor revision the four bytes
# land in an unrelated instruction.
#
# Here the device patches ITSELF: copy /vendor/lib64/libsl_fp_impl.so, verify the
# site and the free cave, patch the copy, mount the copy. Any mismatch means "no
# patch": enroll then fails with HW_UNAVAILABLE while authenticate keeps working,
# and the reason is one line in the log.
VENDOR_LIB=/vendor/lib64/libsl_fp_impl.so
ENGINE_DIR="$RUN/engine"
ENGINE_LIB="$ENGINE_DIR/libsl_fp_impl.so"
PATCH_OFFSET=$((0x2f45c))
PATCH_BEFORE="2002f837"   # file bytes of tbnz w0, #0x1f (word 0x37f80220)
# 1f2003d5 = nop: the stub's error branch is not taken. The cave below is dead
# code until someone branches to it; it is kept so the guard keeps proving the
# region is free space in this vendor build.
PATCH_AFTER="1f2003d5"
CAVE_OFFSET=$((0xe008))   # VA 0x15008
CAVE_SIZE=$((0x14))       # 20 bytes: mov/movk/cmp/b.eq/b
CAVE_AFTER="09069652298cb2721f00096b60a2105423850014"

hex_at() {
    od -An -v -tx1 -j "$2" -N "${3:-4}" "$1" 2>/dev/null | tr -d ' \n'
}

patch_engine_lib() {
    if [ ! -f "$VENDOR_LIB" ]; then
        log "engine: $VENDOR_LIB отсутствует — патч не требуется"
        return 0
    fi
    mkdir -p "$ENGINE_DIR" 2>/dev/null
    if ! cp -f "$VENDOR_LIB" "$ENGINE_LIB" 2>/dev/null; then
        log "engine: ОТКАЗ: не удалось скопировать $VENDOR_LIB"
        return 1
    fi
    before=$(hex_at "$ENGINE_LIB" "$PATCH_OFFSET")
    if [ "$before" != "$PATCH_BEFORE" ]; then
        log "engine: ОТКАЗ: сайт патча не совпал (0x2f45c='$before', ожидался $PATCH_BEFORE)"
        rm -f "$ENGINE_LIB" 2>/dev/null
        return 1
    fi
    cave_before=$(hex_at "$ENGINE_LIB" "$CAVE_OFFSET" "$CAVE_SIZE")
    if [ "${#cave_before}" -ne $((CAVE_SIZE * 2)) ] || [ -n "${cave_before%%0*}" ]; then
        log "engine: ОТКАЗ: пещера не свободна (0xe008='$cave_before')"
        rm -f "$ENGINE_LIB" 2>/dev/null
        return 1
    fi
    # Cave first, site after the cave has verified.
    printf '\011\006\226\122\051\214\262\162\037\000\011\153\140\242\020\124\043\205\000\024' | \
        dd of="$ENGINE_LIB" bs=1 seek="$CAVE_OFFSET" conv=notrunc 2>/dev/null
    cave_after=$(hex_at "$ENGINE_LIB" "$CAVE_OFFSET" $((${#CAVE_AFTER} / 2)))
    if [ "$cave_after" != "$CAVE_AFTER" ]; then
        log "engine: ОТКАЗ: пещера не записалась (0xe008='$cave_after')"
        rm -f "$ENGINE_LIB" 2>/dev/null
        return 1
    fi
    printf '\037\040\003\325' | dd of="$ENGINE_LIB" bs=1 seek="$PATCH_OFFSET" conv=notrunc 2>/dev/null
    after=$(hex_at "$ENGINE_LIB" "$PATCH_OFFSET")
    if [ "$after" != "$PATCH_AFTER" ]; then
        log "engine: ОТКАЗ: запись не подтвердилась (0x2f45c='$after')"
        rm -f "$ENGINE_LIB" 2>/dev/null
        return 1
    fi
    log "engine: библиотека скопирована, сайт $before -> $after, заглушка проверки пропущена"
    log "engine: пещера 0xe008 записана, но недостижима (ветки на неё больше нет)"
    return 0
}

if patch_engine_lib; then
    chmod 0644 "$ENGINE_LIB" 2>/dev/null
    chcon u:object_r:vendor_file:s0 "$ENGINE_LIB" 2>/dev/null
    if mount --bind "$ENGINE_LIB" "$VENDOR_LIB" 2>/dev/null; then
        log "engine: пропатченная копия смонтирована вместо $VENDOR_LIB"
    else
        log "engine: НЕ УДАЛОСЬ смонтировать $VENDOR_LIB"
    fi
else
    log "engine: патч не применён — enroll вернёт HW_UNAVAILABLE, authenticate работает"
fi

# ------------------------------------------------------------ 2. Calibration
# The engine refuses to capture reliably without calibration data. The working
# directory must be populated explicitly because a clean GSI install leaves it
# empty.
STORE=/data/vendor_de/0/fpdata
mkdir -p "$STORE" 2>/dev/null
chown system:system "$STORE" 2>/dev/null
chmod 0700 "$STORE" 2>/dev/null

for name in fpcal.dat fpcal4.dat fpcal5.dat fpcal6.dat; do
    src="/mnt/vendor/persist/$name"
    dst="$STORE/$name"
    [ -f "$dst" ] && continue
    [ -f "$src" ] || continue
    if cp -f "$src" "$dst" 2>/dev/null; then
        chown system:system "$dst" 2>/dev/null
        chmod 0600 "$dst" 2>/dev/null
        command -v restorecon >/dev/null 2>&1 && restorecon "$dst" 2>/dev/null
        log "calibration: $name copied"
    else
        log "calibration: failed to copy $name"
    fi
done

# --------------------------------------------------------------- 3. VINTF
# /vendor/etc/vintf/manifest is read-only EROFS, so a single extra file cannot be
# added. A directory is bind-mounted instead: every stock manifest, minus the
# HIDL fingerprint provider, plus our AIDL declaration. Leaving the HIDL entry
# in place makes the framework build a HIDL client that then blocks the
# biometric scheduler forever.
VINTF_TARGET=/vendor/etc/vintf/manifest
STAGE="$RUN/vintf"

# Unmount a previous boot's bind mount before copying, otherwise the staging
# copy would include our own AIDL entry and stale files.
VINTF_OK=1
if grep -q " $VINTF_TARGET " /proc/mounts 2>/dev/null; then
    if umount "$VINTF_TARGET" 2>/dev/null; then
        log "vintf: previous mount removed"
    else
        # Never rm -rf a staging directory that is still the live mount: that
        # deletes the mounted manifests while init still reads them (F-014).
        VINTF_OK=0
        log "vintf: ВНИМАНИЕ: прежний mount занят, staging не пересоздаётся"
    fi
fi

[ "$VINTF_OK" = "1" ] && rm -rf "$STAGE" 2>/dev/null
mkdir -p "$STAGE" 2>/dev/null

# Remove only <hal> blocks that declare the biometric fingerprint HAL. Do not
# drop whole files: manifest.xml also carries unrelated HALs on some builds.
count_tag() { grep -o "$2" "$1" 2>/dev/null | wc -l | tr -d ' '; }

# Remove only <hal> blocks that declare the biometric fingerprint HAL. Returns
# non-zero unless the result is provably "the input minus exactly one block and
# nothing else", so the caller can keep the original file (review F-009). Losing
# an unrelated HAL from the vendor manifest is not acceptable collateral.
strip_fingerprint_hal() {
    awk '
        /<hal[ >]/ {
            if ($0 ~ /<\/hal>/) {
                if ($0 !~ /<name>android\.hardware\.biometrics\.fingerprint<\/name>/) print $0
                next
            }
            buf = $0 ORS
            inblock = 1
            next
        }
        inblock {
            buf = buf $0 ORS
            if ($0 ~ /<\/hal>/) {
                if (buf !~ /<name>android\.hardware\.biometrics\.fingerprint<\/name>/) printf "%s", buf
                inblock = 0
                buf = ""
            }
            next
        }
        { print }
    ' "$1" > "$2" 2>/dev/null || return 1

    [ -s "$2" ] || return 1

    before_hal=$(count_tag "$1" '<hal')
    after_hal=$(count_tag "$2" '<hal')
    after_close=$(count_tag "$2" '</hal>')
    fp_before=$(count_tag "$1" 'android\.hardware\.biometrics\.fingerprint</name>')
    fp_after=$(count_tag "$2" 'android\.hardware\.biometrics\.fingerprint</name>')

    # Exactly one fingerprint block must disappear, no other <hal> may vanish and
    # every opened block must be closed again.
    if [ "$fp_before" -ne 1 ] || [ "$fp_after" -ne 0 ]; then
        return 1
    fi
    if [ $((before_hal - 1)) -ne "$after_hal" ]; then
        return 1
    fi
    if [ "$after_hal" -ne "$after_close" ]; then
        return 1
    fi
    return 0
}

if [ -d "$VINTF_TARGET" ]; then
    count=0
    for src in "$VINTF_TARGET"/*.xml; do
        [ -f "$src" ] || continue
        base=${src##*/}
        case "$base" in
            *fingerprint@*.xml|manifest_fingerprint.xml) continue ;;
        esac
        if ! grep -q 'android\.hardware\.biometrics\.fingerprint' "$src" 2>/dev/null; then
            cp -f "$src" "$STAGE/$base" 2>/dev/null
        elif strip_fingerprint_hal "$src" "$STAGE/$base"; then
            log "vintf: ${base}: fingerprint HAL убран, остальные блоки сохранены"
        else
            log "vintf: ${base}: fingerprint HAL не убрать безопасно — файл оставлен без изменений"
            cp -f "$src" "$STAGE/$base" 2>/dev/null
        fi
        [ -s "$STAGE/$base" ] || rm -f "$STAGE/$base" 2>/dev/null
        count=$((count + 1))
    done

    if [ -s "$MODDIR/vintf/fingerprint_aidl.xml" ]; then
        cp -f "$MODDIR/vintf/fingerprint_aidl.xml" "$STAGE"/ 2>/dev/null
    else
        log "vintf: ОШИБКА: $MODDIR/vintf/fingerprint_aidl.xml отсутствует или пуст"
    fi

    # The declared AIDL version must sit inside the band the running system's
    # compatibility matrix accepts and must not exceed what this build implements
    # (AOSP android17 sources, IFingerprint/ISession/ISessionCallback v5). Reading
    # the matrix rather than the SDK number is the only way to be right on both
    # Android 15 (3-4) and Android 16/17 (3-5).
    HAL_VERSION_IMPL=5
    matrix_max=$(grep -h -A2 '<name>android.hardware.biometrics.fingerprint</name>' \
        /system/etc/vintf/compatibility_matrix*.xml 2>/dev/null |
        sed -n 's/.*<version>[0-9]*-\([0-9]*\)<.*/\1/p' | sort -n | tail -1)
    [ -n "$matrix_max" ] || matrix_max=4
    [ "$matrix_max" -gt "$HAL_VERSION_IMPL" ] && matrix_max=$HAL_VERSION_IMPL
    [ "$matrix_max" -lt 3 ] && matrix_max=3
    if [ -s "$STAGE/fingerprint_aidl.xml" ]; then
        sed -i "s|<version>[0-9]*</version>|<version>$matrix_max</version>|" \
            "$STAGE/fingerprint_aidl.xml" 2>/dev/null
        log "vintf: fingerprint AIDL version declared $matrix_max (implemented $HAL_VERSION_IMPL, matrix max $matrix_max)"
    fi

    chcon -R u:object_r:vendor_configs_file:s0 "$STAGE" 2>/dev/null
    chmod 0755 "$STAGE" 2>/dev/null
    chmod 0644 "$STAGE"/*.xml 2>/dev/null

    if [ "$VINTF_OK" = "1" ] && [ -s "$STAGE/fingerprint_aidl.xml" ] && mount --bind "$STAGE" "$VINTF_TARGET" 2>/dev/null; then
        mounted=$(ls "$STAGE"/*.xml 2>/dev/null | wc -l)
        log "vintf: mounted $mounted manifests (input $count), AIDL fingerprint declared"
        # Keep the mount private to this namespace when the kernel supports it.
        mount --make-private "$VINTF_TARGET" 2>/dev/null || true
    else
        log "vintf: НЕ УДАЛОСЬ смонтировать $VINTF_TARGET"
    fi
else
    log "vintf: $VINTF_TARGET отсутствует"
fi

# A single-file manifest cannot be replaced by a directory bind mount. Warn
# instead of silently leaving a HIDL client path behind.
if [ -f /vendor/etc/vintf/manifest.xml ] &&
   grep -q 'android.hardware.biometrics.fingerprint' /vendor/etc/vintf/manifest.xml 2>/dev/null; then
    log "vintf: ВНИМАНИЕ: /vendor/etc/vintf/manifest.xml содержит fingerprint HAL"
fi

# The vendor HIDL provider opens /dev/silead_fp exclusively. Stop it before the
# HAL starts; init may otherwise reacquire the node after our late_start.
stop vendor.tran_fps_hal 2>/dev/null
stop vendor.fps_hal 2>/dev/null

# ------------------------------------------------------------ 4. Permissions
chmod 0755 "$MODDIR/files/fp_hal" 2>/dev/null
chmod 0755 "$MODDIR/service.sh" "$MODDIR/post-fs-data.sh" "$MODDIR/uninstall.sh" 2>/dev/null

# The HAL runs as `user system` under init, so it cannot create its own instance
# lock under /data/adb. Pre-create it with the right owner; the binary falls back
# to running without a lock if it still cannot be opened.
mkdir -p "$RUN" 2>/dev/null

# Mounts and processes do not survive a reboot, so a lock directory left in /data
# is always stale. Its stored pid may even belong to an unrelated process now
# (pid reuse), which would make service.sh refuse to start the supervisor.
rm -rf "$RUN/supervisor.lock" 2>/dev/null
rm -f "$RUN/fp_hal.pid" 2>/dev/null

chown system:system "$RUN" 2>/dev/null
chmod 0775 "$RUN" 2>/dev/null
: > "$RUN/fp_hal.lock" 2>/dev/null
chown system:system "$RUN/fp_hal.lock" 2>/dev/null
chmod 0600 "$RUN/fp_hal.lock" 2>/dev/null

# The HAL appends nothing to the module log (its stdout goes to logcat), but the
# scripts keep writing here; make it readable for post-mortems.
chmod 0600 "$LOGFILE" 2>/dev/null

log "=== km6fod: post-fs-data готово ==="
