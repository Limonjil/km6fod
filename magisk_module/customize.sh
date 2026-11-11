#!/system/bin/sh
#
# Sourced by the installer of Magisk, KernelSU and APatch after the payload is
# extracted (review F-001). All three export MODPATH; only the manager variables
# differ, and MAGISK_VER_CODE is spoofed by the other two, so KSU and APATCH are
# tested first.
#
# The policy file is per-manager because the process domain differs: a rule that
# names an undefined type is not skipped, the whole sepolicy.rule is rejected.
if [ -n "$MODPATH" ] && [ "$KSU" = "true" ] && [ -f "$MODPATH/sepolicy.rule.ksu" ]; then
    cp -f "$MODPATH/sepolicy.rule.ksu" "$MODPATH/sepolicy.rule"
fi
true
