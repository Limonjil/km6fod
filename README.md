# KM6 FOD — clean-room AIDL fingerprint HAL

Under-display fingerprint support for the **Tecno Spark 40 Pro (KM6, MT6789H)**
running an Android 15 GSI. Sensor: **Silead GSL7002** (optical, TEE-backed,
`silead_opt_tee`), panel `vtdr6126a_fhdp_dsi_vdo_dsc_tm_tm_144hz_km6`.

The deliverable is a self-contained Magisk module containing a native AIDL HAL
plus the VINTF and SELinux plumbing it needs. Nothing is written to a raw block
device, nothing is flashed, and every change is reversible by removing the module
and rebooting.

This implementation is clean-room: it was written from the architecture
specification and observations made on the live device. No code or assets were
taken from any other fingerprint module.

Two things go beyond the HAL. With the display off the vendor touch controller is
put into gesture mode by its driver, so no multitouch reaches the HAL at all; the
module therefore loads the device's own `adaptive-ts.ko` and
`focaltech_ft3683g.ko` with four bytes changed, after checking their stock
sha256 and with a fallback to the untouched vendor copies. And because a plain
kernel wakelock raises the SoC without making the display interactive (SystemUI
then never creates the `authenticate` operation), the HAL owns a virtual
`KEY_WAKEUP` device through `/dev/uinput` and presses it on a dark-screen touch.
See `NOTICE.md` for the licensing status of those two vendor binaries.

---

## 1. The blocker, and why this design

The vendor engine (`/vendor/lib64/libsl_fp_impl.so`) refuses to start a capture
until it observes this uevent:

```
TRAN_FULL_HBM_EVENT=FULL_HBM_SET
```

On stock firmware the panel driver raises it through `tran_fp.ko`
(`tran_fasync_for_lcm_full_HBM`). Under a GSI it is never raised, so every
`enroll`/`authenticate` stalls: the engine's netlink thread
(`_cust_icon_hbm_status`) waits forever and the capture gate never opens.

The engine replaces embedded NULs with spaces and performs a `strstr()` over a
`NETLINK_KOBJECT_UEVENT` datagram. A datagram injected from userspace into
multicast group 1 is therefore indistinguishable from a kernel event. This module
uses that path. (The vendor library and the two touch modules *are* adjusted
elsewhere — see `NOTICE.md` — but nothing on this path needs either.)

Verified on device: after injection the engine logs

```
[AUTH_TIME] TRAN_FULL_HBM_EVENT=true, screen status: ..., is blind: 0
```

---

## 2. What was verified on hardware

These facts drive the implementation and are not assumptions:

| Fact | Evidence |
|---|---|
| `lcm_hbm_state` write takes ~21–26 ms | timed on device |
| Readback is `0xOK` only when the panel actually engaged | `cat` after write; dmesg `panel_hbm_set_cmdq_switch ... en=1` |
| Writing `1` while the display is asleep/idle can be **silently ignored** | dmesg `display state is slept or idle, skip set hbm node`; node stays `0xFA` |
| A skipped ON is **not latched**; waking later does not restore it | write 1 while off, wake, read still `0xFA` |
| `silfp_impl_set_touch_info` returns **int**; `0` means accepted | disassembly + device log `rc=0` |
| `silfp_cmd_get_chipid` returns **int**; `0` means success, negative is an error | disassembly; device reported `rc=-1007` once |
| Engine notification `type == -1` is an **error**, not a vendor "raw" event | disassembly of `silfp_notify_send_error_notice`: it sets type `-1` |
| Engine error codes are 0-based and align with AIDL `Error` | `silfp_common_cancel()` sends `5` = `CANCELED` |
| `AcquiredInfo.GOOD` is `1` in AIDL, engine still sends `0` | generated AIDL headers |
| `SensorStrength.STRONG` is `2`, `SensorShape.CIRCLE` is `1` | generated AIDL headers |
| The AIDL manifest with `<version>2</version>` is accepted by this GSI | `service list`, `dumpsys fingerprint` |
| `AIBinder_markVintfStability` returns `3` (VINTF category) on this libbinder | logged; treated as success |

