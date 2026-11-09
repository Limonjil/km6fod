#!/usr/bin/env bash
# patch_verify_probe.sh — собрать диагностический трамплин в libsl_fp_impl.so,
# который логирует РЕАЛЬНЫЙ код возврата silfp_impl_verify_enroll_challenge.
#
# Зачем: на сайте VA 0x3645c (файловое 0x2f45c) оригинал делал
#     tbnz w0, #0x1f, 0x364a4     ; любой отрицательный код => ветка ошибки
# и мы заменили это на NOP, потеряв различение «ложный отказ» / «честный отказ TA».
# Чтобы сузить патч, нужен фактический код, который возвращается на валидном HAT.
# Диспетчер silfp_cmd_verify_enroll_challenge уходит по `br` в слот 0xb8 глобальной
# таблицы, поэтому код виден только на живом вызове — отсюда трамплин.
#
# Почему компилятором: инструкции кодируются clang'ом, байты проверяются
# llvm-objdump'ом. Руками ARM64 не пишем — это источник ошибок.
#
# Использование (read-only по умолчанию, ничего не пишет без --apply):
#   tools/patch_verify_probe.sh <libsl_fp_impl.so> [--apply]
#
# Результат без --apply: отчёт (найденные пещеры, байты трамплина, проверка
# дизассемблером). С --apply: копия <lib>.probe с патчем и файл ожидаемых байтов
# для post-fs-data.sh.
set -euo pipefail

NDK="${NDK:-/c/Users/Limonjil/Desktop/android/android-ndk-r26d}"
HOST_TAG="windows-x86_64"
BIN="$NDK/toolchains/llvm/prebuilt/$HOST_TAG/bin"
CLANG="$BIN/aarch64-linux-android23-clang.cmd"
OBJCOPY="$BIN/llvm-objcopy.exe"
OBJDUMP="$BIN/llvm-objdump.exe"
READELF="$BIN/llvm-readelf.exe"

LIB="${1:?usage: $0 <libsl_fp_impl.so> [--apply]}"
APPLY="${2:-}"

# --- адрес сайта: файловое смещение -> VA -----------------------------------
# Первый LOAD этой библиотеки: off 0x0 -> VA 0x7000, поэтому VA = off + 0x7000.
SITE_OFF=$((0x2f45c))
SITE_VA=$((0x3645c))
RET_VA=$((0x36460))      # куда вернуться после проверки (adrp x8, ...)
ERR_VA=$((0x364a4))      # ветка ошибки оригинального tbnz

# --- 1. собрать трамплин -----------------------------------------------------
TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT
cat > "$TMP/probe.S" <<'ASM'
// r28rc_probe: сохраняет код возврата проверки, логирует его через PLT
// silfp_log_dump и возвращает управление в silfp_enroll_enroll (0x36460).
// Живёт в кодовой пещере той же библиотеки, поэтому PLT ей доступен.
    .text
    .global r28rc_probe
    .type   r28rc_probe, %function
r28rc_probe:
    // w0 = код возврата verify_enroll_challenge, x30 = адрес возврата в 0x36460
    // Сохраняем w0 в глобальную переменную библиотеки — её читает HAL по адресу
    // из dlsym("r28rc_value"), чтобы напечатать код в свой лог.
    adrp    x9, r28rc_value
    str     w0, [x9, :lo12:r28rc_value]
    ret
    .size   r28rc_probe, . - r28rc_probe

    .bss
    .global r28rc_value
    .type   r28rc_value, %object
    .size   r28rc_value, 4
r28rc_value:
    .word   0
ASM

"$CLANG" -target aarch64-linux-android23 -O2 -fPIC -c "$TMP/probe.S" -o "$TMP/probe.o"
"$OBJCOPY" -O binary --only-section=.text "$TMP/probe.o" "$TMP/probe.bin"
PROBE_SIZE=$(stat -c %s "$TMP/probe.bin")
echo "== трамплин собран: $PROBE_SIZE байт"
"$OBJDUMP" -d "$TMP/probe.o" | sed 's/^/   /'

# --- 2. найти кодовую пещеру -------------------------------------------------
# Пещера: непрерывный участок .text, где байты нулевые (выравнивание/padding).
python - "$LIB" "$PROBE_SIZE" <<'PY'
import sys
lib, need = sys.argv[1], int(sys.argv[2])
data = open(lib, 'rb').read()
# .text первого LOAD: файл 0x1000.., VA 0x7000.. — ищем нули в этом диапазоне
start, end = 0x1000, min(len(data), 0x5d874)
run = 0
best = []
i = start
while i < end:
    if data[i] == 0:
        run += 1
        if run == need:
            best.append(i - need + 1)
            run = 0
    else:
        run = 0
    i += 1
best = [b for b in best if b % 4 == 0]
print(f"== пещер (>= {need} байт, выровнены по 4): {len(best)}")
for b in best[:8]:
    print(f"   off 0x{b:x}  VA 0x{b + 0x7000:x}")
PY

if [ "$APPLY" != "--apply" ]; then
    echo
    echo "== dry-run. Чтобы применить: $0 $LIB --apply"
    exit 0
fi

echo "== --apply пока не реализован намеренно:"
echo "   сначала нужно подтвердить, что пещера не пересекается с релокациями/PLT"
echo "   ($READELF -r $LIB) и что трамплин не задел соседние функции."
exit 2
