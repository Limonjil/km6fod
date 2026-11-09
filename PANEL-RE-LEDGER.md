# PANEL-RE-LEDGER

Формат: гипотеза → тест → результат → вердикт.
Метки: наблюдение, гипотеза, доказанная причина.

## Состояние устройства

- S1 (наблюдение): `kernel.kptr_restrict` был поднят в 0 ранее для чтения адресов модулей; возвращён в 2 в рамках этого этапа. Других изменений на устройстве нет.
- S2 (наблюдение): `/proc/gesture_function` при чтении отдаёт не конфиг, а последний 4-байтовый пакет жеста (`00 0a`). Конфиг читается из `/proc/gesture_state` (формат `%c%c : %d`).
- S3 (наблюдение): сейчас `cc : 1`, `fp : 1`; остальные жесты 0.
- S4 (действие): `echo cc1 > /proc/gesture_function` выполнен (rc=0), `/proc/gesture_state` подтвердил `cc : 1`. Обратимо: `echo cc0 > /proc/gesture_function`.

## L1. `led_mode = 4` не даёт выставить яркость 0

- Тест: дизассемблер `leds-mtk.ko` (`.data+0xe0`, `led_mode_show`, `mt_leds_parse_dt`), дампы в `analysis/dis2/`.
- Результат: атрибут создан с mode `0444`, `store = NULL` (нет релокации вовсе). Значение 4 — DT-свойство `led_mode`, прочитанное один раз в `mt_leds_parse_dt` в `drvdata+0x1ac`; больше это поле нигде не читается и не пишется.
- Вердикт: гипотеза опровергнута. `led_mode` — информационный read-only атрибут, на яркость не влияет.

## L2. Запись яркости игнорируется из-за гейта «crtc not enabled»

- Тест: дизассемблер `mtk_drm_setbacklight` (`0x4c388`), гейт `ldrb w8,[x20,#0x678]; cbz w8, 0x4c5cc`.
- Результат: при `ctx+0x678 == 0` уровень не программируется вообще, печатается «Sleep State set backlight stop --crtc not ebable». `ctx+0x678` — флаг включённого CRTC; ставится в `mtk_crtc_set_status`, `mtk_drm_crtc_enable`, `mtk_drm_crtc_first_enable`, снимается в `mtk_drm_crtc_disable`.
- Вердикт: доказанная причина игнорирования записи в спящем экране.

## L3. HBM-гейт — это выключенный DSI

- Тест: дизассемблер `tran_lcm_hbm_set` (`0xa1a1c`) плюс все обращения к `ctx+0xdc4/0xdc5` в `mediatek-drm.ko`.
- Результат: условие `lcm->[9] != 0 && ctx+0xdc4 != 0 && ctx+0xdc5 == 0`. `0xdc4 = 1` ставится в `mtk_dsi_probe` после `phy_power_on` и включения клоков DSI; в `mtk_output_dsi_disable` — `0xdc4 = 0`, `0xdc5 = аргумент`.
- Вердикт: доказанная причина сообщения «display state is slept or idle, skip set hbm node»: HBM нельзя включить при выключенном DSI.

## L4. `lcm_doze_backlight` — управляемый уровень AOD

- Тест: дизассемблер `lcm_doze_backlight_show` (`0x11db94`) и `mtk_drm_aod_setbacklight` (`0x4d338`).
- Результат: show только печатает статический массив `3;2;11;22;` (4 слова из `.bss`), сеттера нет. `mtk_drm_aod_setbacklight` при `crtc on && !doze` возвращает `-EINVAL` с логом «crtc is on and not in doze mode»; иначе берёт wakelock `hold`, включает top-клоки и cmdq и программирует AOD-подсветку.
- Вердикт: гипотеза «управляемый уровень AOD из userspace» опровергнута. AOD-путь существует, но требует crtc off/idle, то есть недостижим на этой GSI, где doze не работает.

## L5. Тач уходит в suspend по собственному таймеру

- Тест: дизассемблер `nvt_disp_notifier_callback` (`0x7690`) и `state_change_machine` (`0x6b0`).
- Результат: blank (`data->event == 1`) → `cancel_work_sync` → `queue_delayed_work(..., 0x2d)` → `state_change_machine` → `TRAN_GESTURE_SUSPEND_MODE`; unblank (`event == 2`) → `queue_work(dsp_event_callback)` → `TRAN_ACTIVE_MODE`.
- Вердикт: гипотеза опровергнута в части «свой таймер»: задержка ~45 тиков — это отложенный work от DRM-нотификатора, но инициатор — именно blank/unblank.

## L6. Жест 0xcc отклоняется всегда

- Тест: `GestureIdTranslate` (`0x1eb8`) и таблица `gesture_maps` (stride 0xc, код в `+4`, enable в `+3`, флаг в `+8`); конфиг — `/proc/gesture_state`.
- Результат: для записи разрешение = (`entry[8] == 2 && bss+0x1a4 == 1`) либо `entry[3] != 0`. `entry[3]` — per-gesture enable из `gesture_data_write` (`echo cc1 > /proc/gesture_function`). Сейчас `cc : 1`.
- Вердикт: живой тест T1: событие не пришло вообще. Статически отказ возможен только при `entry[3] == 0`; на практике FW не отдал жест.

## L7. Жест 0xcc не будит систему

- Тест: путь разрешённого жеста в `GestureIdTranslate` + строка `pm_wakeup_dev_event` в `focaltech_ft3683g.ko`.
- Результат: при разрешении — `input_event(EV_KEY, 183, 1)`, SYN, `input_event(EV_KEY, 183, 0)`, SYN на `mtk-tpd`; драйвер FT вызывает `device_init_wakeup` и `pm_wakeup_dev_event`.
- Вердикт: живой тест T1: key-событие доставлено при полностью выключенном экране, но `KEY_F13` не будит (`mScreenState=OFF`). Патч на `KEY_WAKEUP` (0x8f) не делаем — владелец закрыл ветку.

## L8. Ветка отказа в текущей сборке мертва

- Тест: `llvm-readelf -r` и grep по полному дизассемблеру на `0x1bcf`.
- Результат: строка-дубль `Sorry, this gesture has been disabled.` (`0x1bcf`) не имеет ни одной релокации; рабочая формат-строка — `0x1baf` (`[tran_touch-INFO]%s %d:[GESTURE]Sorry, ...`).
- Вердикт: дубль мёртв, сам отказ жив. Исторический лог был без префикса `[GESTURE]`.

## T1 (план). Живой тест жеста 0xcc

- Одна переменная: включённый `cc` (уже включён, `cc : 1`).
- Предсказание: экран выключен, касание FOD-зоны → в dmesg `[GESTURE]gesture report KEY_F13 done !`, экран просыпается; MT-событий нет.
- Отрицание: если dmesg даёт `[GESTURE]Sorry, ...` — гейт `entry[8]/bss+0x1a4`, а не enable.
- Результат: IRQ 76 140530 → 140538; `fts_gesture_buf` → `Gesture ID:36`, `PointNum:2`; dmesg: `fts_gesture_readdata gesture_id=36, point_num=2` → `g_gtp_gesture=0xcc` → `gesture report KEY_F13 done !`. Экран остался `OFF` (`mScreenState=OFF`, `mWakefulness=Dozing`).
- Вердикт: жест доставляется при полностью выключенном экране; не будит только потому, что код клавиши 183 (`KEY_F13`), а не `KEY_WAKEUP`. Механизм жестов к «честному удержанию» отношения не имеет: это короткий тап, а не удержание.