The HBM window is therefore **verified by readback**, never by the fact that a
`write()` returned a positive length.

---

## 3. Architecture

### Threads

| Thread | Responsibility |
|---|---|
| Binder pool (4) | Short AIDL entry points only: state change, create cancellation, post work. Never blocks on vendor I/O. |
| `SerialWorker` | Serialises every vendor-engine call: open, group, pre/post enroll, enroll, authenticate, enumerate, remove, auth-id. A Binder thread must not wait for a sensor. |
| `CallbackDispatcher` | Serialises every HAL → framework callback. The engine never calls Binder from its own lock-holding threads. |
| `HbmController::worker` | Owns the panel node: readback-verified ON, settle, uevent, normal OFF. Sleeps indefinitely when no window is open. |
| `HbmController::guard` | Independent hard watchdog. Uses a fresh fd and does not wait for the worker, so a stuck sysfs write cannot leave the panel in HBM indefinitely. |
| `DeadlineScheduler` | One thread for operation timeout and cancel escalation. Sleeps until the earliest deadline; no periodic tick. |
| `TouchReader` | Blocks in `poll()` on evdev. No busy loop; wakes only on input. |
| `sigwait` thread | Blocks SIGTERM/SIGINT in every thread, receives them here, force-offs HBM synchronously, then `_exit()`. |

### Operation identity and cancellation

`generation_` is the operation identity. Every start increments it; every
`Cancellation` remembers the generation it belongs to. A late cancel from a
superseded operation cannot kill the current one. Engine events are accepted only
while a matching operation is active.

Cancel is acknowledged asynchronously with `onError(CANCELED)` (never inline from
inside `ICancellationSignal.cancel()`, which would re-enter the framework). A 3 s
escalation repeats the vendor cancel if the engine ignored it.

### HBM state machine

```
closed --requestOn--> verifying (write 1, read 0xOK, retry)
      --verified--> settling (90 ms, abortable in 10 ms slices)
      --uevent SET--> open (inactivity 5 s / absolute cap 30 s)
      --requestOff / inactivity / cap--> off (uevent CLEAR, write 0, read 0xFA)
```

- If readback never reaches `0xOK`, the capture is aborted with
  `HW_UNAVAILABLE`; the gate is not injected into a dark panel.
- The guard can force-off during `verifying` or `settling`; an epoch bump makes
  the worker undo any `1` that lands after the guard's `0`.
- The operation stays active after a watchdog close; the next touch re-arms HBM.
- `detectInteraction` does not open the sensor or HBM: the evdev reader detects
  the touch. This avoids a pointless panel flash and wear.

### Lazy sensor and per-user group

`createSession()` never touches the vendor device. `armCapture()` (on the
`SerialWorker`) opens the sensor, applies the session's per-user template group,
then opens the HBM window. `generateChallenge()` also opens lazily because the
engine needs a group before `pre_enroll()`.

### Supervisor and fail-safe

The HAL binary takes an exclusive `flock` on
`/data/adb/km6fod/run/fp_hal.lock`; a second instance exits 0 and logs. The
`service.sh` supervisor uses `wait(1)` — no polling loop — and immediately after
the child exits (including SIGKILL) writes `0` to the panel node, verifies
`0xFA`, and only then restarts with backoff. This is the external fail-safe that a
process cannot provide for itself.

---

## 4. Layout

```
src/
  main.cpp              entry: instance lock, sigwait, register, join pool
  fingerprint_hal.*     IFingerprint: props, createSession, engine bridge
  session.*             ISession: operations, generations, cancellation, callbacks
  vendor_engine.*       shared_mutex proxy over the vendor vtable, lazy open/group
  hbm.*                 verified panel HBM, worker + independent guard
  deadline.*            one-shot deadline scheduler
  executors.*           bounded dispatcher and serial worker
  uevent.*              netlink uevent injector (mutex-protected)
  touch_reader.*        poll()-based evdev reader, scaled to display pixels
  auth_token.*          69-byte hw_auth_token_t <-> AIDL HardwareAuthToken
  engine_message.h      88-byte vendor notification format
  stability.h           explicit AIBinder_markVintfStability helper
  logging.h
third_party/
  aidl_gen/             generated AIDL stubs (--structured --stability vintf)
  syslibs/arm64-v8a/    real libbinder_ndk.so used for linking
magisk_module/          module payload (scripts, VINTF, sepolicy)
tools/
  fp_test.cpp           diagnostic AIDL client (not packaged)
  uevent_probe.c        netlink diagnostic
  namespace_probe.c     dlopen/namespace diagnostic
```

