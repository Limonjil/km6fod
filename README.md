# KM6 FOD

[![License: GPL-3.0](https://img.shields.io/badge/license-GPL--3.0-blue.svg)](LICENSE)
[![Release](https://img.shields.io/badge/release-v1.9.0-green.svg)](../../releases)
![Android 15 GSI](https://img.shields.io/badge/android-15%20GSI-lightgrey.svg)

Подэкранный отпечаток для **Tecno Spark 40 Pro (KM6, MT6789H)** на GSI.
Under-display fingerprint for the **Tecno Spark 40 Pro (KM6, MT6789H)** on GSI.

[Русский](#русский) · [English](#english)

---

## Русский

### Что это

Модуль Magisk, который возвращает нормальное удержание пальца на подэкранном
датчике **Silead GSL7002** под GSI: экран полностью погашен, палец прижат —
панель просыпается, вспышка включается только после того, как панель
стабилизировалась, и телефон разблокируется без второго касания.

Внутри — собственный AIDL HAL (`android.hardware.biometrics.fingerprint` v5),
VINTF-фрагмент, RRO для SystemUI и политика SELinux. Ничего не прошивается в
разделы, всё снимается удалением модуля и перезагрузкой.

### Что было сломано

1. **Движок не открывал затвор.** Вендорская библиотека требует uevent
   `TRAN_FULL_HBM_EVENT=FULL_HBM_SET`; на стоке его поднимает `tran_fp.ko`, под
   GSI не поднимает никто. HAL инжектит событие в netlink сам.
2. **На погашенном экране не было тача.** Драйвер тача уводит контроллер в
   gesture-режим; MT-события исчезают. Модуль грузит **собственные** модули
   устройства (`adaptive-ts.ko`, `focaltech_ft3683g.ko`) с изменёнными байтами,
   предварительно сверив sha256 стока; при несовпадении остаются vendor-копии.
3. **Пробуждение не открывало экран.** Wakelock поднимает SoC, но SystemUI не
   создаёт операцию `authenticate`. HAL владеет виртуальным устройством
   `/dev/uinput` и нажимает `KEY_WAKEUP`.
4. **Телефон не просыпался от пальца после глубокого сна.** Патч
   `fts_pm_suspend → fts_gesture_suspend` армирует линию `mtk-eint 9` как
   wake-источник, не отправляя контроллеру sleep-команду.
5. **Полоса при разблокировке.** HBM включается через 180 мс после того, как
   подсветка перестала быть нулевой, — иначе панель переключает FPS на лету.

### Требования

- Tecno Spark 40 Pro (KM6), панель `vtdr6126a_fhdp_dsi_vdo_dsc_tm_tm_144hz_km6`;
- Android 15 GSI (проверено на LineageOS GSI), arm64;
- Magisk 27+ (проверено на 31.1);
- экран 4K-страниц; 16K не проверялось;
- других модулей, публикующих `IFingerprint/default`, быть не должно.

### Установка

```bash
adb push dist/km6fod-v1.9.0.zip /sdcard/
adb shell su -c "magisk --install-module /sdcard/km6fod-v1.9.0.zip"
adb reboot
```

Лог: `/data/adb/km6fod.log`. Удаление: `magisk --remove-module km6fod` + ребут.

### Сборка

```bash
./build.sh              # -> build/fp_hal
./build_module.sh       # -> dist/km6fod-v1.9.0.zip
```

Нужен Android NDK r26+ (`NDK=/path/to/ndk`) и Git Bash/WSL. Сборка проверяет,
что `files/fp_hal` побайтно совпадает с собранным бинарником, а все ELF выровнены
под 16K-страницы.

### Структура

```
src/                 AIDL HAL: сессии, вендорский мост, HBM, uevent, evdev
magisk_module/       payload: скрипты, VINTF, sepolicy, RRO, HAL, патчи модулей тача
overlay/             исходники RRO (SystemUI UDFPS, product)
tools/               диагностика: fp_test, uevent_probe, namespace_probe
tests/               проверки патч-сайтов и AIDL-версии
third_party/         сгенерированные AIDL-стабы и линкуемые syslibs
FOD_ARCHITECTURE_SPEC.md   как устроен вендорский движок и HAL
```

### Ограничения

- Только этот девайс и эта панель: смещения вендорского vtable и геометрия
  датчика зафиксированы.
- `.ko` в модуле — вендорские бинарники с изменёнными байтами, они не под
  GPL; см. `NOTICE.md`.
- KernelSU и APatch не проверены на устройстве; Android 16/17 собран, но не
  запускался.

### Лицензия

Код — **GPL-3.0-only**, см. `LICENSE`. Вендорские бинарники в
`magisk_module/files/` — собственность производителя, распространяются как есть.

---

## English

### What it is

A Magisk module that restores a normal press-and-hold on the under-display
**Silead GSL7002** sensor under a GSI: the screen is fully off, the finger is
already down — the panel wakes, HBM opens only after the panel settles, and the
phone unlocks without a second tap.

It ships its own AIDL HAL (`android.hardware.biometrics.fingerprint` v5), the
VINTF fragment, a SystemUI RRO and SELinux rules. Nothing is written to raw
block devices; removing the module and rebooting restores everything.

### What was broken

1. **The engine never opened its gate.** The vendor library waits for
   `TRAN_FULL_HBM_EVENT=FULL_HBM_SET`; on stock firmware `tran_fp.ko` raises it,
   under a GSI nobody does. The HAL injects the uevent itself.
2. **No touch while the screen was off.** The driver parks the controller in
   gesture mode and multitouch disappears. The module loads the device's **own**
   `adaptive-ts.ko` and `focaltech_ft3683g.ko` with a few bytes changed, after
   checking their stock sha256 and with a fallback to the vendor copies.
3. **Waking the SoC did not light the display.** A wakelock raises the CPU, but
   SystemUI never creates the `authenticate` operation. The HAL owns a virtual
   `/dev/uinput` device and presses `KEY_WAKEUP`.
4. **The phone did not wake from a finger after deep sleep.** A patch of
   `fts_pm_suspend → fts_gesture_suspend` arms `mtk-eint 9` as a wake source
   without sending the controller its sleep command.
5. **A bright band on unlock.** HBM is armed 180 ms after the backlight stopped
   being zero; earlier it catches the panel's FPS switch.

### Requirements

- Tecno Spark 40 Pro (KM6), panel `vtdr6126a_fhdp_dsi_vdo_dsc_tm_tm_144hz_km6`;
- Android 15 GSI (verified on LineageOS GSI), arm64;
- Magisk 27+ (verified on 31.1);
- 4K pages; 16K untested;
- no other module may publish `IFingerprint/default`.

### Install

```bash
adb push dist/km6fod-v1.9.0.zip /sdcard/
adb shell su -c "magisk --install-module /sdcard/km6fod-v1.9.0.zip"
adb reboot
```

Log: `/data/adb/km6fod.log`. Uninstall: `magisk --remove-module km6fod` + reboot.

### Build

```bash
./build.sh              # -> build/fp_hal
./build_module.sh       # -> dist/km6fod-v1.9.0.zip
```

Android NDK r26+ (`NDK=/path/to/ndk`) and Git Bash/WSL. The packaging step
verifies that `files/fp_hal` matches the built binary byte for byte and that
every shipped ELF is 16K-page aligned.

### Layout

```
src/                 AIDL HAL: sessions, vendor bridge, HBM, uevent, evdev
magisk_module/       payload: scripts, VINTF, sepolicy, RROs, HAL, touch patches
overlay/             RRO sources (SystemUI UDFPS, product)
tools/               diagnostics: fp_test, uevent_probe, namespace_probe
tests/               patch-site and AIDL-version checks
third_party/         generated AIDL stubs and syslibs used for linking
FOD_ARCHITECTURE_SPEC.md   how the vendor engine and the HAL fit together
```

### Limitations

- This device and panel only: vendor vtable offsets and sensor geometry are
  pinned.
- The `.ko` files are vendor binaries with changed bytes and are not GPL; see
  `NOTICE.md`.
- KernelSU and APatch are not device-verified; Android 16/17 is built for but
  was never booted.

### License

Code is **GPL-3.0-only**, see `LICENSE`. The vendor binaries in
`magisk_module/files/` belong to their manufacturer and are redistributed
as-is.
