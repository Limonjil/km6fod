#!/usr/bin/env bash
#
# Build the KM6 AIDL fingerprint HAL for arm64-v8a.
#
# Produces a fully self-contained PIE executable: the C++ runtime is linked
# statically so the binary only depends on libraries that always exist in the
# GSI (libbinder_ndk, liblog, libdl, libc, libm). That matters because the
# binary runs from /data/adb/modules, not from /vendor.
#
# Usage:  ./build.sh [--clean] [--verbose] [--debug] [--tools]
#
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
SRC="$ROOT/src"
GEN="$ROOT/third_party/aidl_gen"
OUT="$ROOT/build"
OBJ="$OUT/obj"

# --- toolchain ---------------------------------------------------------------
# NDK r26+ is required. Override with: NDK=/path/to/ndk ./build.sh
# The default is auto-detected because these trees get moved around.
detect_ndk() {
    for candidate in \
        "${NDK:-}" \
        "/c/Users/Limonjil/Desktop/android/android-ndk-r26d" \
        "/c/Users/Limonjil/Desktop/android-ndk-r26d" \
        "$HOME/android-ndk-r26d" \
        "$HOME/Android/Sdk/ndk/26.3.11579264" \
        "$HOME/Android/Sdk/ndk/27.0.12077973"; do
        if [ -n "$candidate" ] && [ -d "$candidate/toolchains" ]; then
            printf '%s\n' "$candidate"
            return 0
        fi
    done
    return 1
}

NDK="$(detect_ndk)" || {
    echo "error: Android NDK not found. Set NDK=/path/to/ndk" >&2
    exit 1
}
API="${API:-34}"

HOST_TAG="windows-x86_64"
case "$(uname -s)" in
    Linux*) HOST_TAG="linux-x86_64" ;;
    Darwin*) HOST_TAG="darwin-x86_64" ;;
esac

TOOLCHAIN="$NDK/toolchains/llvm/prebuilt/$HOST_TAG"
CC="$TOOLCHAIN/bin/aarch64-linux-android${API}-clang"
if [ -f "${CC}.cmd" ]; then
    CC="${CC}.cmd"
fi

if [ ! -x "$CC" ] && [ ! -f "$CC" ]; then
    echo "error: compiler not found at $CC" >&2
    echo "       set NDK to a valid Android NDK (r26+) directory" >&2
    exit 1
fi

CXX="${CC/clang/clang++}"
if [ -f "${CXX}.cmd" ]; then
    CXX="${CXX}.cmd"
fi
if [ ! -x "$CXX" ] && [ ! -f "$CXX" ]; then
    # The Windows wrappers are clang only; drive C++ through the driver flag.
    CXX="$CC"
    CXX_EXTRA="-x c++"
else
    CXX_EXTRA=""
fi

SYSROOT_LIB="$TOOLCHAIN/sysroot/usr/lib/aarch64-linux-android"
CXX_STATIC="$SYSROOT_LIB/libc++_static.a"
CXX_ABI="$SYSROOT_LIB/libc++abi.a"

# The NDK ships only a reduced 107-symbol stub of libbinder_ndk.so: no
# AServiceManager_*, ABinderProcess_* or AIBinder_markVintfStability. Those are
# exactly the entry points a HAL needs to publish itself, so linking uses the
# real library extracted from the target device instead.
SYSLIBS="$ROOT/third_party/syslibs/arm64-v8a"
if [ ! -f "$SYSLIBS/libbinder_ndk.so" ]; then
    echo "error: missing $SYSLIBS/libbinder_ndk.so" >&2
    echo "       pull it from the device: adb pull /system/lib64/libbinder_ndk.so" >&2
    exit 1
fi

VERBOSE=0
TOOLS=0
DEBUG=0
for arg in "$@"; do
    case "$arg" in
        --clean) rm -rf "$OUT"; echo "cleaned $OUT" ;;
        --verbose) VERBOSE=1 ;;
        --tools) TOOLS=1 ;;
        --debug) DEBUG=1 ;;
        *) echo "unknown option: $arg" >&2; exit 2 ;;
    esac
done

mkdir -p "$OBJ" "$OUT"

CXXFLAGS=(
    -std=c++17
    -fPIC
    -ffile-prefix-map=$ROOT=.
    -fPIE
    -pthread
    -DNDEBUG
    -fno-exceptions
    -fno-rtti
    -fstack-protector-strong
    -D_FORTIFY_SOURCE=2
    -ffunction-sections
    -fdata-sections
    -fno-omit-frame-pointer
    -Wall
    -Wextra
    -Wno-unused-parameter
    -Wno-deprecated-declarations
    -I"$GEN/include"
    -I"$GEN"
    -I"$SRC"
)
if [ "$DEBUG" = "1" ]; then
    CXXFLAGS+=(-O0 -g)
else
    CXXFLAGS+=(-O2)
fi

# -fno-exceptions is only safe if the AIDL stubs agree; they use ndk::ScopedAStatus
# rather than exceptions. libc++ static is built with exceptions enabled, which
# is still link-compatible.
echo "== toolchain: $CC (API $API)"