### Vendor vtable map (verified slot by slot)

| Offset | Function | Offset | Function |
|---|---|---|---|
| `0x70` | close/free | `0xb0` | enumerate |
| `0x80` | set_notify | `0xb8` | remove |
| `0x88` | pre_enroll | `0xc0` | set_active_group |
| `0x90` | enroll | `0xc8` | authenticate |
| `0x98` | post_enroll | | |
| `0xa0` | get_auth_id | `0xa8` | cancel |

### Engine notification (88 bytes)

| Offset | Field |
|---|---|
| 0 | `type` (`-1` = error, `1` acquired, `3` enroll, `4` removed, `5` authenticated, `6` enumerate) |
| 8 | `a` — acquired info / enroll *remaining* / error code |
| 12 | `b` — enrollment id |
| 16 | `c`, or the 69-byte `hw_auth_token_t` for authenticated |

---

## 5. Build

Requirements: Android NDK r26+ (default `/c/Users/Limonjil/Desktop/android-ndk-r26d`),
bash (Git Bash or WSL), `adb` with root.

```bash
./build.sh                 # -> build/fp_hal
./build.sh --clean         # clean objects
./build.sh --tools         # also build build/fp_test
./build_module.sh          # -> dist/km6fod-v1.1.0.zip
NDK=/path/to/ndk API=34 ./build.sh
```

The binary is a PIE executable with a statically linked C++ runtime. Its only
dynamic dependencies are `libbinder_ndk`, `liblog`, `libdl`, `libm`, `libc`;
the build fails if `libc++_shared.so` leaks in.

Hardening: `-fstack-protector-strong`, `-D_FORTIFY_SOURCE=2`, RELRO, `-z now`,
non-executable stack, `--gc-sections`, `--no-undefined`.

---

## 6. Deploy

```bash
./install.sh --test        # build + push + run now, no reboot
./install.sh --status      # process, service registration, provider dump, log
./install.sh --diag        # HBM dmesg, input devices, AVC denials, logcat
./install.sh --install     # build and install as a Magisk module (reboot)
./install.sh --uninstall   # stop supervisor/HAL, reset HBM, remove module
```

`--test` stops this project's previous instance and the stock vendor HIDL
provider. It does not know about, and does not modify, any other Magisk module.
If another module also publishes `IFingerprint/default`, disable that module's
fingerprint part first: only one AIDL instance may exist.

The diagnostic client:

```bash
adb push build/fp_test /data/local/tmp/
adb shell su -c /data/local/tmp/fp_test
```

It calls `getSensorProps`, `createSession`, `generateChallenge`,
`getAuthenticatorId`, `enumerateEnrollments`, `authenticate` and `cancel`. Run it
only while testing; it opens a real HBM window.

---

## 7. Validation checklist

1. `service list | grep fingerprint.IFingerprint` shows
   `android.hardware.biometrics.fingerprint.IFingerprint/default`.
2. `dumpsys fingerprint` shows `FingerprintProvider/default` and
   `Current operation: null`.
3. `dumpsys activity service com.android.systemui/.SystemUIService | grep udfpsBounds`
   returns `Rect(460, 2105 - 660, 2305)`.
4. The enroll screen reaches "lift and touch repeatedly".
5. `fp_test` prints a real challenge, a non-zero authenticator id and the
   enumeration list.
6. During `authenticate`, `/sys/kernel/tran_display/lcm_hbm_state` reads `0xOK`
   and dmesg shows `panel_hbm_set_cmdq_switch ... en=1`.
7. After `cancel`/`close`, the node reads `0xFA` and dmesg shows `en=0`.
8. `kill -9 <fp_hal-pid>` → the supervisor writes and verifies `0` and restarts
   within a few seconds.
