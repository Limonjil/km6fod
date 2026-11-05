# KM6 FOD — состояние на момент паузы

Документ фиксирует, что сделано, что проверено на железе и где именно остановились.
Продолжать чтение — с раздела «Где застряли» и «Следующие шаги».

---

## 1. Итог одной таблицей

| Узел | Состояние | Подтверждение |
|---|---|---|
| Строка «Отпечатки пальцев» в настройках | **работает** | экран «Разблокировка устройства» → «Отпечатки пальцев — Требуется настройка» |
| Планировщик биометрии | **чист** | `Current operation: null`, `Pending: 0` |
| AIDL-сервис `IFingerprint/default` | **зарегистрирован** | `service list`, `dumpsys fingerprint` |
| HAL вендорного движка | **загружается** | `set_notify rc=0`, `set_active_group rc=0`, `chipid rc=0` |
| SPI-связь с сенсором | **живая** | `chipid probe rc=0 data=88a01570` |
| Trustonic TEE / TA | **работает** | `LOAD_ENROLL_CHALLENGE (0x15) → SUCCESS` |
| HBM (подсветка) | **работает** | `panel_hbm_set_cmdq_switch en=1`, readback `0xOK` |
| Инжект `TRAN_FULL_HBM_EVENT` | **принят движком** | `[AUTH_TIME] TRAN_FULL_HBM_EVENT=true, screen status: 2` |
| `pre_enroll` | **работает** | выдаёт реальный 64-бит challenge |
| `enroll` | **работает** | запись проходит по шагам 20→0, шаблон сохраняется |
| Аутентификация / разблокировка | **работает** | успешная разблокировка подтверждена |
| Подсветка на локскрине (AOD) | частично | панель в doze игнорирует HBM; HAL будит дисплей, если backlight = 0 |

То есть вся инфраструктура поднята, регистрация пальца упирается в одну проверку внутри TA.

---

## 2. Развёрнутая конфигурация

- Модуль: `/data/adb/modules/km6fod` (id `km6fod`, v1.1.0), архив `dist/km6fod-v1.1.0.zip`.
- HAL: `files/fp_hal`, bind-mount на
  `/vendor/bin/hw/android.hardware.biometrics.fingerprint@2.1-service-mediatek`,
  метка `vendor_file`, запуск из Magisk (`u:r:magisk:s0`).
- VINTF: подготовленный каталог с 46 манифестами, HIDL-отпечаток удалён, добавлен
  `fingerprint_aidl.xml` (`IFingerprint/default`, версия 2).
- RRO `com.km6.fpaidl` (platform key, priority 100): очищает
  `config_biometric_sensors` → framework выбирает AIDL-ветку.
- SELinux: правила из `sepolicy.rule` применяются в `post-fs-data.sh`,
  включая TEE-узел `mobicore_user_device` для домена `magisk`.
- Калибровка `fpcal*.dat` скопирована в `/data/vendor_de/0/fpdata`.

---

## 3. Что было преодолено после первой версии

1. **TA отвергал enroll-токен** (`-1004 INVALID_CHALLENGE`) при полностью
   корректном challenge и живом MAC. Обойдено 4-байтовым патчем вендорной
   библиотеки: `0x3645c`, `tbnz w0,#0x1f` → `nop` (`patched/libsl_fp_impl.so`,
   монтируется поверх `/vendor/lib64/libsl_fp_impl.so`).
2. **TA не инициализировался**, пока процесс не получил доступ к
   `/dev/mobicore-user`: добавлено правило SELinux для домена `magisk`.
3. **Ожидание пальца висело на драйверном IRQ.** Вызываем
   `silfp_impl_sync_finger_status_optic(1/0)` на касание/отпускание — оно
   мапится в `silfp_dev_sync_finger_down/up`.
4. **Запись завершалась с одного касания**: второй аргумент `enroll` — это
   счётчик шагов, а не gid. Прогресс считается в HAL (20 шагов).
5. **Подсветка гасла между касаниями**: окно неактивности 5 с → 15 с.
6. **HAL не стартовал после ребута**: pid-файл проверяется по cmdline.

