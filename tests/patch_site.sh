#!/usr/bin/env bash
#
# Behaviour test for the vendor-library patch guard in post-fs-data.sh.
#
# The guard compares the four bytes at the patch site with PATCH_BEFORE. That
# comparison is byte order, not word order: od prints file bytes, so the word
# 0x37f80220 reads as "2002f837". Getting that wrong makes the guard refuse the
# patch on a perfectly good vendor build - which is exactly what happened on the
# device at 2026-11-04 02:41.
#
# The same file-bytes rule applies to the trampoline cave: its code is checked
# against the reference artifact at the file offset.
#
# Runs on the host: no device, no NDK, no build. Assert-based, no framework.
#
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SCRIPT="$ROOT/magisk_module/post-fs-data.sh"
REF="$ROOT/patched/libsl_fp_impl.so"

fail() { echo "FAIL: $*" >&2; exit 1; }

[ -f "$SCRIPT" ] || fail "post-fs-data.sh not found"
if [ ! -f "$REF" ]; then
    # The reference is a proprietary vendor binary and is not in git; without it
    # there is nothing to compare the constants against.
    echo "SKIP: patched/libsl_fp_impl.so not present (vendor reference, not in git)"
    exit 0
fi

# Constants are read from the script itself, so the test cannot pass while the
# script disagrees with itself.
OFFSET_HEX="$(sed -n 's/^PATCH_OFFSET=\$((\(0x[0-9a-fA-F]*\)))$/\1/p' "$SCRIPT")"
BEFORE="$(sed -n 's/^PATCH_BEFORE="\([0-9a-fA-F]*\)".*/\1/p' "$SCRIPT")"
AFTER="$(sed -n 's/^PATCH_AFTER="\([0-9a-fA-F]*\)".*/\1/p' "$SCRIPT")"
CAVE_OFFSET_HEX="$(sed -n 's/^CAVE_OFFSET=\$((\(0x[0-9a-fA-F]*\))).*/\1/p' "$SCRIPT")"
CAVE_AFTER="$(sed -n 's/^CAVE_AFTER="\([0-9a-fA-F]*\)".*/\1/p' "$SCRIPT")"

[ -n "$OFFSET_HEX" ] || fail "PATCH_OFFSET not found in post-fs-data.sh"
[ -n "$BEFORE" ] || fail "PATCH_BEFORE not found in post-fs-data.sh"
[ -n "$AFTER" ] || fail "PATCH_AFTER not found in post-fs-data.sh"
[ -n "$CAVE_OFFSET_HEX" ] || fail "CAVE_OFFSET not found in post-fs-data.sh"
[ -n "$CAVE_AFTER" ] || fail "CAVE_AFTER not found in post-fs-data.sh"

OFFSET=$((OFFSET_HEX))
CAVE_OFFSET=$((CAVE_OFFSET_HEX))

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
cp "$REF" "$TMP/lib.so"

hex_at() { od -An -v -tx1 -j "$2" -N 4 "$1" | tr -d ' \n'; }
hex_range() { od -An -v -tx1 -j "$2" -N "$3" "$1" | tr -d ' \n'; }

# 1. The reference artifact is the patched one: the site must read PATCH_AFTER.
got="$(hex_at "$TMP/lib.so" "$OFFSET")"
[ "$got" = "$AFTER" ] || fail "reference reads '$got' at the site, PATCH_AFTER is '$AFTER'"

# 2. Put the original instruction back (tbnz w0, #0x1f = bytes 20 02 f8 37) and
#    require the guard's own expectation to match the file bytes.
printf '\040\002\370\067' | dd of="$TMP/lib.so" bs=1 seek="$OFFSET" conv=notrunc 2>/dev/null
got="$(hex_at "$TMP/lib.so" "$OFFSET")"
[ "$got" = "$BEFORE" ] || fail "original word reads '$got', PATCH_BEFORE is '$BEFORE'"

# 3. Applying the patch bytes from the script must produce exactly PATCH_AFTER.
printf '%b' "$(echo "$AFTER" | sed 's/../\\x&/g')" | \
    dd of="$TMP/lib.so" bs=1 seek="$OFFSET" conv=notrunc 2>/dev/null
got="$(hex_at "$TMP/lib.so" "$OFFSET")"
[ "$got" = "$AFTER" ] || fail "patched word reads '$got', PATCH_AFTER is '$AFTER'"

# 4. The reference artifact must carry the cave code the script writes there.
got="$(hex_range "$TMP/lib.so" "$CAVE_OFFSET" $((${#CAVE_AFTER} / 2)))"
[ "$got" = "$CAVE_AFTER" ] || fail "cave reads '$got', CAVE_AFTER is '$CAVE_AFTER'"

echo "PASS: site 0x$(printf %x "$OFFSET"): $BEFORE -> $AFTER, cave verified"
echo "      (byte order checked against od, not against the instruction word)"
