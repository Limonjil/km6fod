# STATE — km6fod (сессия 09.11, вечер)

Читать это, а не старый чат. Ниже только то, что нужно для продолжения.

## Где мы находимся

Модуль `km6fod` **v1.7.0** (versionCode 17000, автор limonjil, GPL-3.0-only). Главное
изменение: **удержание пальца на полностью тёмном экране работает** — дисплей выключен,
тач продолжает отдавать обычные MT-события, HAL будит панель, включает HBM и
разблокирует без второго касания. Панель и яркость (`km6fix`) не трогали.

Последняя правка HAL — закрытие хвоста ревью (H-12, H-23, H-31, H-33) плюс вторая
фреймворка, `open()` съедал его целиком, пустой список уходил как авторитетный и
фреймворк стирал регистрацию. Теперь окно перевзводится после `engine.enumerate()`, а
уведомления движка записываются даже вне окна.
Третья правка того же дня — id шаблона больше не схлопывается в 0: движок отдаёт
отрицательные id (`-4293707`, `-6827783`), старый код отдавал фреймворку `0` для любого из
них, из-за чего два шаблона назывались одинаково, `remove(0)` не находил ничего в TA и
палец нельзя было ни удалить, ни записать заново. Теперь `frameworkEnrollmentId()` снимает
знаковый бит, `engineEnrollmentId()` возвращает его перед вызовом движка. На устройстве
проверено: cleanup удалил оба фантома (`remove(2143189941) rc=0`, `remove(2140655865) rc=0`),
запись пальца прошла, разблокировка работает (`accept: 7`).
Сборка `7407be7d` **работает на устройстве** (bind-mount пересоздан перезагрузкой 09.11,
10:52) и лежит в `magisk_module/files/fp_hal` + `dist/km6fod-v1.7.0.zip`
(sha256 `f063459a…`).
Открытый хвост: движок всё ещё перечисляет один шаблон, которого нет в базе фреймворка
(мусор от старых сборок). Он невидим фреймворку, разблокировки по нему нет, следующая
загрузка снесёт его тем же cleanup. Если он переживёт ещё один `remove rc=0` — добавить в
`removeEnrollments` проверку «перечислить после удаления и повторить». Ещё открыто:
разблокировка на тёмном экране (на 09.11 контроллер не отдаёт событий после гашения,
`getevent` на `event5` пуст; владелец говорит, что починил это в другой сессии — здесь не
перепроверялось).

## Устройство

- Tecno KM6 (Spark 40 Pro), MT6789H, GSI Android 15, Magisk 31.1, SELinux enforcing, 4 KB.
- Тач FT3683G (`adaptive-ts.ko` + `focaltech_ft3683g.ko`), сенсор Silead GSL7002,
  FOD-зона (560, 2205) r≈100, экран 1080×2400.
- HAL: `android.hardware.biometrics.fingerprint@2.1-service-mediatek`, код в `src/`.
- git в проекте **есть** (`C:/Users/Limonjil/Desktop/2/.git`); бинарники всё равно
  копировать в `build/archive/` перед заменой.
- После ребута `adb` = shell, не root: push только в `/data/local/tmp`, дальше `su -c cp`.

## Что сделано в этой сессии (проверено на устройстве)

- **Реверс панели/тача** (было задачей №1): разобраны `mediatek-drm.ko`, `leds-mtk.ko`,
  `adaptive-ts.ko`, `focaltech_ft3683g.ko`; модули вынуты из `vendor_boot` (LZ4-legacy).
  Ответы на 5 вопросов брифа — в `PANEL-RE-LEDGER.md`.
- **Патч тач-модулей** (4 байта в двух файлах): `focaltech` не уходит в firmware-gesture
  и не ставит флаг suspend (`0x1c80`, `0x90bc`, `0x1db0`), `adaptive-ts` не шлёт свой
  suspend-вендор-вызов (`0x88d0`). Контроллер продолжает сканировать при выключенном экране.
- **`post-fs-data.sh`, раздел 1c2**: проверка sha256 вендорских копий, `rmmod` focaltech →
  adaptive (focaltech держит adaptive), `insmod` патченных из `files/`, откат на вендорские
  при любом несовпадении; всё пишется в boot-лог. Проверено ребутом.
- **HAL**: пробуждение панели — вендорский kernel-wakelock (`VendorEngine::wakeSystem()`,
  `tran_fp -> do_wakeup_set`). Инъекции клавиш в HAL больше нет: `uinput`/`KEY_WAKEUP`
  удалены (они же запускали камеру и оставляли зомби). HBM включается только после
  осветления панели (`waitForSettledPanel`, 180 мс после появления подсветки); при
  истечении ожидания захват всё равно делается, если палец ещё лежит; ожидание
  отменяется после успеха; ветка «разбудить» работает только при тёмной панели.
- **Ожидания на таймерах** (H-12/H-33): ни один блокирующий сон не остался на
  SerialWorker — подсветка (одна попытка + один ретрай через 80 мс), пауза перед
  телом FOD-нажатия, тишина enumerate и settle калибровки перед `preEnroll` идут
  через `DeadlineScheduler`. `open()` движка — машина состояний с ожиданием на
  `condition_variable` (H-23). `stop()` HAL сливает очереди dispatcher → timers →
  worker, поставленные задачи не теряются (H-31).
