MODDIR=/data/adb/modules/km6fod
AD_MOD=$MODDIR/files/adaptive-ts.ko
FTS_MOD=$MODDIR/files/focaltech_ft3683g.ko
rmmod focaltech_ft3683g 2>&1
rmmod adaptive_ts 2>&1
insmod "$AD_MOD" 2>&1 && echo "ad: patched loaded"
insmod "$FTS_MOD" 2>&1 && echo "fts: patched loaded"
lsmod | grep -E "adaptive|focaltech"
ls -l /dev/input/event5 2>&1