9. Starting a second `fp_hal` exits 0 with
   `another fp_hal instance holds the lock`.
10. `./install.sh --uninstall` leaves no `fp_hal` process and HBM reads `0xFA`.

Logs: `/data/adb/km6fod.log`, `adb logcat -s km6fod`, and the vendor engine's
`fingerprint` tag.

---

## 7a. Why the Settings entry was missing (two independent causes)

Both had to be fixed; neither is HAL-side.

### A. `config_biometric_sensors` forced the legacy HIDL path

`AuthService` picks the fingerprint code path from this array:

```java
// services/core/java/com/android/server/biometrics/AuthService.java
new FingerprintSensorConfigurations(
        !(hidlConfigStrings != null && hidlConfigStrings.length > 0));
```

A **non-empty** array selects `addHidlSensors()` in `FingerprintProvider`, which
wraps every sensor in `HidlToAidlSensorAdapter` and asks for a HIDL
`IBiometricsFingerprint` daemon. That daemon does not exist on this GSI, so
`FingerprintUpdateActiveUserClient` waits forever in the biometric scheduler and
blocks the AIDL client queued behind it. Symptom: no fingerprint entry in
Settings, and `dumpsys fingerprint` reporting the user-switch client stuck in
`State: 2`.

`framework-res` ships the array empty, but two overlays populate it:
`com.mediatek.frameworkresoverlay` and `android.auto_generated_rro_product__`.

The module ships `KM6FpAidl.apk`: a platform-signed static RRO with priority 100
that clears the array, flipping `AuthService` onto the AIDL branch.

### B. The HAL ran in the wrong linker namespace

A process executed from `/data` gets the `default` linker namespace, whose
permitted paths exclude `/vendor/lib64`. `dlopen` of the vendor engine then fails:

```
library "/vendor/lib64/hw/fingerprint.silead.default.so" ... is not accessible
for the namespace "(default)"
```

So the binary cannot simply live in the module directory. `post-fs-data.sh`
bind-mounts it over the stock HIDL binary and `service.sh` executes it from
`/vendor/bin/hw/...`, which places it in the vendor namespace. The vendor HIDL
init service is stopped in `post-fs-data` (before `class late_start`), so the
mount point is free and no HIDL sensor is ever registered.

---

## 8. Troubleshooting

| Symptom | Check |
|---|---|
| HAL exits immediately, rc=0 | Another instance holds `/data/adb/km6fod/run/fp_hal.lock`; find it with `pgrep -f fp_hal`. |
| `addService` fails | Another module/vendor HAL already owns `IFingerprint/default`; disable its fingerprint part. |
| HBM never reaches `0xOK` | Display is off/doze, or the panel node is not writable; check `dmesg | grep tran_lcm_hbm`, `ls -Z` on the node, and the live SELinux rules. |
| Engine stalls after `enroll` | Check `logcat -s fingerprint` for `TRAN_FULL_HBM_EVENT=true`; verify the uevent injector is running and the netlink rule is live. |
| Touch coordinates wrong | `getevent -pl /dev/input/event5`; check `EVIOCGABS` ranges and the sensor zone filter. |
| `dumpsys` shows a HAL death | Check the HAL log for a callback failure; the supervisor should have restarted it. |

Do **not** repeatedly force HBM on with `echo 1` while debugging: use the HAL or
`fp_test`, which close the window on cancel/timeout.

---

## 9. Known limitations

- Multi-user is implemented (`set_active_group` per session user) but only user 0
  was exercised on hardware.
- The UDFPS SystemUI overlay (`pixel_pitch`) **is** shipped
  (`system/product/overlay/KM6SuiUdfps`); the sensor-props/geometry values it
  pins are those measured on this panel, so another panel needs its own overlay.
- The vendor ABI is private: the vtable offsets and the 69-byte token are pinned
  with `static_assert`/runtime checks but can change with a vendor library update.
- The uevent injection relies on the engine's substring test. A vendor library
  update that changes netlink handling would require revisiting it.