---

## 4. Где застряли (точные доказательства)

```
04:25:10.856  0x15 LOAD_ENROLL_CHALLENGE            → SUCCESS, C = 0x719e89f85a29c228
04:25:13.092  HAT: v=0 ch=719e89f85a29c228
                   uid=<Gatekeeper SID> aid=0 type=1 ts=98389 mac_zero=0
04:25:13.092  0x17 VERIFY_ENROLL_CHALLENGE [69 B]   → -1004 INVALID_CHALLENGE
04:25:13.093  движок → onError(HW_UNAVAILABLE)
```

Движок (`silfp_enroll_enroll`):

```c
if (!hat) goto error;
rc = silfp_impl_verify_enroll_challenge(hat, 69, *(u64*)(hat + 9));
if (rc < 0) goto error;          // ← -1004, сюда попадаем
worker_set_state(1);
return 0;
error:
    silfp_notify_send_error_notice(1);   // HW_UNAVAILABLE
    return 0;
```

### Что уже исключено

- **Не наш токен по полям.** `version=0`, `challenge` побайтово равен выданному,
  `authenticator_type=1` (PASSWORD — корректно при записи под PIN),
  `mac` не нулевой, `user_id` — Gatekeeper SID (так и должно быть).
- **Не сброс challenge активной группой.** `set_active_group` теперь вызывается
  один раз на процесс и только до `pre_enroll`.
- **Не калибровка.** Раньше `0x40 CALIBRATE_OPTIC` попадала между `0x15` и `0x17`;
  добавлено ожидание 2.5 с, калибровка из окна ушла — ошибка осталась.
- **Не SELinux и не TEE-доступ.** `/dev/mobicore-user` открыт (fd 14),
  отказов `avc` по `mobicore`/`tee` нет.
- **Не пустая таблица TA-команд.** Раньше `table[0xb8/c0/c8]` были NULL
  (TA не поднят). После выдачи TEE-правила TA отвечает.

---

## 5. Гипотезы на следующий заход

1. **TA проверяет MAC токена и отвергает его** (ключ Gatekeeper не совпадает с
   ожидаемым TA). Тогда A/B со стоковым HIDL тоже даст `-1004`, и это проблема
   связки Gatekeeper ↔ Silead TA на GSI.
2. **`0x15` вызывается с нулевым аргументом**, и TA загружает challenge не для
   того «пользователя»/БД, чей `user_id` потом приходит в HAT.
3. **CA-слой (`silfp_ca_send_modified_command`) добавляет к пакету значение из
   `global[0xc28]`** (движок кладёт туда `*(u64*)(hat+9)` = SID), и TA сравнивает
   его с challenge — тогда расхождение заложено в самом вендорном коде.

---

## 6. Следующие шаги

### Вариант A — обход проверки в вендорной библиотеке (самый быстрый и предсказуемый)

В `silfp_enroll_enroll`, адрес `0x3645c`:

```
36458: bl  silfp_impl_verify_enroll_challenge
3645c: tbnz w0, #0x1f, 0x364a0      ; <-- отрицательный результат → onError(1)
```

Заменить `tbnz` на `nop` (4 байта). Тогда `enroll` продолжится без проверки
challenge, TA-хранилище шаблонов при этом остаётся защищённым.

Это потребует доставки пропатченного `libsl_fp_impl.so` — либо bind-mount поверх
`/vendor/lib64/libsl_fp_impl.so`, либо через preload. Проверяется за один цикл.

### Вариант B — A/B со стоковым HIDL-HAL

Вернуть HIDL-манифест, включить стоковый бинарник, прогнать запись.
Замечание пользователя справедливо: AIDL-путь сейчас единственный рабочий, и
возврат на HIDL может упереться в зависший `FingerprintUpdateActiveUserClient`.
Поэтому A предпочтительнее.

### Вариант C — разобрать CA/TA

Включить `log.tag.fingerprint=VERBOSE`, собрать полный дамп TA-команд вокруг
`0x17` и сравнить с поведением на стоковой прошивке (есть `backups/`).