if command -v mapfile >/dev/null 2>&1; then
    mapfile -t SOURCES < <(find "$SRC" "$GEN" -name '*.cpp' | sort)
else
    SOURCES=($(find "$SRC" "$GEN" -name '*.cpp' | sort))
fi
echo "== sources: ${#SOURCES[@]}"

fail=0
compile_one() {
    local source="$1"
    local object="$OBJ/$(echo "${source#"$ROOT"/}" | tr '/' '_').o"
    local log="$object.log"
    if ! "$CXX" $CXX_EXTRA "${CXXFLAGS[@]}" -c "$source" -o "$object" 2>"$log"; then
        echo "!! FAILED: ${source#"$ROOT"/}"
        sed -n '1,25p' "$log" >&2
        return 1
    fi
    if [ "$VERBOSE" = "1" ] && [ -s "$log" ]; then
        sed -n '1,10p' "$log"
    fi
    return 0
}

# Compile in parallel; collect failures instead of aborting on the first one.
pids=()
running=0
# Cap the parallelism: with 16 jobs the NDK .cmd wrappers intermittently
# fail with "no such file or directory" on random sources (see DEBUG.md F22).
max_jobs="$(nproc 2>/dev/null || echo 4)"
[ "$max_jobs" -gt 4 ] && max_jobs=4
for source in "${SOURCES[@]}"; do
    compile_one "$source" &
    pids+=($!)
    running=$((running + 1))
    if [ "$running" -ge "$max_jobs" ]; then
        wait "${pids[0]}" || fail=1
        pids=("${pids[@]:1}")
        running=$((running - 1))
    fi
done
for pid in "${pids[@]:-}"; do
    [ -n "$pid" ] || continue
    wait "$pid" || fail=1
done

if [ "$fail" -ne 0 ]; then
    echo "== build FAILED" >&2
    exit 1
fi

echo "== linking"
shopt -s nullglob
OBJECTS=("$OBJ"/*.o)
"$CXX" -pie -pthread -o "$OUT/fp_hal" "${OBJECTS[@]}" \
    -nostdlib++ -Wl,--start-group "$CXX_STATIC" "$CXX_ABI" -Wl,--end-group \
    -Wl,--gc-sections \
    -Wl,--no-undefined \
    -Wl,-z,relro,-z,now,-z,noexecstack \
    -Wl,-z,max-page-size=16384 -Wl,--build-id=sha1 \
    -L"$SYSLIBS" -lbinder_ndk -llog -ldl -lm

echo "== done: $OUT/fp_hal"
ls -la "$OUT/fp_hal"

READELF="$TOOLCHAIN/bin/llvm-readelf"
if [ -x "${READELF}.exe" ]; then READELF="${READELF}.exe"; fi
if [ -x "$READELF" ]; then
    echo "== dynamic dependencies"
    NEEDED=$("$READELF" -d "$OUT/fp_hal" | sed -n 's/.*Shared library: \[\(.*\)\]/\1/p')
    echo "$NEEDED"
    for lib in $NEEDED; do
        case "$lib" in
            libbinder_ndk.so|liblog.so|libdl.so|libm.so|libc.so) ;;
            *)
                echo "error: unexpected dependency $lib (libc++_shared must not leak)" >&2
                exit 1
                ;;
        esac
    done
else
    echo "warning: llvm-readelf not found, dependency check skipped"
fi

if command -v file >/dev/null 2>&1; then
    file "$OUT/fp_hal"
fi

# The FOD wake gesture helper is shipped in the module, so it is built here and
# nothing else may supply it: a prebuilt copy in the payload was 4 KB aligned and
# silently failed the executable-bit check (review F-002, F-021).
echo "== linking trantp_probe"
"$CXX" -std=c++17 -O2 -fPIE -pie -pthread -fno-exceptions -fno-rtti \
    -I"$GEN/include" -I"$GEN" \
    -o "$OUT/trantp_probe" tools/trantp_probe.cpp \
    -nostdlib++ -Wl,--start-group "$CXX_STATIC" "$CXX_ABI" -Wl,--end-group \
    -Wl,--gc-sections -Wl,-z,relro,-z,now,-z,noexecstack \
    -Wl,-z,max-page-size=16384 -Wl,--build-id=sha1 \
    -L"$SYSLIBS" -lbinder_ndk -llog -ldl -lm
echo "== done: $OUT/trantp_probe"
ls -la "$OUT/trantp_probe"

# Optional diagnostic client. It is never packaged into the Magisk module.
if [ "$TOOLS" = "1" ]; then
    echo "== linking diagnostic client"
    GEN_OBJECTS=("$OBJ"/third_party_aidl_gen_*.o)
    "$CXX" -std=c++17 -O2 -fPIE -pie -pthread -fno-exceptions -fno-rtti \
        -I"$GEN/include" -I"$GEN" \
        -o "$OUT/fp_test" tools/fp_test.cpp "${GEN_OBJECTS[@]}" \
        -nostdlib++ -Wl,--start-group "$CXX_STATIC" "$CXX_ABI" -Wl,--end-group \
        -L"$SYSLIBS" -lbinder_ndk -llog -ldl -lm
    echo "== done: $OUT/fp_test"
    ls -la "$OUT/fp_test"
fi
