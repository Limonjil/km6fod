#!/system/bin/sh
set -e
MOD=/data/adb/modules/km6fod/files
V=/vendor/bin/hw/android.hardware.biometrics.fingerprint@2.1-service-mediatek
# A previous deploy can leave the backup name occupied (or a mount point), so
# never reuse a fixed name for the old inode.
mv -f "$MOD/fp_hal" "$MOD/fp_hal.$(date +%s)" 2>/dev/null || true
cp "$MOD/fp_hal.new" "$MOD/fp_hal"
chmod 0755 "$MOD/fp_hal"
chcon u:object_r:vendor_file:s0 "$MOD/fp_hal"
mount --bind "$MOD/fp_hal" "$V"
md5sum "$MOD/fp_hal" "$V"
PID=$(cat /data/adb/km6fod/run/fp_hal.pid 2>/dev/null)
echo "killing $PID"
kill "$PID" 2>/dev/null || true