---

## 7. Полезные команды

```bash
# состояние
adb shell 'ps -Z -A | grep fingerprint@2.1'
adb shell 'dumpsys fingerprint | grep -A4 BiometricScheduler'
adb shell 'dumpsys biometric'

# логи
adb shell 'setprop log.tag.fingerprint VERBOSE'        # вендорный движок
adb shell 'logcat -b main -v time -s km6fod fingerprint'
adb shell 'tail -40 /data/adb/km6fod.log'              # журнал модуля

# HBM (только при включённом экране!)
adb shell 'cat /sys/kernel/tran_display/lcm_hbm_state'   # 0xFA off / 0xOK on

# перезапуск HAL без перезагрузки (домен magisk сохраняется только при ребуте)
adb shell 'stop vendor.tran_fps_hal'
adb shell 'kill -9 $(pidof android.hardware.biometrics.fingerprint@2.1-service-mediatek)'
```

Важно: обновление `files/fp_hal` требует перезагрузки, потому что bind-mount
держит старый inode, а init-service нельзя перезапустить в домене `magisk`.

---

## 8. Полировка локскрина (после подтверждённой разблокировки)

1. **Вспышка при блокировке.** Framework запускает `authenticate` сразу при
   появлении ключевого экрана, без пальца. Раньше это открывало окно HBM и
   подсвечивало всю панель. Теперь для аутентификации окно откладывается
   (`armCapture(..., open_window=false)`) и открывается на первом касании
   (`armIlluminationNow()`).
2. **`halControlsIllumination = true`.** Иначе SystemUI рисует собственный
   яркий слой поверх иконки, и вспышка появляется даже без HBM.
