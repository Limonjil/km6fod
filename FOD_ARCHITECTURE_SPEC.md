# FOD / UDFPS Architecture Specification (Clean-Room)

> Документ для реализации с нуля. Описывает **поведение, протоколы и контракты**,
> а не код. Все значения получены с живого устройства Tecno Spark 40 Pro (KM6,
> MT6789H) и подтверждены экспериментально, если не помечено «требует проверки».
>
> Целевая среда: Android 15, **AIDL**-HAL отпечатка, оптический подэкранный сенсор
> **Silead GSL7002** с TEE-обработкой (`silead_opt_tee`).

---

## СОДЕРЖАНИЕ

1. [Аппаратный и драйверный слой](#1)
2. [IPC и Binder-контракт](#2)
3. [Поведенческая модель (state machine)](#3)
4. [Системное окружение и права](#4)
5. [Анализ ошибок прошлой реализации](#5)
6. [Приложение: калибровка, диагностика, чек-листы](#6)

---

<a name="1"></a>
## 1. АППАРАТНЫЙ И ДРАЙВЕРНЫЙ СЛОЙ

### 1.1 Сенсор

| Параметр | Значение |
|---|---|
| Модель | **Silead GSL7002** (оптический, подэкранный) |
| Режим обработки | TEE (`silead_opt_tee`) — кадры не покидают secure world |
| Драйвер ядра | `tran_fp.ko` (`/vendor/lib/modules/`, дубль в `/vendor_dlkm/lib/modules/`) |
| Панель | `vtdr6126a_fhdp_dsi_vdo_dsc_tm_tm_144hz_km6`, DSI, DSC, 144 Гц |
| Экран | 1080 × 2400 px, density 420 |

Сенсор имеет два независимых канала:

```
 (1) SPI + IRQ  ──► /dev/silead_fp ──► driver tran_fp.ko ──► libsl_fp_impl.so
 (2) ext-socket ──► абстрактный unix-сокет "com_silead_fpext" ──► тот же движок
```

Канал (1) — данные и команды сенсора. Канал (2) — сервисная координация
(готовность UI, калибровка, тесты), обслуживается отдельным процессом
`vendor.silead.hardware.fingerprintext@1.0-service` (init-сервис `fpexts_hal`).

### 1.2 Символьные устройства

Движок при инициализации печатает таблицу поддерживаемых узлов (индекс → путь):

```
tran_fp_fd[0] = /dev/fortsense_fp
tran_fp_fd[1] = /dev/sunwave_fp
tran_fp_fd[2] = /dev/goodix_fp
tran_fp_fd[3] = /dev/fpsensor
tran_fp_fd[4] = /dev/silead_fp      ◄── используется на этом устройстве
```

Фактические атрибуты нашего узла:

| Свойство | Значение |
|---|---|
| Тип | character, misc (`major 10`, `minor 228`) |
| Права | `crw-rw-rw-` (0666) |
| SELinux-метка | `u:object_r:silead_fpd_device:s0` |

Доступ к узлу нужен **процессу HAL**. Если HAL запускается не из vendor-домена,
права 0666 достаточно, но SELinux-политика должна разрешать `rw_file_perms`.

### 1.3 Sysfs / proc — управление подсветкой (HBM) и координатами

#### 1.3.1 Основной узел подсветки

```
/sys/devices/platform/soc/14013000.dsi/lcm_hbm_state
(алиас: /sys/kernel/tran_display/lcm_hbm_state)
```

| Запись | Чтение после записи | Физический эффект |
|---|---|---|
| `0` | `0xFA` | HBM выключен, обычная яркость |
| `1` | `0xOK` | HBM включён (панель переходит в режим повышенной яркости) |
| `2`, `3` | `0xOK` | store принимает значение, но панельная команда **не выдаётся** — не использовать |

Ядро при записи печатает в dmesg:

```
tran_lcm_hbm_store value= 1
[LCM] panel_hbm_set_cmdq_switch FPS=144Hz en=1, mapped_level=193, aod=0
[LCM] panel_hbm_set_cmdq_switch HBM_CMD=0x63, HBM_PARA=0xb,0x53,0x1,0x91
```

Это подтверждает, что запись доходит до панели.

#### 1.3.2 Связанные узлы дисплея (для диагностики / AOD)

```
/sys/kernel/tran_display/lcm_dimming_state        (rw)
/sys/kernel/tran_display/lcm_doze_backlight       (r)
/sys/kernel/tran_display/lcm_name                 (r)  → vtdr6126a_...
/sys/kernel/tran_display/lcm_pcd_check            (r)
/sys/kernel/tran_display/lcm_pcd_check_by_read_lcm(r)
/sys/class/leds/lcd-backlight/brightness          (rw) 0..5119
/sys/class/leds/lcd-backlight/max_brightness      (r)  → 5119
```

#### 1.3.3 Координаты пальца в ядре — **НЕ РАБОТАЕТ**

```
/sys/kernel/tran_fp/fod_location_xy     (rw, но заглушка)
/sys/kernel/tran_fp/ic_name             (r)  → GSL7002
/sys/kernel/tran_fp/vendor_name         (r)  → silead_opt_tee
```

Запись в `fod_location_xy` **принимается без ошибки**, но значение остаётся `null`,
а dmesg пишет:

```
[Tran_FP][Info][fod_location_xy_store][2067][fod_location_xy_store]has not been implemented.
```

Форматы, проверенные безрезультатно: `"560,2205"`, `"560 2205"`, `"560:2205"`, `"560"`.
**Вывод: координаты через ядро передать нельзя. Не тратить на это время.**

#### 1.3.4 Критический механизм: kernel-uevent о «полном HBM»

Оптический сенсор **физически не может** захватить кадр без подсветки панели над
собой. Движок это знает и блокирует стадии `PressEnroll` / `PressAuth` до
подтверждения от ядра.

Признаки в бинарнике движка `libsl_fp_impl.so`:

```
_thr_netlink_recv
TRAN_FULL_HBM_EVENT=FULL_HBM_SET
[AUTH_TIME] TRAN_FULL_HBM_EVENT=true, screen status: %d, is blind: %d
[AUTH_TIME] [screen blind] screen off, icon disappear
```

Признаки в драйвере `tran_fp.ko`:

```
tran_fasync_for_lcm_full_HBM
"lcm start setting HBM and prepare to notify fp HAL by uevent"
TRAN_FULL_HBM_EVENT=FULL_HBM_SET
```

**Цепочка (сток):**

```
драйвер панели (LCM) включает «полный HBM»
   │  fasync-сигнал
   ▼
tran_fp.ko : tran_fasync_for_lcm_full_HBM
   │  uevent через netlink (NETLINK_KOBJECT_UEVENT, группа 1)
   ▼
libsl_fp_impl.so : поток _thr_netlink_recv
   │  strstr(buf, "TRAN_FULL_HBM_EVENT=FULL_HBM_SET")
   ▼
разблокировка стадии захвата → SPI-опрос сенсора
```

**Наблюдение на GSI:** запись в `lcm_hbm_state` включает HBM на панели, но
fasync-уведомление **не отправляется** — ни при открытом, ни при закрытом узле
сенсора. Слушатель netlink за всё время не зафиксировал ни одного события
`TRAN_FULL_HBM_EVENT`. Это и есть корневая причина «сенсор молчит».

Реализатору **обязательно** решить одну из трёх задач (порядок предпочтения):

1. Найти инициирующий путь дисплея, который приводит к вызову
   `tran_fasync_for_lcm_full_HBM` (AOD/doze-переход, отдельный узел «full HBM»,
   иная последовательность включения).
2. Обойти ожидание в движке: строка по vaddr `0x5aa27`, ссылка в коде `0x4d970`,
   сравнение `strstr` по адресу `0x4dac4` (бинарник `libsl_fp_impl.so`, 418 784 байт).
3. Реализовать собственный путь захвата, не полагаясь на вендорный движок.

#### 1.3.5 Передача координат касания

Экспортируемая функция движка:

```
int silfp_impl_set_touch_info(const void* buf, int len)
      └─► silfp_cmd_send_cmd_with_buf(cmd=1, buf, len)
```

Внутри библиотеки вызовов нет — данные приходят извне (ext-канал или HAL).
Формат `buf` не документирован; рабочий минимум, проверенный на практике:

```
struct { int32 x; int32 y; int32 minor; int32 reserved; }   // 16 байт, little-endian
```

Возврат `0` = принято (сенсор подтверждает приём 56 вызовов подряд без ошибок).
Является ли этот формат тем, что ждёт прошивка, — **не подтверждено**; при
отсутствии эффекта считать приоритетным §1.3.4.

#### 1.3.6 Прочее в драйвере (справочно)

```
/sys/module/tran_fp/parameters/bufsiz
ioctl-канал: tran_fp_ioctl (enable/disable clk, SPI transfer, fasync)
```

Драйвер умеет включать/выключать SPI-клок по ioctl — движок делает это
автоматически вокруг каждой транзакции.

### 1.4 Тайминги

| Этап | Значение | Комментарий |
|---|---|---|
| Пауза после записи HBM | **20 мс** | движок выдерживает её сам (`usleep`) перед обращением к сенсору |
| Ожидание «UI готов» | ≤ 1000 мс, шаг опроса 1 мс | `silfp_cust_wait_ui_ready` опрашивает состояние экрана |
| Таймаут операции enroll в AOSP | 60 с (параметр `timeout` в команде enroll) | передаётся в сенсор |
| Watchdog планировщика AOSP | ~3 с после `cancel()` | при отсутствии завершения клиент снимается принудительно |
| Ретрай ext-соединения | **4 с** | сервис `fpexts_hal` повторяет `connect()` каждые 4 с |

Практическое правило: **HBM включать до первой команды сенсора и выключать не
позднее 100 мс после завершения операции**, иначе AMOLED деградирует.

### 1.5 Калибровочные данные

| Источник | Назначение |
|---|---|
| `/mnt/vendor/persist/fpcal.dat` | базовая калибровка |
| `/mnt/vendor/persist/fpcal4.dat` | калибровка канала 4 |
| `/mnt/vendor/persist/fpcal5.dat` | калибровка канала 5 |
| `/mnt/vendor/persist/fpcal6.dat` | калибровка канала 6 |

Рабочий каталог: `/data/vendor_de/0/fpdata/`
Требования: файлы `fpcal*.dat`, владелец `system:system`, режим `0600`.
После чистой установки GSI каталог **пуст** — файлы нужно скопировать из `persist`,
иначе сенсор работает без калибровки (захват нестабилен/невозможен).

---

<a name="2"></a>
## 2. IPC И BINDER-КОНТРАКТ

### 2.1 Выбор интерфейса

Используется **AIDL** `android.hardware.biometrics.fingerprint.IFingerprint`
(стабильный VINTF), инстанс `default`.

Причины отказа от HIDL `@2.1`:

| Проблема HIDL-пути | Следствие |
|---|---|
| Framework создаёт `FingerprintUpdateActiveUserClient` к HIDL-демону | При отсутствии/зависании демона клиент навсегда занимает планировщик биометрии, кнопка записи неактивна |
| HIDL 2.1 не передаёт геометрию сенсора | Координаты берутся из конфигурации, легко ошибиться |
| HIDL 2.1 не имеет `onUiReady` | Невозможно сообщить движку о готовности подсветки |

AIDL даёт: `getSensorProps` (полная геометрия), `ISession.onUiReady`,
`onPointerDown/Up`, типизированные ошибки.

### 2.2 Регистрация сервиса

```
Имя:   android.hardware.biometrics.fingerprint.IFingerprint/default
API:   AServiceManager_addService(binder, name)
       ABinderProcess_setThreadPoolMaxThreadCount(4)
       ABinderProcess_joinThreadPool()
```

Обязательные условия:

- интерфейсные классы сгенерированы с `--stability vintf` и несут
  `@VintfStability` — иначе framework не считает HAL VINTF-стабильным;
- VINTF-манифест с этим инстансом присутствует в
  `/vendor/etc/vintf/manifest/` (см. §4.3);
- после регистрации `dumpsys fingerprint` должен показать
  `"service":"FingerprintProvider/default"`.

### 2.3 `IFingerprint` — методы, вызываемые фреймворком

| Метод | Когда вызывается | Требования |
|---|---|---|
| `getSensorProps()` | при инициализации провайдера | вернуть **ровно один** `SensorProps` (см. §2.4) |
| `createSession(sensorId, userId, callback)` | при старте пользователя / каждой операции | вернуть объект `ISession`, сохранить `callback` |

`createSession` вызывается **раньше**, чем любая операция; открывать сенсор в нём
**нельзя** — это подвешивает очередь (проверено).

### 2.4 `SensorProps` — точные значения

```
commonProps.sensorId                = 0
commonProps.sensorStrength          = STRONG (15)
commonProps.maxEnrollmentsPerUser   = 5
commonProps.componentInfo           = { "silead", "1", "1", "0", "1" }

sensorType                 = UNDER_DISPLAY_OPTICAL (3)
halControlsIllumination    = false     ← подсветку рисует framework
halHandlesDisplayTouches   = false     ← касания обрабатывает framework
supportsDetectInteraction  = false
supportsNavigationGestures = false

sensorLocations[0]:
    display        = ""               ← ПУСТАЯ строка = основной дисплей
    sensorLocationX = 560
    sensorLocationY = 2205
    sensorRadius    = 100
    sensorShape     = CIRCLE
```

**Два критических условия:**

1. `display` **не может** быть `"local:0"` — framework не распознаёт такой дисплей
   и обнуляет координаты, из-за чего `udfpsBounds = Rect(0,0,0,0)`, кольцо записи
   рисуется в левом верхнем углу, а касания не передаются в HAL.
2. Координаты `(560, 2205)` — калиброванная зона сканера, получена по светящейся
   точке панели. Радиус 100 соответствует видимой зоне.

### 2.5 `ISession` — методы и обязательные ответы

| Метод | Что обязан сделать HAL |
|---|---|
| `generateChallenge()` | вызвать вендорный `pre_enroll`, вернуть полученный 64-битный challenge через `onChallengeGenerated`. Если сенсор недоступен — вернуть 1 (не ошибку) |
| `revokeChallenge(challenge)` | **обязательно** вызвать `onChallengeRevoked(challenge)` |
| `enroll(HardwareAuthToken, out cancellation)` | собрать 69-байтный `hw_auth_token_t`, вызвать `enroll`, вернуть объект `Cancellation` |
| `authenticate(operationId, out cancellation)` | запустить захват, вернуть `Cancellation` |
| `detectInteraction(out cancellation)` | вернуть `Cancellation`; по факту касания — `onInteractionDetected()` |
| `enumerateEnrollments()` | вернуть список через `onEnrollmentsEnumerated(ids)` |
| `removeEnrollments(ids)` | `onEnrollmentsRemoved(ids)` |
| `getAuthenticatorId()` | **никогда не возвращать ошибку**: `onAuthenticatorIdRetrieved(0)` |
| `invalidateAuthenticatorId()` | `onAuthenticatorIdInvalidated(newId)` |
| `resetLockout(hat)` | `onLockoutCleared()` |
| `close()` | `post_enroll` при необходимости, освободить ресурсы, `onSessionClosed()` |
| `onUiReady()` | сообщить движку о готовности UI (§1.3.4), включить HBM |
| `setIgnoreDisplayTouches(bool)` | учесть режим; framework ставит `false` на старте, `true` на отмене |
| `onPointerDown/Up(...)`, `onPointerDownWithContext/UpWithContext(...)` | передать координаты в сенсор (§1.3.5); только для касаний в зоне сенсора |

#### 2.5.1 Структура `hw_auth_token_t` (69 байт, packed, little-endian)

| Смещение | Размер | Поле | Источник в AIDL |
|---|---|---|---|
| 0 | 1 | `version` | всегда 0 |
| 1 | 8 | `challenge` | `hat.challenge` |
| 9 | 8 | `user_id` | `hat.userId` |
| 17 | 8 | `authenticator_id` | `hat.authenticatorId` |
| 25 | 4 | `authenticator_type` | `hat.authenticatorType` |
| 29 | 8 | `timestamp` | `hat.timestamp.milliSeconds` |
| 37 | 32 | `hmac` | `hat.mac` (обрезать/дополнить до 32) |

Ошибка в этом пакете не даёт видимого сбоя — сенсор возвращает 0, но захват
не начинается. Проверять раскладку обязательно.

### 2.6 Колбэк `ISessionCallback` — что и когда отправлять

| Событие | Метод | Примечание |
|---|---|---|
| challenge создан | `onChallengeGenerated(challenge)` | 64-бит |
| challenge отозван | `onChallengeRevoked(challenge)` | без него планировщик зависает |
| касание принято | `onAcquired(info, vendorCode)` | `info = GOOD (0)` на нормальном касании |
| частичное касание | `onAcquired(PARTIAL (1), code)` | по ситуации |
| прогресс записи | `onEnrollmentProgress(enrollmentId, remaining)` | `remaining = 0` ⇒ запись завершена |
| успех аутентификации | `onAuthenticationSucceeded(fid, hat)` | `fid != 0` и валидный HAT |
| неуспех | `onAuthenticationFailed()` | обязателен при несовпадении |
| ошибка | `onError(errorCode, vendorCode)` | **единственный** допустимый способ сообщить об ошибке |
| шаблоны | `onEnrollmentsEnumerated/Removed` | |
| блокировка | `onLockoutTimed/Permanent/Cleared` | |
| сессия закрыта | `onSessionClosed()` | |

#### 2.6.1 Перечисление кодов ошибок (AIDL `Error`)

```
UNKNOWN = 0            HW_UNAVAILABLE = 1      UNABLE_TO_PROCESS = 2
TIMEOUT = 3            NO_SPACE = 4            CANCELED = 5
UNABLE_TO_REMOVE = 6   VENDOR = 7              BAD_CALIBRATION = 8
```

#### 2.6.2 Перечисление `AcquiredInfo` (используемые значения)

```
GOOD = 0   PARTIAL = 1   INSUFFICIENT = 2   IMAGE_DIRTY = 3
FINGER_UP_TOO_FAST = 5   IMMOBILE = 6   TOO_SLOW …
```

#### 2.6.3 ЖЕЛЕЗНОЕ ПРАВИЛО

Возвращать из методов `ISession` **статус с ошибкой запрещено**.
`ScopedAStatus::fromServiceSpecificError()` превращается в
`android.os.ServiceSpecificException`, которую AOSP-клиенты биометрии **не
перехватывают**: исключение всплывает в `FingerprintHandler` и убивает
`system_server` (наблюдался bootloop). Любая проблема — через `onError(...)` и
`return ok()`, выходной `Cancellation` заполняется всегда.

### 2.7 Проксирование в вендорный слой

#### 2.7.1 Вариант A (рекомендуемый): прямой вызов вендорной библиотеки

```
dlopen("/vendor/lib64/hw/fingerprint.silead.default.so")
dlsym("HMI") → hw_module_t*
module->methods->open(module, "default", &device)
```

Карта слотов устройства (смещение в байтах / 8), получена дизассемблированием
`fingerprint_open`:

| Слот | Смещение | Функция | Примечание |
|---|---|---|---|
| 15 | 0x78 | (хранилище колбэка) | пишется `set_notify` |
| **16** | 0x80 | `set_notify(dev, cb)` | колбэк событий |
| **17** | 0x88 | `pre_enroll(dev)` → u64 | возвращает challenge |
| **18** | 0x90 | `enroll(dev, hat*, gid, timeout)` | |
| **19** | 0x98 | `post_enroll(dev)` | |
| **20** | 0xa0 | `get_auth_id(dev)` | |
| **21** | 0xa8 | `cancel(dev)` | |
| **22** | 0xb0 | `enumerate(dev)` | |
| **23** | 0xb8 | `remove(dev, gid, fid)` | |
| **24** | 0xc0 | `set_active_group(dev, gid, path)` | |
| **25** | 0xc8 | `authenticate(dev, opId, gid)` | |

Порядок инициализации обязателен:

```
open() → set_notify(cb) → set_active_group(0, "/data/vendor_de/0/fpdata")
```

`set_active_group` возвращает 0 при успехе. Вызов слота 17 вместо 24 (частая
ошибка нумерации) приводит к «тихому» отсутствию группы и неработающей записи.

#### 2.7.2 Дополнительные символы движка `libsl_fp_impl.so`

| Символ | Назначение |
|---|---|
| `silfp_cust_notify_ui_ready(int)` | «UI готов» |
| `silfp_impl_sync_finger_status_optic(int)` | синхронизация статуса оптики |
| `silfp_worker_screen_state_callback(int)` | состояние экрана, `2` = `STATE_ON` |
| `silfp_cmd_get_chipid(void*, void*)` | диагностика SPI (возврат 0 + 4 байта данных) |
| `silfp_impl_set_touch_info(const void*, int)` | координаты касания |
| `silfp_cust_set_hbm_mode(int)` | включение HBM (внутри пишет `lcm_hbm_state`) |
| `silfp_cust_restore_hbm()` | восстановление |
| `silfp_cust_set_brightness(int)` | используется калибровкой (`750`) |

Вызывать **после** открытия сенсора: инициализация сбрасывает часть состояния.

#### 2.7.3 Вариант B: вендорный HIDL + MTK-расширение (справочно)

```
android.hardware.biometrics.fingerprint@2.1::IBiometricsFingerprint/default
vendor.mediatek.hardware.biometrics.fingerprint@2.1::ITranBiometricsFingerprint/default
      sendCommand(int cmd, int extras) → silfp_finger_cust_cmd
com.anc.fingerprint.extension@1.0::IFingerprintExtension/default
      excuteCommand(...) / setNotify(...)
vendor.silead.hardware.fingerprintext@1.0::ISileadFingerprint/default
      requestCmd(uint32 cmd, vec<uint8> data) / setNotify(cb) / onCmdResponse(int, vec)
```

Сокет ext-канала: абстрактный unix-stream `com_silead_fpext`.

Формат кадра (проверен):

```
[u32 длина][u32 команда big-endian]["s","l","f","p"][параметры]
   длина = число байт ПОСЛЕ поля длины
   минимальная длина = 8, иначе запись отклоняется
```

Обрабатываемые коды: `0x0D`, `0x0E`, `0x11`, `0x104`.
Семантика (по дизассемблеру):

| Код | Действие |
|---|---|
| `0x0E`, `0x11` | `notify_ui_ready` + `set_capture_dump_flag` + `sync_finger_status_optic` |
| `0x0D` | `set_hbm_mode` + `download_normal` + заводской тест оптики |
| `0x104` | калибровка: HBM on → яркость 750 → `cal_step` → восстановление |

Сокет принимает **только одного клиента** одновременно; его занимает
`fpexts_hal`. Прямая отправка кадров возможна лишь при остановленном сервисе
(`stop fpexts_hal`), после теста сервис нужно вернуть (`start fpexts_hal`).

---

<a name="3"></a>
## 3. ПОВЕДЕНЧЕСКАЯ МОДЕЛЬ (STATE MACHINE)

### 3.1 Запуск сервиса

```
[init] → service.sh (Magisk, late_start)
   │  ожидание sys.boot_completed (ограниченно, ≤120 с)
   │  chmod 755 на все *.sh
   │  запуск демона подсветки (fod-hbm)
   │  watchdog-цикл (интервал 5 с):
   │      если процесс HAL не найден → запустить
   ▼
[hal] main()
   │  ABinderProcess_setThreadPoolMaxThreadCount(4)
   │  AServiceManager_addService(IFingerprint/"default")
   │  ABinderProcess_joinThreadPool()
   ▼
[framework] FingerprintProvider  →  getSensorProps()  →  createSession()
```

HAL **не должен** открывать сенсор на старте: только регистрирует Binder-сервис.
Открытие — ленивое, при первой операции (§1.3.4 требует, чтобы подсветка была
включена позже).

### 3.2 Открытие сенсора (однократно, лениво)

```
open_sensor():
    if device != null: return device
    dlopen(fingerprint.silead.default.so)      // требует vendor-namespace!
    module = dlsym("HMI")
    device = module->methods->open(module,"default")
    load_symbols(libsl_fp_impl.so)             // §2.7.2
    slot16(device, event_callback)             // set_notify
    slot24(device, 0, "/data/vendor_de/0/fpdata")
    chipid = silfp_cmd_get_chipid(...)         // диагностика, 0 = SPI живой
    start_touch_reader_thread()                // §3.6
    return device
```

Если `dlopen` невозможен (namespace) — процесс должен работать из vendor-пути,
см. §4.5.

### 3.3 Цикл записи отпечатка (enroll)

```
Framework                HAL                      Движок/сенсор            Панель
   │ createSession         │                             │                     │
   │──────────────────────►│ сохранить callback          │                     │
   │ generateChallenge     │                             │                     │
   │──────────────────────►│ slot17 pre_enroll           │                     │
   │                       │────────────────────────────►│                     │
   │                       │◄──────────── u64 challenge ─│                     │
   │◄─ onChallengeGenerated│                             │                     │
   │ enroll(hat)           │                             │                     │
   │──────────────────────►│ собрать 69-байт HAT         │                     │
   │                       │ screen_state(2)             │                     │
   │                       │ notify_ui_ready(1)          │                     │
   │                       │ HBM on  ────────────────────┼────────────────────►│ lcm_hbm_state=1
   │                       │        ждать ≥20 мс          │                     │
   │                       │ slot18 enroll(hat,0,60)     │                     │
   │                       │────────────────────────────►│                     │
   │                       │◄──────────────── rc=0 ──────│                     │
   │◄────── ok(cancellation)                             │                     │
   │                       │        [ожидание событий]   │                     │
   │                       │◄──── acquired/enroll ───────│  (требует §1.3.4)   │
   │◄─ onEnrollmentProgress│                             │                     │
   │  … повтор до remaining=0 …                        │                     │
   │◄── onEnrollmentProgress(id, 0)                      │                     │
   │                       │ HBM off ────────────────────┼────────────────────►│ lcm_hbm_state=0
   │                       │ slot19 post_enroll          │                     │
```

Требования к реализации:

- HBM включается **до** команды `enroll` и выключается в любом исходе
  (успех, ошибка, отмена, таймаут, `close`);
- между включением HBM и первой командой сенсора — пауза ≥20 мс;
- события передаются в framework **сразу**, без накопления;
- при `remaining == 0` — завершение, HBM off.

### 3.4 Цикл аутентификации

```
Framework                 HAL                     Движок              Панель
   │ authenticate(opId)     │                        │                   │
   │───────────────────────►│ HBM on ────────────────┼──────────────────►│
   │                        │ slot25/20 → захват     │                   │
   │                        │◄──── acquired ─────────│                   │
   │◄────────── onAcquired  │                        │                   │
   │                        │◄──── result ───────────│                   │
   │                        │  fid≠0 → onAuthenticationSucceeded(fid,hat)│
   │                        │  fid=0 → onAuthenticationFailed()          │
   │                        │ HBM off ───────────────┼──────────────────►│
```

Правила:

- HAT передаётся в `onAuthenticationSucceeded` **только** если его длина 69 байт;
- при несовпадении — исключительно `onAuthenticationFailed()`, иначе устройство
  разблокируется чужим пальцем;
- между повторными попытками в рамках одной операции HBM **не выключать**.

### 3.5 Отмена и таймауты

```
cancel():
    slot21 cancel(device)
    HBM off
    вернуть Cancellation.cancel() → ok
```

Обязательные таймеры (рекомендуемые значения):

| Таймер | Значение | Действие |
|---|---|---|
| Максимальное время операции | 60 с | принудительный `cancel` + HBM off + `onError(TIMEOUT)` |
| Страховка HBM | 5 с без событий | HBM off, операция продолжается |
| Watchdog сенсора | 3 с после `cancel()` | повторный `slot21`, затем `onError(CANCELED)` |

**HBM не должен оставаться включённым дольше времени операции** — это выжигает
панель и приводит к деградации.

### 3.6 Поток чтения касаний (обход отсутствия касаний из framework)

Фреймворк GSI не передаёт касания в HAL (`shouldListenForUdfps = false`),
поэтому HAL читает тачскрин сам.

```
устройство: /dev/input/event5   (драйвер mtk-tpd)
структура события: struct input_event { timeval, u16 type, u16 code, s32 value }

отслеживать:
    EV_ABS / ABS_MT_POSITION_X (0x35)   → raw_x
    EV_ABS / ABS_MT_POSITION_Y (0x36)   → raw_y
    EV_ABS / ABS_MT_TRACKING_ID         → down = (value >= 0)
    EV_KEY / BTN_TOUCH                  → down = value
    EV_SYN / SYN_REPORT                 → кадр завершён

пересчёт в пиксели (диапазон тачскрина 17279 × 38399):
    px = raw_x * 1080 / 17279
    py = raw_y * 2400 / 38399

фильтр зоны (радиус ~250 px от центра сенсора):
    |px - 560| < 250  &&  |py - 2205| < 250

действие:
    silfp_impl_set_touch_info({px, py, 0, 0}, 16)
```

Поток создаётся один раз, при открытии сенсора. Завершать его — по флагу
`std::atomic<bool>` при `close()`.

### 3.7 Обработка событий движка

Движок вызывает зарегистрированный колбэк с указателем на сообщение, где:

```
w[0] — тип события (u32)
w[1..] — данные (union по типу)
```

**У Silead наблюдаемый тип — `0xFFFFFFFF` (то есть `-1`).** В родственном
вендорном ABI отрицательный тип означает «raw / vendor subtype», а не ошибку.
Игнорировать такие события нельзя: раньше они молча отбрасывались, и запись
отменялась по таймауту, не доходя до второго шага.

Рекомендуемая реализация: передавать событие в framework по конфигурируемой
таблице, чтобы подбирать трактовку без пересборки:

| Режим | Действие |
|---|---|
| 0 | ничего не отправлять (только лог) |
| 1 | `onAcquired(GOOD, arg)` |
| 2 | `onEnrollmentProgress(w1, w2)` |
| 4 | `onAcquired(PARTIAL, arg)` |
| иное | `onError(UNABLE_TO_PROCESS, arg)` |

Режим хранить в отдельном файле (`/data/local/tmp/fp_event_mode`), читать при
каждом событии. Полезно для отладки без перезапуска.

---

<a name="4"></a>
## 4. СИСТЕМНОЕ ОКРУЖЕНИЕ И ПРАВА

### 4.1 SELinux — обязательные правила

#### 4.1.1 Доступ к устройству сенсора

```
allow hal_fingerprint_default silead_fpd_device:chr_file rw_file_perms;
```

#### 4.1.2 Доступ к тачскрину (для §3.6)

```
allow hal_fingerprint_default input_device:chr_file rw_file_perms;
allow hal_fingerprint_default input_device:dir r_dir_perms;
```

#### 4.1.3 Netlink-uevent (для §1.3.4)

```
allow hal_fingerprint_default self:netlink_kobject_uevent_socket
      { read write create bind setopt sendto };
```

#### 4.1.4 Межпроцессное взаимодействие HAL ↔ вендорный сервис — **частая причина отказа**

Если HAL запускается из скрипта Magisk, его процесс и созданный им сокет
`com_silead_fpext` получают домен **`magisk`**, а вендорный сервис
`fingerprintext` работает в `hal_fingerprint_default`. В результате:

```
avc: denied { connectto } for path=com_silead_fpext
    scontext=u:r:hal_fingerprint_default:s0
    tcontext=u:r:magisk:s0
    tclass=unix_stream_socket  permissive=0
```

Отказ повторяется **каждые 4 секунды**, канал координации мёртв. Требуется:

```
allow hal_fingerprint_default magisk unix_stream_socket connectto;
```

Применение: `magiskpolicy --live "<правило>"` из `post-fs-data.sh`.
Файл `sepolicy.rule` в каталоге модуля на практике **не применился** — не
полагаться на него.

#### 4.1.5 Прочие типовые правила

```
allow hal_fingerprint_default fingerprint_vendor_data_file:dir  rw_dir_perms;
allow hal_fingerprint_default fingerprint_vendor_data_file:file create_file_perms;
r_dir_file(hal_fingerprint_default, sysfs)
allow hal_fingerprint_default sysfs_fp_name_path:dir  r_dir_perms;
allow hal_fingerprint_default sysfs_fp_name_path:file rw_file_perms;
```

### 4.2 RRO-оверлеи AOSP

| Пакет | target | Ресурс | Значение | Зачем |
|---|---|---|---|---|
| `com.km6.udfps` | `android` | `config_udfps_sensor_props` | **массив ровно из 3 int**: `560, 2205, 100` | геометрия зоны сканера |
| `com.km6.suiudfps` | `com.android.systemui` | `dimen/pixel_pitch` (float) | `0.0654` | GSI отдаёт `-1.0`, из-за чего `UdfpsOverlayInteractor` отказывается работать |
| `com.km6.sbfix` | `android` | `config_mainBuiltInDisplayCutout` | `M -21,0 L -21,41 L 21,41 L 21,0 Z @dp` | вырез под камеру |
| | | `status_bar_height` | `44dp` | высота статус-бара |

**Критично:** `config_udfps_sensor_props` применяется только при
`length == 3` (формат `{x, y, radius}`). При любой другой длине framework берёт
жёстко прошитые значения `540, 1636, 130` и игнорирует оверлей. Базовое значение
в `framework-res` — пустой массив.

Требования к сборке RRO: подпись platform-ключом, `resources.arsc` в APK
**без сжатия (STORED)**, выравнивание 4 байта.

### 4.3 VINTF

Каталог `/vendor/etc/vintf/manifest` находится на EROFS, добавить файл нельзя,
поэтому монтируется **целиком** подготовленная копия:

```
все стоковые манифесты (47 файлов)
  минус  android.hardware.biometrics.fingerprint@2.1-service-mediatek.xml
  минус  manifest_fingerprint.xml
  плюс   fingerprint_aidl.xml  (IFingerprint/default, AIDL)
```

Монтировать с `chcon -R u:object_r:vendor_configs_file:s0`.
Одиночный bind-mount файла внутри EROFS-каталога **не работает** — только каталог.

Дополнительно: вендорный HIDL-бинарник
`/vendor/bin/hw/android.hardware.biometrics.fingerprint@2.1-service-mediatek`
перекрывается нашим HAL (bind-mount), чтобы framework не поднимал HIDL-путь,
и выполняется `stop vendor.tran_fps_hal`.

### 4.4 Конфигурация запуска (init.rc)

Если HAL ставится как штатный init-сервис (а не через Magisk-скрипт), требуется:

```
service fingerprint_aidl /vendor/bin/hw/android.hardware.biometrics.fingerprint-aidl-service
    class hal
    user system
    group system input
    oneshot
    disabled        # запускается по запросу VINTF (interface_start)

on property:init.svc.zygote=running
    start fingerprint_aidl
```

Ключевые моменты:

| Параметр | Значение | Почему |
|---|---|---|
| `user` | `system` | доступ к `/data/vendor_de/0/fpdata` |
| `group` | `system`, `input` | чтение `/dev/input/event5` |
| `class` | `hal` | старт после готовности hwservicemanager |
| домен | `hal_fingerprint_default` | иначе SELinux-правила §4.1 не сработают |

**Если запуск через Magisk** — процесс попадёт в домен `magisk`; тогда обязательны
правила §4.1.4 и проверка, что каталог `/vendor/lib64` доступен (namespace).

### 4.5 Namespace и путь запуска

`dlopen` вендорной библиотеки из `/vendor/lib64/hw/` возможен **только** если
процесс запущен из vendor-раздела. При запуске из `/data`:

```
dlopen failed: library "/vendor/lib64/hw/fingerprint.silead.default.so"
  is not accessible for the namespace "(default)"
```

Решение: разместить бинарник по vendor-пути (в т.ч. через bind-mount) и запускать
оттуда, либо запускать `/vendor/bin/hw/...`.

### 4.6 Данные и каталоги

```
/data/vendor_de/0/fpdata          рабочий каталог шаблонов
    fpcal.dat, fpcal4.dat, fpcal5.dat, fpcal6.dat
    владелец system:system, режим 0600

/mnt/vendor/persist               источник калибровки
/data/adb/km6fix.log              журнал модуля
```

---

<a name="5"></a>
## 5. АНАЛИЗ ОШИБОК ПРОШЛОЙ РЕАЛИЗАЦИИ

Раздел составлен по фактически наблюдавшимся сбоям. Каждый пункт стоил
перезагрузок и часов отладки.

### 5.1 Фатальные (приводят к bootloop или падению системы)

| # | Ошибка | Симптом | Правильно |
|---|---|---|---|
| 1 | Возврат `ServiceSpecificException` из AIDL-методов | `FATAL EXCEPTION IN SYSTEM PROCESS: FingerprintHandler`, циклический ребут | `onError(...)` + `return ok()` |
| 2 | `transitionPoint = 1.0` в конфиге дисплея | `IllegalArgumentException: HBM transition point invalid` → bootloop | строго `0 < t < 1` (использовать `0.99`) + численная валидация перед монтированием |
| 3 | Патч `services.jar` без маскировки `oat/arm64/services.art` | правка «не применяется», поведение прежнее | создать пустые `services.odex`, `services.vdex`, `services.art`, `services.odex.fsv_meta` |
| 4 | Патч `start()` HIDL-клиента с вызовом `onUserStarted` при `null` | падение `system_server` в цикле | либо проверять на `null`, либо не патчить этот клиент вовсе |

### 5.2 Блокирующие работу (система жива, но функция не работает)

| # | Ошибка | Симптом | Правильно |
|---|---|---|---|
| 5 | `revokeChallenge` без `onChallengeRevoked` | `FingerprintRevokeChallengeClient` висит в `State: 2`, кнопка «Принять» серая | всегда вызывать колбэк |
| 6 | `getAuthenticatorId` возвращает ошибку | падение `system_server` при старте пользователя | `onAuthenticatorIdRetrieved(0)` + `ok()` |
| 7 | `loc.display = "local:0"` | `udfpsBounds = Rect(0,0,0,0)`, кольцо в углу, касания не доходят | `display = ""` |
| 8 | Массив `config_udfps_sensor_props` длиной ≠ 3 | координаты подменяются дефолтом `540,1636,130` | ровно 3 элемента |
| 9 | Открытие сенсора в `createSession` | очередь биометрии зависает | открывать лениво, в операции |
| 10 | События движка с типом `-1` отбрасываются | запись отменяется по таймауту, экран не двигается | передавать в framework (см. §3.7) |
| 11 | Касания не передаются в сенсор | сенсор не получает координат пальца | читать `/dev/input/event5` самостоятельно (§3.6) |
| 12 | `set_active_group` вызван не тем слотом (17 вместо 24) | `rc=0`, но группа не установлена, запись не идёт | карта слотов §2.7.1 |

### 5.3 Инфраструктурные

| # | Ошибка | Симптом | Правильно |
|---|---|---|---|
| 13 | HAL запускается из `post-fs-data` | регистрация Binder не удаётся, процесс завершается | запускать из `service.sh` (late_start) |
| 14 | HAL собирается с `-shared` | segfault сразу после старта (нет `main`) | `-pie`, проверять `file` |
| 15 | HAL запущен из `/data` | `dlopen` вендорной библиотеки невозможен | запуск из vendor-пути |
| 16 | Скрипт демона без `chmod 755` | `[ -x ]` не срабатывает, демон не стартует | восстанавливать права в `service.sh` |
| 17 | SELinux: сокет HAL в домене `magisk` | `connectto` отклоняется каждые 4 с, канал мёртв | правило §4.1.4 |
| 18 | Каталог `/data/vendor_de/0/fpdata` пуст | работа без калибровки | копировать `fpcal*.dat` из `persist` |

### 5.4 Архитектурные (осознанный выбор, который стоит пересмотреть)

1. **Свой AIDL-HAL поверх вендорного движка** оправдан, но требует постоянной
   поддержки при обновлении GSI: приватные структуры (`hw_auth_token_t`, карта
   слотов) могут измениться.
2. **Патч `services.jar`** (принудительный выбор AIDL-ветки) — необходимое зло для
   GSI, но он ломается при обновлении системы. Альтернатива — обеспечить
   присутствие HIDL-обёртки так, чтобы `FingerprintUpdateActiveUserClient`
   завершался сам.
3. **Демон подсветки, слушающий `logcat`** — хрупкая конструкция (зависит от
   формата строк). Правильнее управлять HBM из HAL по `onUiReady`/`cancel`.
4. **Полное отсутствие решения §1.3.4** — единственный оставшийся блокер. Всё
   остальное перечисленное реализуемо и проверено.

### 5.5 Подводные камни, не относящиеся к коду

- `transitionPoint` патчится в **живом** файле при каждой загрузке; хранить
  готовую копию нельзя — она устареет после обновления прошивки.
- Bind-mount поверх файла, который занят запущенным процессом, невозможен
  (`Text file busy`): сначала останавливать процесс, потом монтировать.
- Замена файла, являющегося источником bind-mount, через `mv` работает,
  через `cat >` — нет.
- Логи вендорской библиотеки идут в logcat с тегом `fingerprint`, а вывод
  собственного процесса — в stdout; при отладке нужно смотреть оба потока.
- Таймауты AOSP (watchdog планировщика) срабатывают быстрее, чем кажется:
  после `cancel()` клиент снимается примерно через 3 с.
- Проверка «сенсор видит палец» через `/proc/interrupts` — **ненадёжна**:
  оптический сенсор опрашивается по SPI, прерывания почти не растут.

---

<a name="6"></a>
## 6. ПРИЛОЖЕНИЕ

### 6.1 Диагностические команды

```bash
# провайдер и очередь
dumpsys fingerprint | grep -E 'service|Current operation'

# сенсор и модальность
dumpsys biometric | grep -A2 Sensors:

# геометрия зоны сканера в SystemUI
dumpsys activity service com.android.systemui/.SystemUIService | grep udfpsBounds

# состояние HAL и каналов
pgrep -af 'fingerprint@2.1'
cat /proc/net/unix | grep -i silead        # ожидается 2 записи: слушатель + соединение
lshal | grep -i fingerprint

# HBM
cat /sys/devices/platform/soc/14013000.dsi/lcm_hbm_state
dmesg | grep -iE 'tran_lcm|panel_hbm'

# утечка событий/отказов
logcat -d | grep -c 'avc.*fpext'
logcat -d | grep -iE 'silead|silfp|TRAN_FULL_HBM'
```

### 6.2 Устройство ввода: как найти тачскрин

```bash
cat /proc/bus/input/devices        # искать Name="mtk-tpd" и Handlers=eventN
getevent -pl /dev/input/eventN     # диапазоны ABS_MT_POSITION_X/Y
```

### 6.3 Псевдокод HAL (сводка)

```
on_start():
    register_service(IFingerprint/"default")
    join_thread_pool()            // всё остальное — по вызовам

get_sensor_props():
    return [ SensorProps{ id=0, STRONG, 5, type=UNDER_DISPLAY_OPTICAL,
                          halControlsIllumination=false,
                          halHandlesDisplayTouches=false,
                          location{display="", x=560, y=2205, r=100, CIRCLE} } ]

create_session(sensorId, userId, cb):
    store(cb); return Session()

Session.generate_challenge():
    if !sensor(): cb.onChallengeGenerated(1); return ok
    cb.onChallengeGenerated(slot17_pre_enroll())

Session.revoke_challenge(ch):
    cb.onChallengeRevoked(ch); return ok

Session.enroll(hat, out):
    if !sensor(): cb.onError(UNABLE_TO_PROCESS,0); *out=Cancellation; return ok
    screen_state(STATE_ON); notify_ui_ready(1)
    hbm_on();  sleep(20ms)
    rc = slot18_enroll(pack_hat_69(hat), gid=0, timeout=60)
    if rc != 0: cb.onError(UNABLE_TO_PROCESS, rc)
    *out = Cancellation
    return ok

on_engine_event(msg):
    type = msg.w[0]
    if type == 0xFFFFFFFF: dispatch_by_mode(msg)     // см. §3.7
    else: dispatch_standard(msg)

Session.close():
    hbm_off(); slot19_post_enroll(); cb.onSessionClosed(); return ok
```

### 6.4 Что считается «готово»

1. `dumpsys fingerprint` → `FingerprintProvider/default`, `Current operation: null`.
2. `udfpsBounds` = `Rect(460, 2105 - 660, 2305)`.
3. Экран записи открывается и доходит до шага «коснитесь несколько раз».
4. Касания в зоне сенсора вызывают вызовы `silfp_impl_set_touch_info` с
   корректными пиксельными координатами.
5. **Запись завершается**: `onEnrollmentProgress(id, 0)`, шаблон появляется в
   `enumerateEnrollments`, разблокировка по отпечатку работает.
6. При отмене/таймауте HBM выключен (`lcm_hbm_state` = `0xFA`).
