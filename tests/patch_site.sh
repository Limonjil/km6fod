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
# Runs on the host: no device, no NDK, no build. Assert-based, no framework.
#
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SCRIPT="$ROOT/magisk_module/post-fs-data.sh"
REF="$ROOT/patched/libsl_fp_impl.so"

fail() { echo "FAIL: $*" >&2; exit 1; }

[ -f "$SCRIPT" ] || fail "post-fs-data.sh not found"
[ -f "$REF" ] || fail "reference artifact patched/libsl_fp_impl.so not found"

# Constants are read from the script itself, so the test cannot pass while the
# script disagrees with itself.
OFFSET_HEX="$(sed -n 's/^PATCH_OFFSET=\$((\(0x[0-9a-fA-F]*\)))$/\1/p' "$SCRIPT")"
BEFORE="$(sed -n 's/^PATCH_BEFORE="\([0-9a-fA-F]*\)".*/\1/p' "$SCRIPT")"
AFTER="$(sed -n 's/^PATCH_AFTER="\([0-9a-fA-F]*\)".*/\1/p' "$SCRIPT")"

[ -n "$OFFSET_HEX" ] || fail "PATCH_OFFSET not found in post-fs-data.sh"
[ -n "$BEFORE" ] || fail "PATCH_BEFORE not found in post-fs-data.sh"
[ -n "$AFTER" ] || fail "PATCH_AFTER not found in post-fs-data.sh"

OFFSET=$((OFFSET_HEX))

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
cp "$REF" "$TMP/lib.so"

hex_at() { od -An -tx1 -j "$2" -N 4 "$1" | tr -d ' \n'; }

# 1. The reference artifact is the patched one: the site must read PATCH_AFTER.
got="$(hex_at "$TMP/lib.so" "$OFFSET")"
[ "$got" = "$AFTER" ] || fail "reference reads '$got' at the site, PATCH_AFTER is '$AFTER'"

# 2. Put the original instruction back (tbnz w0, #0x1f = bytes 20 02 f8 37) and
#    require the guard's own expectation to match the file bytes.
printf '\040\002\370\067' | dd of="$TMP/lib.so" bs=1 seek="$OFFSET" conv=notrunc 2>/dev/null
got="$(hex_at "$TMP/lib.so" "$OFFSET")"
[ "$got" = "$BEFORE" ] || fail "original word reads '$got', PATCH_BEFORE is '$BEFORE'"

# 3. Applying the patch bytes must produce exactly PATCH_AFTER (NOP).
printf '\037\040\003\325' | dd of="$TMP/lib.so" bs=1 seek="$OFFSET" conv=notrunc 2>/dev/null
got="$(hex_at "$TMP/lib.so" "$OFFSET")"
[ "$got" = "$AFTER" ] || fail "patched word reads '$got', PATCH_AFTER is '$AFTER'"

echo "PASS: site 0x$(printf %x "$OFFSET"): $BEFORE -> $AFTER"
echo "      (byte order checked against od, not against the instruction word)"
