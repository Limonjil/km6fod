#!/system/bin/sh
sleep 25
/system/bin/input keyevent 224
sleep 2
for i in 1 2 3; do
  echo 1 > /sys/kernel/tran_display/lcm_hbm_state
  sleep 1
  echo 0 > /sys/kernel/tran_display/lcm_hbm_state
  sleep 1
done
echo done > /data/local/tmp/hbm_watch.done
