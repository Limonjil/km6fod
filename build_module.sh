#!/usr/bin/env bash
#
# Builds the HAL and packages the Magisk module.
#
# Output: dist/km6fod-<version>.zip (flashable in the Magisk app)
#
# Provenance rules (review F-001, F-002, F-021):
#   * the payload in magisk_module/files must be byte-identical to build/,
#     otherwise packaging fails;
#   * file modes come from a table, never from a suffix;
#   * every packaged ELF must be 16 KB page aligned or packaging fails;
#   * version and versionCode come from one source (VERSION).
#
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
MODULE="$ROOT/magisk_module"
DIST="$ROOT/dist"

VERSION="$(sed -n 's/^version=//p' "$MODULE/module.prop")"
VERSION_CODE="$(sed -n 's/^versionCode=//p' "$MODULE/module.prop")"
if [ -z "$VERSION" ] || [ -z "$VERSION_CODE" ]; then
    echo "error: module.prop must carry version and versionCode" >&2
    exit 1
fi
ZIP="$DIST/km6fod-${VERSION}.zip"

echo "== building HAL"
bash "$ROOT/build.sh"

echo "== staging module payload"
mkdir -p "$MODULE/files"
cp -f "$ROOT/build/fp_hal" "$MODULE/files/fp_hal"


# Remove editor and runtime leftovers explicitly: the supervisor creates run/
# on the device and it must never be captured into an archive.
rm -rf "$MODULE/run"
find "$MODULE" -name '*.sh~' -delete 2>/dev/null || true
find "$MODULE" -name '*.log' -delete 2>/dev/null || true
find "$MODULE" -name '*.pid' -delete 2>/dev/null || true

mkdir -p "$DIST"
rm -f "$ZIP"

echo "== packaging $ZIP"
if command -v python >/dev/null 2>&1 || command -v python3 >/dev/null 2>&1; then
    PYTHON=$(command -v python 2>/dev/null || command -v python3)
    if command -v cygpath >/dev/null 2>&1; then
        MODULE_ARG=$(cygpath -w "$MODULE")
        ZIP_ARG=$(cygpath -w "$ZIP")
        BUILD_ARG=$(cygpath -w "$ROOT/build")
    else
        MODULE_ARG="$MODULE"
        ZIP_ARG="$ZIP"
        BUILD_ARG="$ROOT/build"
    fi
    "$PYTHON" - "$MODULE_ARG" "$ZIP_ARG" "$BUILD_ARG" <<'PY'
import hashlib
import os
import stat
import sys
import zipfile

root, out, build_dir = sys.argv[1], sys.argv[2], sys.argv[3]
skip_dirs = {"run", ".git", "__pycache__"}
skip_ext = (".log", ".pid", ".tmp", ".lock", ".sh~")

MODE = {
    "files/fp_hal": 0o755,
    "post-fs-data.sh": 0o755,
    "service.sh": 0o755,
    "uninstall.sh": 0o755,
    "module.prop": 0o644,
    "sepolicy.rule": 0o644,
    "sepolicy.rule.ksu": 0o644,
    "customize.sh": 0o755,
    "vintf/fingerprint_aidl.xml": 0o644,
}

# The firmware revision is a build input, not a guess: the payload must be the
# binary that was just built, or the shipped module is not the module that was
# tested (review F-001).
def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as fh:
        for chunk in iter(lambda: fh.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


for name in ("fp_hal",):
    built = os.path.join(build_dir, name)
    staged = os.path.join(root, "files", name)
    if not os.path.isfile(built):
        raise SystemExit("error: build/%s missing; run build.sh first" % name)
    if not os.path.isfile(staged) or sha256(built) != sha256(staged):
        raise SystemExit(
            "error: magisk_module/files/%s differs from build/%s; refusing to package"
            % (name, name)
        )

# 16 KB page compatibility: a PT_LOAD with p_align < 0x4000 cannot be loaded on
# a 16 KB page device, so shipping one would be a silent promise we break.
def elf_load_aligns(path):
    with open(path, "rb") as fh:
        data = fh.read()
    if data[:4] != b"\x7fELF" or data[4] != 2:
        return []
    phoff = int.from_bytes(data[32:40], "little")
    phentsize = int.from_bytes(data[54:56], "little")
    phnum = int.from_bytes(data[56:58], "little")
    aligns = []
    for i in range(phnum):
        off = phoff + i * phentsize
        if int.from_bytes(data[off:off + 4], "little") == 1:  # PT_LOAD
            aligns.append(int.from_bytes(data[off + 48:off + 56], "little"))
    return aligns


names = []
with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
    for dirpath, dirs, files in os.walk(root):
        dirs[:] = sorted(d for d in dirs if d not in skip_dirs)
        for name in sorted(files):
            if name.endswith(skip_ext):
                continue
            full = os.path.join(dirpath, name)
            rel = os.path.relpath(full, root).replace(os.sep, "/")
            mode = MODE.get(rel, 0o644)
            with open(full, "rb") as probe:
                is_elf = probe.read(4) == b"\x7fELF"
            if is_elf:
                for align in elf_load_aligns(full):
                    if align < 0x4000:
                        raise SystemExit(
                            "error: %s has PT_LOAD p_align=0x%x (<0x4000); "
                            "rebuild with -Wl,-z,max-page-size=16384" % (rel, align)
                        )
            info = zipfile.ZipInfo(rel, date_time=(2020, 1, 1, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            info.create_system = 3  # Unix: preserve external_attr
            info.external_attr = (stat.S_IFREG | mode) << 16
            with open(full, "rb") as source:
                archive.writestr(info, source.read())
            names.append(rel)

required = {"module.prop", "post-fs-data.sh", "service.sh", "uninstall.sh",
            "sepolicy.rule", "files/fp_hal", "vintf/fingerprint_aidl.xml"}
missing = required - set(names)
if missing:
    raise SystemExit("error: module payload missing: " + ", ".join(sorted(missing)))
bad = [n for n in names if "\\" in n]
if bad:
    raise SystemExit("error: archive contains backslash paths: " + ", ".join(bad))
print("== archive entries:", len(names))
for name in names:
    print("   ", name)
PY
elif command -v zip >/dev/null 2>&1; then
    ( cd "$MODULE" && zip -qr "$ZIP" . -x 'run/*' -x '*.log' -x '*.pid' -x '*~' )
else
    echo "error: neither python nor zip is available; cannot build a Magisk-compatible archive" >&2
    exit 1
fi

echo "== done"
ls -la "$ZIP"
echo
echo "sha256: $(sha256sum "$ZIP" | cut -d' ' -f1)"
ZIP_PRINT="$ZIP"
if command -v cygpath >/dev/null 2>&1; then
    ZIP_PRINT=$(cygpath -w "$ZIP")
fi
echo "Install with:  adb push \"$ZIP_PRINT\" /sdcard/ && adb shell magisk --install-module /sdcard/$(basename "$ZIP")"
echo "Then reboot."