- **Версия/лицензия**: `module.prop` v1.7.0, `license=GPL-3.0-only`, SPDX во всех
  исходниках, `LICENSE`, `NOTICE.md` (в т.ч. честно указано, что два патченных
  вендорских `.ko` едут в модуле).
- Удалён стухший `files/fp_hal.new` (post-fs-data ставил бы его поверх рабочего HAL).
- `dist/km6fod-v1.7.0.zip` — sha256 `a5660ecae9091c24637d5c993fd15da80d1f025a823dc9b7f870290b6422fdf2`
  (сборка HAL `949413d3`).

## Открытые задачи

1. **Проверить затуп** после `6c5e1982`: вспышка → разблокировка без паузы? Если осталась
   пауза ~1.2–1.5 с, это ожидание иконки (`overlayTick`, 1500 мс) — уменьшать осторожно.
2. **`PANEL-RE-FINDINGS.md` не написан** (ответы на 5 вопросов + доказательства; ledger есть).
3. **Юридический хвост**: патченные `.ko` — проприетарные вендорские бинарники, сейчас едут
   в zip. Правильнее патчить копии устройства при загрузке (как `libsl_fp_impl.so`), не возить свои.
4. **Замеры питания**: базовый A/B (обычный сон vs новое поведение, по 5 минут) не снят.
5. Ревью `review/20261105-hard` **закрыто**: открытых находок нет (`STATUS.md`).
   Осталась проверка на устройстве после перезагрузки: запись 20/20, разблокировка
   локскрина и тёмного экрана, `HAL deaths` = 0, отсутствие миганий.
6. security-аудитор падал на 429 — перезапустить (`review/20261105-2340/PLAN.md`).

## Проверенные факты, чтобы не повторять тесты

- `echo 0` в узел подсветки обратим **только на включённом экране** (723→0→300→723);
  на спящем — игнорируется: `mtk_drm_setbacklight` выходит по `ctx+0x678==0`.
- `cmd display set-brightness 0` зажимается до `bl=16`.
- `led_mode=4` — DT-свойство, узел 0444 без store; на поведение не влияет.
- HBM гейтится `lcm[9] && ctx+0xdc4 && !ctx+0xdc5` (`0xdc4` — DSI включён, `0xdc5` — панель
  выключена); строка «display state is slept or idle, skip set hbm node» — отсюда.
- `mtk_drm_aod_setbacklight` при `ctx+0x678!=0` и `lcm[0x210]==0` возвращает `-EINVAL`
  («crtc is on and not in doze mode»).
- Тач уходит в `TRAN_SUSPEND_MODE` через ~45 тиков после DRM blank (delayed work).
- Жест id 36 → `0xcc` → `KEY_F13 (183)`; экран не будит — **ветка жестов закрыта владельцем**,
  обычный путь идёт через MT-события тача.
- `input keyevent` из HAL не работает (`Failed transaction (2147483646)`), `uinput`-инъекция
  работала, но запускала камеру и плодила зомби — удалена. Будит вендорский wakelock
  (`do_wakeup_set`), ключгард SystemUI поднимается сам по `displayState=LOCKSCREEN`.
- `displayState` (`LOCKSCREEN=1`, `NO_UI=2`, `AOD=4`) — состояние UI, не питание.
- Ребут обязателен для: ядра/модулей, `post-fs-data`, VINTF, `system/`.

## Файлы, которые менялись

`src/session.cpp`, `src/session.h`, `src/hbm.cpp`, `src/hbm.h`, `src/main.cpp`,
`src/touch_reader.cpp`, `src/vendor_engine.cpp`, `src/paths.h`, `src/sensor_geometry.h`;
`magisk_module/{module.prop,post-fs-data.sh,service.sh,uninstall.sh}`,
`magisk_module/files/{fp_hal,adaptive-ts.ko,focaltech_ft3683g.ko}`;
`NOTICE.md`, `LICENSE`, `deploy_hal.sh`, `touch_reload.sh`, `hbm_watch.sh`, `PANEL-RE-LEDGER.md`.

## Как продолжить

- Код: `bash build.sh`, тесты `bash tests/patch_site.sh && bash tests/aidl_version.sh`.
- Выкладка HAL: `adb push build/fp_hal /data/local/tmp/fp_hal.new &&
  adb shell 'su -c "cp /data/local/tmp/fp_hal.new /data/adb/modules/km6fod/files/fp_hal.new &&
  sh /data/local/tmp/deploy_hal.sh"'` (скрипт сам делает стек-bind и убивает HAL).
- Тач без ребута: `touch_reload.sh` (rmmod focaltech → adaptive, insmod из модуля).
- Упаковка: `bash build_module.sh` → `dist/km6fod-v1.7.0.zip`.
- Разбор драйверов: `analysis/kmod/disas.sh <ko> <outdir> <sym…>`, дампы в `analysis/dis2/`.
- Логи этой сессии: `debug-evidence/` (dmesg, logcat, `ev2.log`).