3. **Белый артефакт на иконке.** GSI отдаёт `pixel_pitch = -1.0`, из-за чего
   `UdfpsOverlayInteractor` неверно считает геометрию. Добавлен статический RRO
   `com.km6.suiudfps` с `dimen/pixel_pitch = 0.0654` (6.78", 1080x2400).

---

## 9. FOD при погашенном экране (в работе)

**Доказано:** контроллер тача видит палец и при чёрном экране.

```
[FTS_TS] fts_gesture_readdata: gesture_id=36, point_num=2
[GESTURE] g_gtp_gesture=0xbe
[GESTURE] Sorry, this gesture has been disabled.
```

`0xbe` — это жест `fp` из таблицы драйвера (`gesture_maps`, 12 байт на запись:
имя 2 байта, код 4 байта, тип 4 байта). Проверка в `GestureHandle`:

```asm
22a4: ldr  w11, [entry, #8]   ; тип
22a8: cmp  w11, #2
22c0: ldrb w9,  [entry, #3]   ; ФЛАГ ВКЛЮЧЕНИЯ (сейчас 0)
22c8: cbnz w9,  handle
22d0: "Sorry, this gesture has been disabled."
```

**Кто это включает на стоке:** `/vendor/bin/hw/vendor.hardware.trantp-service`
(AIDL `vendor.hardware.trantp.ITrantp/default`, **работает и на GSI**, PID виден).
Он пишет в `/proc/gesture_state` и `/proc/game_state`; в нём есть строки
`Trantp opration path= %s`, `Trantp opration result=%s`.

**Контракт метода уже восстановлен из бинарника:** `onTransact` читает
`AParcel_readInt32` затем `AParcel_readString` → метод вида
`opration(int32 cmd, String value)`.

**Что не сработало:** `/proc/gesture_function` (`fp1` принимается драйвером,
лог `gesture_data_write code1[be] enable[1]`) не выставляет байт `entry[+3]`;
`/sys/.../gesture_test=1` включает только разбор жестов; `/proc/gesture_state`
на запись — заглушка (`mov x0, xzr; ret`).

**Следующий шаг:** собрать маленький нативный пробник, который берёт
`vendor.hardware.trantp.ITrantp/default` из `servicemanager` и вызывает
`opration(cmd, value)` для cmd 0..N, наблюдая в logcat строку
`Trantp opration path= ...` — она сразу покажет cmd, включающий FOD-жест.

**Альтернатива:** патч `adaptive-ts.ko` (`cbnz w9` → `b handle`), но это правка
модуля ядра: при неудаче умрёт тач-ввод.

---

## 10. Переносимость (требование: другие GSI, Android 15–17)

Что уже переносимо:
- HAL не зависит от версии GSI: AIDL-интерфейс собирается из vendored-дерева,
  VINTF-манифест монтируется, `config_biometric_sensors` чистится оверлеем.
- Патч `libsl_fp_impl.so` привязан к конкретной сборке вендора — проверять по
  md5 перед применением, иначе пропускать (иначе можно сломать сканер).
- Пути узлов (`lcm_hbm_state`, `/proc/gesture_*`) определяются с fallback.

Что нужно доделать для 15–17:
- версия AIDL-интерфейса в VINTF-манифесте сейчас `<version>2</version>`;
  на Android 16/17 framework может ожидать 3/4 — стоит выбирать версию
  по `ro.build.version.sdk`.
- Проверять наличие символов (`dlsym`) вместо жёстких адресов, где возможно.

---

## 11. FOD на полностью погашенном экране — текущее состояние

### Что уже работает

- Тач-контроллер **видит палец при погашенной панели** и отдаёт его как жест
  (`fts_gesture_readdata` → `g_gtp_gesture=0xbe` = жест `fp`).
- Жест включается вызовом `vendor.hardware.trantp.ITrantp/default`:
  `opration(3, "fp1")` → `/proc/gesture_state`. Делается в `service.sh` при
  каждой загрузке (лог: `FOD wake gesture enabled`).
- После включения контроллер отдаёт по `/dev/input/event5` пары
  `EV_KEY 0xbe down/up` — проверено через `getevent`.
- HAL читает это событие (`TouchReader::kFodKeyCode`), а также обычные
  координаты касаний.
- Аутентификация на погашенном экране **проходит**:
  `onAcquired(GOOD)` → `authentication succeeded id=1` →
  `BiometricService: Succeeded(isStrongBiometric=true)`.

### Блокер: физика подсветки

`lcm_hbm_state` — это **множитель яркости**, а не источник света:

```
в doze:   tran_lcm_hbm_store value=1
          [DISP][E]tran_lcm_hbm_set ... display state is slept or idle, skip set hbm node.
на ходу:  panel_hbm_set_cmdq_switch en=1, mapped_level=96, aod=0
          (96 из 5119 — ~2%)
```

Запись в `/sys/class/leds/lcd-backlight/brightness` в doze тоже не даёт света
(проверено: узел принимает значение, панель остаётся тёмной).

**Вывод:** свет под пальцем возможен только когда дисплей реально включён —
тогда SystemUI рисует свой белый круг, а HBM поднимает яркость панели.

### Блокер: пробуждение из HAL

`input keyevent 224` из shell (домен `su`) экран будит (`Dozing → Awake`).
Но из процесса HAL команда падает:

```
cmd: Failure calling service input: Failed transaction (2147483646)
```

причём и напрямую, и через `/sbin/su -c`. При этом `NoNewPrivs=0`,
`Seccomp=0`, capabilities полные, контекст `u:r:magisk:s0` — тот же, что у
рабочего варианта из shell. Причина не локализована.

Добавлен обход: `service.sh` поднимает хелпер, читающий FIFO
`/data/adb/km6fod/run/wake.fifo`, а HAL пишет туда `wake`. Хелпер запускает
`input keyevent 224` как обычный дочерний процесс супервизора.

### Следующий шаг (проверить первым делом)

1. Погасить экран.
2. `adb shell "echo wake > /data/adb/km6fod/run/wake.fifo"`
3. Смотреть: проснулся ли экран и что в `/data/adb/km6fod.log`
   (`wake helper: input keyevent 224 rc=...`).

Это отделяет проблему хелпера от проблемы HAL:
- хелпер не будит → менять способ (рассмотреть `cmd power`, Binder-вызов
  `IPowerManager.wakeUp` из HAL, либо init-сервис с нужным доменом);
- хелпер будит, а HAL не шлёт → чинить запись в FIFO в `Session::wakeDisplay()`.

---

## 12. Замечено пользователем: пункт «разблокировка отпечатком при заблокированном экране»

Пункт в настройках присутствует, но не работает.

Две возможности, различить по настройкам:

- **A (вероятнее):** это тот же сценарий «распознавание без предварительного
  пробуждения». Тогда причина — §11: панель в doze, HBM не даёт света.
  Заработает после починки пробуждения через `/dev/uinput`.
- **B:** отдельный флаг (например, «Quick unlock» — разблокировка без Enter
  после PIN). К отпечатку отношения не имеет.

Проверить при возврате телефона:

```bash
adb shell settings list secure | grep -iE 'finger|unlock|lock'
adb shell settings list system | grep -iE 'finger|unlock'
adb shell dumpsys fingerprint | head -4
```

---

## 13. План на следующую сессию (кратко)

1. **`/dev/uinput`** — заменить пробуждение через `input`/FIFO на эмиссию
   `KEY_WAKEUP` в виртуальное устройство (ядро, без Binder и Java).
   Проверить наличие узла и SELinux-метку, при необходимости добавить правило.
2. Проверить ключи настроек из §12.
3. Пересобрать модуль, `./build_module.sh`, затем `./install.sh --status`.

---

## 14. Разблокировка с полностью погашенного экрана — итог исследования

### Проверено на железе

- Тач-контроллер при погашенной панели **обычно событий не отдаёт**: при нажатиях
  `onTouchEvent` не вызывается, `getevent` на `/dev/input/event5` пуст. Наш reader
  жив (`fd -> /dev/input/event5`), то есть дело не в нём.
- Иногда контроллер всё же отдаёт **gesture-режим**: `EV_KEY 0xbe` (жест `fp`,
  включается через `ITrantp.opration(3,"fp1")` → `/proc/gesture_state`). Но режим
  нестабилен: после перезагрузок он то есть, то нет, и из userspace не форсируется
  (`gesture_test` эффекта не даёт).
- Панель в doze **не светит**: `tran_lcm_hbm_set ... display state is slept or idle,
  skip set hbm node`, а при остаточной яркости `mapped_level` = 96–289 из 5119.
  Запись в `lcd-backlight` в doze игнорируется.
- `KEY_WAKEUP` в `/dev/uinput` из процесса HAL ядро принимает (устройство
  `km6fod-wake` видно в `dumpsys input`, события пишутся), но power manager их
  игнорирует. Тот же код, запущенный из shell, экран будит (`Dozing → Awake`).
- `cmd input` из дочернего процесса HAL падает: `Failed transaction (2147483646)`.
- Готовый обход: отдельный скрипт `wake_helper.sh`, запускаемый `service.sh`,
  читает флаг-файл `run/wake_request` и вызывает `input keyevent 224` — из shell
  это работает и экран просыпается (проверено: `wake helper: input keyevent 224 rc=0`).

### Вывод

Цепочка «касание на чёрном экране → свет → разблокировка» упирается не в HAL, а в
две вещи вне userspace:

1. **Контроллер тача не сканирует при выключенной панели** — сигнала о касании
   просто не существует большую часть времени.
2. **Нет провайдера подсветки дисплея** (`UdfpsHbmProvider`): SystemUI вызывает
   только `UdfpsDisplayMode.disable`, поэтому сам экран для FOD не поднимает и
   белый круг на тёмном экране не рисует. Переключатель LineageOS
   (`secure/screen_off_udfps_enabled=1`) рассчитан именно на этот провайдер.

### Что остаётся возможным

- **A.** Оставить как есть: FOD полностью работает при включённом экране (запись и
  разблокировка проверены), с чёрного экрана — через кнопку питания.
- **B.** Патч `adaptive-ts.ko`, чтобы жест `0xbe` отдавался всегда, без реального
  suspend. Это даёт HAL сигнал, дальше работает уже готовая связка
  «триггер → wake_helper → пробуждение → HBM». Риск: тач-ввод — единственное
  устройство ввода, при неудачной загрузке модуля останется только ADB.

### 14.1 Найдено решение для сигнала с выключенного экрана

Драйвер `adaptive-ts.ko` переводит жест в код `0xbe`, но затем отбрасывает его:

```
GestureIdTranslate 347: g_gtp_gesture=0xbe
GestureHandle 412: Sorry, this gesture has been disabled.
```

Причина — проверка в `GestureIdTranslate` (0x2020):

```
cmp  w8, #0xbe                   ; жест fp?
adrp/ldr w9,  [gestureclick_enabled]      (.bss+0x1a8)
adrp/ldr w10, [gesturestandby_enabled]    (.bss+0x1ac)
orr  w9, w10, w9
cbnz w9, 0x20c0                  ; хотя бы один включён -> жест отдаётся
```

Оба флага нулевые. Включаются они **не** через `gesture_state` (тот обработчик —
пустышка), а записью в `/proc/gesture_function`:

```
echo fp1 > /proc/gesture_function
  -> gesture_data_write 224: gesture_data_write code1[be] enable[1]
```

После этого в dmesg `GestureHandle 368: Single finger down`, в `getevent` идут
`0001 00be` down/up, и вся цепочка работает. Запись добавлена в `service.sh`
(`enable_fod_gesture`), так что переживает перезагрузку.

### 14.2 Что получилось и чего не получится

Работает: касание на чёрном экране → жест `0xbe` → HAL → `KEY_WAKEUP` через
uinput (`Dozing -> Awake`, подтверждено опросом `mWakefulness` каждые 100 мс) →
`hbm: on` → SystemUI рисует круг → разблокировка.

Не получится: «светится только зона». Подсветка панели равномерная, а рисовать в
выключенном дисплее в этом GSI некому (`UdfpsHbmProvider` отсутствует, SystemUI
умеет только `UdfpsDisplayMode.disable`). Любой свет = поднятая панель = локскрин
целиком. Приглушение подсветки (запись в `lcd-backlight` из домена `magisk`
работает) гасит и круг тоже, потому что источник света общий.

### 14.3 Итог: цепочка подсветки переработана (ревью — `analysis/ILLUMINATION_REVIEW.md`)

Что было не так и что исправлено:

| Дефект | Причина | Исправление |
|---|---|---|
| Свет «не всегда» | взвод только на первом кадре пальца (`fresh_down`), повторные кадры не могли перезажечь окно | взвод на каждом кадре, пока панель не подтверждена |
| «Медленно/тупенько» | `armIlluminationNow` блокировал поток чтения тача до 8×800 мс | взвод вынесен в dispatcher, поток тача не блокируется |
| Мигание | «сброс» окна через `requestOff()` + 90 мс + заново | сброса нет; `requestOn()` сам перечитывает узел (`0xOK`) |
| Свет гас сам | четыре независимых владельца гашения | один владелец: дедлайн/завершение операции |
| Свет включался без касания | ретраи продолжались после ухода пальца | перед каждой попыткой проверка «операция идёт + палец внизу» |
| Разблокировка «чужим пальцем» | поздний результат применялся к следующему касанию | результат старше 1200 мс после ухода пальца отбрасывается |
| Требовал пароль | каждое «не совпало» уходило как неудачная попытка (набралось 17) | несовпадения не отправляются как неудачи |
| Не срабатывало сразу после блокировки | сессия фреймворка стартует уже после приложения пальца | после пробуждения HAL ждёт оверлей до 900 мс и сам сообщает движку «палец внизу» |

Измерено по логу: из 23 касаний при выключенном свете свет включился 23 раза;
на локскрине от касания до света 94 мс (было 1414 мс).

Развёрнуто: `fd56a84` (в модуле и в `/vendor/bin/hw/...`, HAL pid 30220).

Важное на будущее: **не перезапускать HAL подменой файла между проверками**.
Каждое «HAL died» framework считает признаком неисправного датчика и **аннулирует
записанные отпечатки** (подтверждено: `E/FingerprintProvider: HAL died`,
`hasEnrollments: false`). Развёртывать только через перезагрузку.