- A kernel that hangs forever inside a sysfs store cannot be fixed from
  userspace. The independent guard, the `sigwait` force-off, and the supervisor's
  `wait`+reset are the layers that bound the damage; a hard kernel lockup still
  requires a reboot.
- The VINTF fragment no longer carries a hand-written version: `post-fs-data.sh`
  takes the highest bound for this HAL from
  `/system/etc/vintf/compatibility_matrix*.xml` and clamps it to the implemented
  interface version (5), which is 4 on this Android 15 GSI and 5 on 16/17.
  Editing the fragment by hand is neither needed nor allowed.

---

## 10. Safety notes

- HBM is never left on after a normal operation: cancel, close, error, timeout and
  inactivity all close it; the guard and supervisor are backstops.
- An all-zero authentication token is treated as a **failed** attempt. The HAL
  does not forward an unproven success to the framework.
- All vendor calls are serialised; only `cancel()` is allowed to preempt an
  in-flight capture, as AOSP requires.
- Registering a second instance is refused by an exclusive file lock.

---

## 11. Support matrix (checked, not aspirational)

| Axis | State | Evidence |
|---|---|---|
| Android 15 (API 35) | **checked** | this device, `dumpsys fingerprint` shows the sensor running and unlocking |
| Android 16 / 17 | **built for, not verified** | the HAL is compiled from AOSP `android17-release` AIDL at interface version 5 (the band Android 16 and 17 require is 3-5), and the VINTF fragment takes its declared version from the device's own compatibility matrix. No A16/A17 image was available to run it |
| Magisk 27+ | **checked** (31.1 here) | module installs and the boot path runs |
| KernelSU | **supported, not device-verified** | `customize.sh` installs the `ksu` variant of the policy (`sepolicy.rule.ksu`) because KernelSU has no `magisk` type, and `post-fs-data.sh` mounts the two RROs itself with overlayfs when the manager does not mount `system/` (no metamodule). No KernelSU device was available to boot it; the gate leaves the stock provider running if any step does not land |
| APatch | **supported, not device-verified** | same zip, `magiskpolicy` and the `magisk` domain, as in Magisk. Nothing was verified on hardware |
| SELinux enforcing | **not checked** | on this device the domain the HAL runs in is permissive, which hides a missing rule. `post-fs-data.sh` now probes `/dev/mobicore-user` and the HBM node with `access()` and refuses the swap (stock provider stays) when they are denied, so a gap shows up as "module did not start", not as a silent half-working sensor |
| 4 KB pages | **checked** | `getconf PAGE_SIZE` = 4096 |
| 16 KB pages | **not checked** | all shipped ELFs are 16 KB aligned and `build_module.sh` refuses to package one that is not, but no 16 KB device was available |
| SELinux enforcing | **partially** | the domain this HAL runs in (`u:r:su:s0`) is permissive on Magisk. The rules were written for a confined domain; on an enforcing-manager they are what would decide whether it works |

### About the AIDL version

The AIDL sources are taken from AOSP `android17-release`
(`hardware/interfaces/biometrics/fingerprint`), frozen API versions 1-5, and the stubs
are generated with `aidl --structured --stability=vintf --min_sdk_version=31 -v 5`.
That gives every interface a real `version = 5` constant:

| Interface | What v5 adds | State here |
|---|---|---|
| `IFingerprint` | nothing since v1 (`getSensorProps`, `createSession`) | implemented |
| `ISession` | `onPointerCancelWithContext`, `setIgnoreDisplayTouches` | implemented |
| `ISessionCallback` | `onAuthenticationSucceededWithResult` | not called; the v5 callback keeps `onAuthenticationSucceeded`, so an unlock is still reported the v4 way |

The declared version comes from the device: `post-fs-data.sh` reads
`/system/etc/vintf/compatibility_matrix*.xml`, takes the highest upper bound for this
HAL, and clamps it to what the build implements (5). On this Android 15 device the
matrix says 3-4, so 4 is declared; on Android 16/17 the same matrix says 3-5, so 5 is
declared. Rebuilding the fragment by hand is not needed and not allowed.

What is still missing for an honest "supported on 16/17" claim is a run on such an
image, not code.

