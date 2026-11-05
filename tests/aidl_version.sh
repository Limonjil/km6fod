#!/usr/bin/env bash
#
# Regression test for the AIDL version contract (review F-007).
#
# The HAL must (a) be built from stubs that carry a real version, and (b) declare,
# in the VINTF fragment, a version that the running system's compatibility matrix
# accepts. Getting (a) wrong makes the HAL report 1 while claiming 2 in the
# fragment; getting (b) wrong makes the declaration miss the band entirely.
#
# Runs on the host: no device, no NDK.
#
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
GEN="$ROOT/third_party/aidl_gen/include/aidl/android/hardware/biometrics/fingerprint"
XML="$ROOT/magisk_module/vintf/fingerprint_aidl.xml"
SCRIPT="$ROOT/magisk_module/post-fs-data.sh"

fail() { echo "FAIL: $*" >&2; exit 1; }

IMPL=5

# 1. Every interface this HAL exposes must carry the same implemented version.
for iface in IFingerprint ISession ISessionCallback; do
    [ -f "$GEN/$iface.h" ] || fail "$iface.h not generated"
    got="$(sed -n 's/.*static inline const int32_t version = \([0-9]*\);.*/\1/p' "$GEN/$iface.h" | head -1)"
    [ -n "$got" ] || fail "$iface has no version constant (stubs were generated without --version)"
    [ "$got" = "$IMPL" ] || fail "$iface version is $got, expected $IMPL"
done

# 2. The fallback declaration in the fragment must already be inside the A15 band
#    (3-4) and inside the A16/17 band (3-5), so an unanswered matrix still works.
decl="$(sed -n 's:.*<version>\([0-9]*\)</version>.*:\1:p' "$XML" | head -1)"
[ -n "$decl" ] || fail "no <version> in $XML"
[ "$decl" -ge 3 ] && [ "$decl" -le "$IMPL" ] || fail "fragment declares $decl, expected 3..$IMPL"

# 3. The boot script must derive the declared version from the device's matrix,
#    because Android 15 caps the band at 4 while Android 16/17 allow 5.
grep -q 'compatibility_matrix\*\.xml' "$SCRIPT" || fail "post-fs-data.sh does not read the compatibility matrix"
grep -q "HAL_VERSION_IMPL=$IMPL" "$SCRIPT" || fail "post-fs-data.sh does not pin HAL_VERSION_IMPL=$IMPL"

echo "PASS: stubs v$IMPL, fragment fallback v$decl, matrix-derived declaration present"
