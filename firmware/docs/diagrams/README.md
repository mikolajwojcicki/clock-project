# Firmware diagrams

PlantUML sources and their SVG/PDF exports for the clock firmware
(`firmware/apps/clock`, `firmware/src`) and its Bluetooth path (`web/`).
Every diagram is drawn **as built from the code**, not from `openspec/specs/`.

**Drawn from commit `8c415fe` (2026-11-02)**, the last commit touching
`firmware/apps/clock`, `firmware/src`, `firmware/boards` or `web/` at drawing time.
After code changes there, re-check the fact tables below and redraw.

## Index

| File (`.puml` → `.svg`, `.pdf`) | Level | Type | Shows |
|---|---|---|---|
| `l1-overview` | L1 | state | CPU states (start, praca, sen WFE, odświeżanie e-papieru), the DCF / alarm / BLE modes that power extra parts, every wake interrupt |
| `l2-modules` | L2 | component | ISRs (flags only), the six `app_poll` steps in `main.c`, all 14 `firmware/src` modules, hardware and SoftDevice |
| `l3-detail-a` … `l3-detail-f` | L3 | activity / state | one figure in six parts: **A** boot, main loop, RTC1 deadline, sleep entry/exit; **B** `poll_uart`, `poll_clock`, `poll_ble`; **C** `poll_alarm` with motion dismiss and buzzer; **D** `poll_dcf`; **E** DCF-77 decoder state machine and `on_edge`; **F** `draw` / e-paper refresh with nested sleep |
| `ble-overview` | BLE | activity, swimlanes | web page, browser and OS BLE, SoftDevice, `ble_sched.c`, schedule store; SoftDevice role note |
| `ble-sequence` | BLE | sequence | advertising, connect, discovery, schedule read, schedule write (single and long), flash save, disconnect |

L3 is split because one chart was far too large for A4. Every L3 node has a
`'` comment above it naming its `file:function`. `ble-overview` and
`ble-sequence` are dense: place them on a full page.
`software-flowchart.*` one level up is the older single-chart summary and is
left unchanged.
Draft Polish figure captions (agent draft, to be rewritten by the author):
[`captions.md`](captions.md).

Printed text size when a figure is scaled to the A4 text block (16 × 24.7 cm),
from the SVG `font-size` (2026-11-02): `l3-detail-d` 8.7 pt, `l3-detail-f` 7.2 pt,
`l3-detail-a`/`-c` 5.9 pt, `l1-overview` and `l3-detail-b` 5.6 pt, `l2-modules` 4.8 pt
(5.9 pt landscape), `l3-detail-e` 4.4 pt (6.8 pt landscape), `ble-sequence` 4.3 pt,
`ble-overview` 4.2 pt. The PDFs are vector, so they stay sharp when zoomed.

## Render

The SVG comes from the `plantuml/plantuml-server` podman image over HTTP;
the PDF comes from that SVG via `rsvg-convert` (keeps the Polish letters,
same pipeline as `firmware/docs/software-flowchart.*`).

```sh
# 1. Start the server (local only, removed when stopped).
podman run -d --rm --name plantuml-diag -p 127.0.0.1:18080:8080 docker.io/plantuml/plantuml-server:latest

# 2. Render every *.puml in this directory to .svg and .pdf.
nix-shell -p librsvg curl --run firmware/docs/diagrams/render.sh
#    or only some: ... --run 'firmware/docs/diagrams/render.sh l1-overview.puml'

# 3. Stop the server.
podman stop plantuml-diag
```

`render.sh` sends each source as the POST body to `/svg`:

```sh
curl -sS -f -X POST -H 'Content-Type: text/plain; charset=utf-8' \
  --data-binary @l1-overview.puml http://127.0.0.1:18080/svg -o l1-overview.svg
rsvg-convert -f pdf -o l1-overview.pdf l1-overview.svg
```

A PlantUML syntax error comes back as HTTP 400, so `curl -f` stops the script.
Checked on 2026-11-02 against `software-flowchart.puml`: the server output had
the same size (1770 × 2586 px) and DejaVu Serif text as the committed export,
and `pdftotext` on the PDF returned "zegarów", "Zwiększ", "brzęczyk".
Set `PLANTUML_URL` to use a server on another port.
Steps 1–3 were run as written on a fresh `git clone` (2026-11-02): all ten
sources rendered, and every `.svg` and `.pdf` was byte-identical to the committed ones.

## Fact table (firmware)

Source of every node in the diagrams. File paths are relative to `firmware/`.

### Interrupts (wake sources)

| Interrupt | Trigger | Handler | What the handler does |
|---|---|---|---|
| RTC1 COMPARE0 | `BOARD_WAKE_APP` deadline | `src/board.c:rtc_evt` | nothing; the IRQ is the wake (nrfx disables the one-shot channel) |
| RTC1 COMPARE1 | `BOARD_WAKE_WAIT` deadline (driver waits) | `src/board.c:rtc_evt` | nothing; wake only |
| RTC1 OVERFLOW | 24-bit counter wrap (512 s) | `src/board.c:rtc_evt` | `s_rtc_overflows++` for `board_ms` |
| GPIOTE PORT (SENSE) | DK button falling edge (P0.13–16) | `src/board.c:on_button` | nothing; `app_poll` reads pin levels |
| GPIOTE PORT (SENSE, toggle) | `DCF_OUT` edge (P0.25), only while a sync window is open | `src/dcf77_sync.c:on_edge` | rise: store time and period; fall: publish one pulse (`s_pending`) or count an overrun |
| GPIOTE IN channel | LIS3DH `INT1` rising (P0.23), only while an alarm sounds | `apps/clock/src/main.c:on_int1` | `s_int1 = true` |
| GPIOTE IN channel | `EPD_BUSY` falling (P0.29), only during an e-paper refresh | `src/epd.c:on_busy` | nothing; `wait_idle` re-reads the pin |
| SWI2 = `SD_EVT_IRQn` | SoftDevice BLE and SoC events | SDK `nrf_sdh.c:SD_EVT_IRQHandler` → `src/ble_sched.c:on_ble_evt`, `nrf_ble_qwr`, `nrf_fstorage_sd` | sets `s_ev_*` flags, answers GATT authorize requests (see BLE facts) |
| UARTE0 | console RX byte (build `SLEEP_UART=1` only) | SDK `app_uart_fifo.c` | byte into the RX FIFO; `poll_uart` reads it |

### Main loop (`apps/clock/src/main.c:main`)

| Step | Function | Facts |
|---|---|---|
| Boot | `main` | `board_safe_state` (DC/DC on, `DCF_PON` high = receiver off, CS lines high, buzzer low), `alarm_init`, `dcf77_sync_init`, `board_button_wake_enable`, `con_init`, `board_time_init` (SoftDevice enable, LFXO, RTC1), `sched_store_init` (load table from flash), `ble_sched_init`, `spi_bus_init`, `lis3dh_power_down`, first `draw`, DCF sync pending (3 tries) |
| Loop | `main` | `app_poll`; then if `s_draw && !s_drawing && !dcf77_sync_active()` → `draw` and loop again; else `board_sleep` |
| Poll | `app_poll` | `poll_uart`, `poll_clock`, `poll_alarm`, `poll_dcf`, `poll_ble`, `arm_wake`, every pass, in this order. No event bits: each handler checks its own state |
| Console | `poll_uart`, `accept_line` | `YYYY-MM-DD HH:MM:SS` sets time; `alarm …` sets a slot then `sched_store_save`; `alarm?` lists; `dcf sync` opens a window; `ble adv` opens advertising |
| Time | `poll_clock` | skipped until time is set (UART or DCF) and while drawing; adds whole elapsed seconds from `board_ms`; on a new minute `alarm_on_minute`; at 03:00 sets DCF sync pending with 3 tries |
| Alarm | `poll_alarm` | `alarm_step` (button stops, 60 s timeout, 1 s on / 1 s off); start → `motion_arm`; `s_int1` → `lis3dh_int1_source` + `motion_count_event` (20 events, 300 ms gap, 30 s window) → `alarm_stop_motion`; stop → `motion_disarm`; buzzer PWM on/off follows the phase; a press edge without an alarm requests a redraw |
| DCF | `poll_dcf` | pending window opens only when no alarm (dropped after 5 min late); `dcf77_sync_step` decodes one pulse; OK → set time, redraw, `alarm_on_clock_set`; failed window → retry after 1 h while tries remain |
| BLE | `poll_ble` | alarm active → `ble_sched_stop`; button held ≥ 2 s and idle → `ble_sched_adv_start`; hold in progress → re-check every 100 ms; `ble_sched_poll` applies a received table |
| Deadline | `arm_wake` | earliest of: next second (`s_last_ms + 1000`), next buzzer phase (`alarm_next_ms`), DCF window end, late DCF drop, DCF retry, BLE link timeout, button-hold check → `board_wake_at(BOARD_WAKE_APP)`; none → `board_wake_cancel` |
| Display | `draw` | `epd_refresh(face_pixel)`; requested only by boot, time set, DCF OK, alarm start and a button press edge — not every minute |

### Peripherals

| Peripheral | Owner | Use | Idle state |
|---|---|---|---|
| RTC1 (LFCLK 32.768 kHz crystal) | `src/board.c:board_time_init`, `board_ms`, `board_wake_at` | free-running ms timebase, CC0 app wake, CC1 wait wake | always running |
| LFCLK, POWER | SoftDevice (`board.c:clocks_start`) | LFXO start, DC/DC via `sd_power_dcdc_mode_set` | running |
| GPIOTE | `board.c`, `dcf77_sync.c`, `main.c`, `epd.c` | buttons and `DCF_OUT` by SENSE (PORT event); `INT1`, `EPD_BUSY` by IN channels | buttons only |
| SPIM0, 250 kHz | `src/spi_bus.c:spi_bus_transfer` | e-paper (mode 0, `EPD_CS`), LIS3DH (mode 3, `SENSOR_CS`) | enabled only per transfer; SCK/MOSI held high |
| PWM0, 2.7 kHz | `src/buzzer.c:buzzer_tone_start`, `buzzer_off` | buzzer tone on `BUZZER_EN` (P0.31) | uninit, pin low |
| UARTE0 115200 | `src/board.c:con_init` (SDK `app_uart`) | debug console | absent in `SLEEP_UART=0` build |
| NVMC via SoftDevice | `src/sched_store.c:sched_store_save` | flash page 0x7F000 | — |
| GPIO `DCF_PON` (P0.24, active low) | `src/dcf77_sync.c:dcf77_sync_open`, `close_window` | receiver power | high = off |
| E-paper SSD1680 | `src/epd.c:refresh_panel` | full refresh, then deep sleep mode 1 (cmd 0x10); `EPD_RST` released | no power switch; panel in deep sleep |
| LIS3DH | `src/lis3dh.c:lis3dh_motion_int1_enable`, `lis3dh_power_down` | 100 Hz, ±2 g, high-pass, INT1 at 256 mg, latched | power-down mode by register (no power switch) |
| SAADC | — | not used (no battery measurement yet) | — |

### Timers and deadlines

| Constant | Value | Where |
|---|---|---|
| Second tick | 1000 ms | `main.c:poll_clock`, `arm_wake` |
| `ALARM_ON_MS` / `ALARM_OFF_MS` | 1000 / 1000 ms | `src/alarm.h`, `alarm.c:alarm_step` |
| `ALARM_TIMEOUT_MS` | 60 s | `alarm.c:alarm_step` |
| `MOTION_N`, `MOTION_GAP_MS`, `MOTION_WINDOW_MS` | 20, 300 ms, 30 s | `src/motion_count.c:motion_count_event` |
| DCF sync start | 03:00 daily and at boot | `main.c:poll_clock`, `main` |
| `DCF77_WINDOW_MS` | 300 s | `dcf77_sync.c:dcf77_sync_step` |
| `DCF_LATE_MS` | 300 s (pending window dropped if alarm still active) | `main.c:poll_dcf` |
| `DCF_RETRIES`, `DCF_RETRY_MS` | 3, 1 h | `main.c:poll_dcf` |
| DCF bit widths | 0: 40–140 ms, 1: 150–260 ms, minute gap 1.7–2.3 s, second 0.8–1.2 s | `src/dcf77_classify.c`, `dcf77_sync.c:on_pulse` |
| `BLE_HOLD_MS`, `BLE_HOLD_POLL_MS` | 2 s, 100 ms | `main.c:poll_ble` |
| `EPD_BUSY_TIMEOUT_MS` | 10 s | `epd.c:wait_idle` |
| RTC wake cap | 500 s (24-bit counter) | `board.c:board_wake_at` |

### DCF-77 decoder (`src/dcf77_sync.c`)

States `ST_OFF` → `ST_WAIT_MARKER` (wait for a pulse after a minute gap) → `ST_RECEIVE`
(59 bits) → `check_frame` (markers bit 0/20/17-18, even parity, BCD range, weekday) →
first frame becomes the candidate, back to `ST_WAIT_MARKER` → second frame must be
candidate + 1 min → `ST_WAIT_START` → next minute-gap pulse gives `DCF77_SYNC_OK` with
`edge_ms` and closes the window. Any bad pulse, overrun or mismatch drops both frames
(`drop_frames`). `close_window` is the only exit: GPIOTE off, `DCF_PON` high.

### Flash writes

| Caller | Function | When |
|---|---|---|
| UART `alarm …` | `main.c:accept_line` → `src/sched_store.c:sched_store_save` | every accepted console edit |
| BLE write | `src/ble_sched.c:ble_sched_poll` → `sched_store_save` | every accepted table write |
| Both | `sched_store_save` | erase page 0x7F000, write record (magic, 113 B, CRC16), waits in `sd_app_evt_wait` for the SoftDevice flash ops |

### Sleep entry and exit (`src/board.c`)

| Step | Function | Facts |
|---|---|---|
| Arm | `main.c:arm_wake` → `board_wake_at` | RTC1 CC0 one-shot; a past target sets `s_wake_due` instead |
| Enter | `board_sleep` | returns at once if `s_wake_due`; clears FPU flags (anomaly 87); `WFE`, `SEV`, `WFE` (System ON, no `sd_app_evt_wait`) |
| Exit | any enabled IRQ above | CPU continues after `WFE`; `main` loop calls `app_poll` |
| Nested sleep | `board_wait_ms`, `epd.c:wait_idle` | sleep on RTC1 CC1 or BUSY, call `app_poll` on each wake so the buzzer keeps its phase during a refresh |
| Never | — | System OFF is not used |

## Fact table (Bluetooth)

Sources: `firmware/src/ble_sched.c`, `sched_codec.[ch]`, `sched_store.c`,
`firmware/apps/clock/config/sdk_config.h`, `firmware/apps/clock/armgcc/*`, `web/app.js`, `web/schedule.js`.

| Topic | Fact | Where |
|---|---|---|
| SoftDevice | S112 7.2.0 (peripheral-only), flash below 0x19000, app RAM from 0x200022c8; flashed once by `make flash_softdevice`, not in the app hex | `armgcc/Makefile`, `clock_gcc_nrf52.ld` |
| SoftDevice owns | radio and link-layer timing, GAP/GATT/ATT, POWER/CLOCK (LFCLK from crystal, 20 ppm), flash erase/write scheduling | `sdk_config.h` `NRF_SDH_*`, `board.c:clocks_start` |
| App → SoftDevice | `sd_*` calls (SVC): `sd_ble_gap_adv_set_configure`, `sd_ble_gap_adv_start/stop`, `sd_ble_gatts_rw_authorize_reply`, `sd_ble_gap_disconnect`, `sd_power_dcdc_mode_set`, flash via `nrf_fstorage_sd` | `ble_sched.c`, `board.c`, `sched_store.c` |
| SoftDevice → app | events via SWI2 (`SD_EVT_IRQn`, `NRF_SDH_DISPATCH_MODEL 0` = interrupt) → `NRF_SDH_BLE_OBSERVER` `on_ble_evt` (prio 3), `nrf_ble_gatt` (1), `nrf_ble_qwr` (2); SoC flash events → `nrf_fstorage_sd` | `nrf_sdh.c`, `ble_sched.c:on_ble_evt` |
| Events handled | `GAP_EVT_CONNECTED`, `DISCONNECTED`, `ADV_SET_TERMINATED`, `PHY_UPDATE_REQUEST` (auto), `SEC_PARAMS_REQUEST` (pairing not supported), `GATTS_EVT_SYS_ATTR_MISSING`, `GATTS_EVT_RW_AUTHORIZE_REQUEST` | `ble_sched.c:on_ble_evt` |
| Advertising | starts only on 2 s button hold (or console `ble adv`) and no alarm; connectable scannable undirected, 100 ms interval, 60 s duration, 1M PHY; adv data = flags + name `ShakeClock`; scan response = 128-bit service UUID | `main.c:poll_ble`, `ble_sched.c:ble_sched_adv_start`, `ble_sched_init` |
| UUIDs | vendor base `e1430d6d-6e9b-4b29-ba99-6f30cbe203d0`; service `e1430001-…`, characteristic `e1430002-…` (bytes 12–13 replaced) | `ble_sched.c:s_base`, `web/schedule.js` |
| Characteristic | read + write, open security, value in stack, `rd_auth` + `wr_auth`, 113 B | `ble_sched.c:ble_sched_init` |
| Format | byte 0 = version 1, then 7 × 8 u16 LE, Monday first; bit 15 = enabled, bits 0–10 = minute of day | `sched_codec.h`, `web/schedule.js:encode` |
| Connection | one link; preferred interval 100–200 ms, latency 0, supervision 4 s; ATT MTU up to 247, data length 27 | `ble_sched.c:ble_sched_init`, `sdk_config.h` |
| Read | authorize-read → `alarm_table_get` + `sched_encode` of the live table → reply 113 B | `ble_sched.c:on_authorize` |
| Write (fits MTU) | authorize-write → `validate` (`sched_decode`: length, version, minute ≤ 1439, no duplicate enabled time in a day; busy while the previous write is not yet applied) → reply status; OK → copy to `s_stage`, `s_stage_ready` | `ble_sched.c:on_authorize`, `validate` |
| Long write (MTU too small) | prepare writes buffered by `nrf_ble_qwr` (256 B); at execute `qwr_cb` gets AUTH_REQUEST with the whole value → `validate` into `s_pend`, then EXECUTE_WRITE → `s_stage`, `s_stage_ready` | `ble_sched.c:qwr_cb` |
| Error codes (ATT) | 0x0D wrong length, 0x82 version, 0x83 minute, 0x84 duplicate, 0x90 busy | `ble_sched.c:status_for`, `ERR_BUSY` |
| Apply and store | main loop `ble_sched_poll` → `alarm_table_replace` → `sched_store_save` (erase page 0x7F000, write record: magic, 113 B, CRC16; waits in `sd_app_evt_wait`) | `ble_sched.c:ble_sched_poll`, `sched_store.c` |
| Link timeouts | disconnect 60 s after connect without an accepted write, 30 s after the last accepted write; alarm start drops adv and link | `ble_sched.c:deadline`, `ble_sched_poll`, `main.c:poll_ble` |
| Web page | `requestDevice` (filter name + service) → `gatt.connect` → `getPrimaryService` → `getCharacteristic` → `readValue` → `decode`; upload = `writeValueWithResponse(113 B)` then `readValue` and byte compare; `gattserverdisconnected` → state "disconnected" | `web/app.js:connect`, `upload`, `onDisconnected` |
| MTU / long write choice | made by the browser and OS BLE stack, not by the page: 113 B fits one Write Request when ATT MTU ≥ 116, else Prepare/Execute Write | `web/app.js:upload`, ATT spec |

## Consistency walk (L1 → L2 → L3)

Checked 2026-11-02 that the levels use the same names and states.

| Step | L1 `l1-overview` | L2 `l2-modules` | L3 |
|---|---|---|---|
| **Alarm cycle** | | | |
| Minute matches a slot | Cisza → Alarm, "minuta zgodna z rozkładem" | `2. poll_clock` → `alarm` | B: `alarm_on_minute()` |
| Sounding | Alarm: PWM 1 s / 1 s, LIS3DH, INT1; BLE → Radio wyłączone; DCF waits | `3. poll_alarm` → `alarm`, `lis3dh`, `buzzer`; `5. poll_ble` → `ble_sched` | C: `motion_arm`, PWM0 2,7 kHz; B: `ble_sched_stop`; D: "czekaj; po 5 min porzuć okno"; A: `arm_wake` adds `alarm_next_ms` |
| Shake | wake GPIOTE IN: INT1 | GPIOTE `on_int1` ⇢ `3. poll_alarm` → `motion_count` | C: `INT1_SRC`, `motion_count_event` (300 ms, 20 in 30 s) |
| Stop | Alarm → Cisza: przycisk, 20 ruchów, 60 s | `3. poll_alarm` → `alarm`, `lis3dh`, `buzzer` | C: `alarm_stop_motion`, `motion_disarm`, PWM0 stop |
| **DCF sync** | | | |
| 03:00 | Odbiornik DCF wyłączony → Okno DCF, "tylko bez alarmu" | `2. poll_clock` "03:00 → DCF"; `4. poll_dcf` → `dcf77_sync` | B: "DCF oczekuje, 3 próby"; D: `dcf77_sync_open()` (DCF_PON = 0) |
| Pulses | wake GPIOTE PORT: DCF_OUT | GPIOTE `on_edge` ⇢ `dcf77_sync` → `dcf77_classify` | E: `on_edge` note, WAIT_MARKER → RECEIVE → `finish_frame` |
| Two frames agree | Okno DCF → Odbiornik DCF wyłączony | `dcf77_sync` → Odbiornik DCF-77 (PON) | E: WAIT_START → OFF, `close_window()`; D: "Wynik OK" |
| Time set | Praca CPU → Odświeżanie e-papieru (after the window closes) | `4. poll_dcf` → `alarm`; `main` → `draw` | D: `s_draw = true`, `alarm_on_clock_set()`; A: draw condition "okno DCF zamknięte"; F |
| Failure | Okno DCF → wyłączony after 5 min; retry after 1 h | `4. poll_dcf` "okno, ponowienia" | E: "5 min od otwarcia"; D: "próby − 1; ponowienie za 1 h" |

## Spec-vs-code mismatches

The diagrams follow the code. Each row is a place where `openspec/specs/` says
something else. Recorded only; fixing them is a separate change.
"Certain" = read in the code; "likely" = follows from the code but not run on hardware.

| # | Spec → requirement | Spec says | Code does | Evidence | Status |
|---|---|---|---|---|---|
| 1 | `ble-schedule-service` → GATT service layout | service UUID `e1430d6d-…` | service `e1430001-…`, characteristic `e1430002-…`; `e1430d6d-…` is only the vendor base (web uses the real ones) | `ble_sched.c:s_base`, `SERVICE_UUID16`; `web/schedule.js` | certain |
| 2 | `ble-schedule-service` → connection policy | disconnect 30 s after the last accepted **or rejected** write | only an accepted write restarts the 30 s timer | `ble_sched.c:ble_sched_poll` (`s_wrote` only on `s_stage_ready`) | certain |
| 3 | `ble-schedule-service` → advertising window | 2 s hold during an alarm only silences it | hold timer is not read while the alarm runs; if the button stays down 2 s after the silence, advertising starts | `main.c:poll_ble`, `board.c:board_button_held_ms` | likely |
| 4 | `dcf77-time-sync` → retry after a failed window | retry at the next full hour | retry 1 h after the window **closed** (03:05 → 04:05 → 05:10 …) | `main.c:poll_dcf` (`s_dcf_retry_ms = now + DCF_RETRY_MS`) | certain |
| 5 | `dcf77-time-sync` → operator can open a window | `dcf sync` uses the scheduled path | console opens the window directly, without the alarm check; a failed manual window uses the scheduled retries | `main.c:accept_line` | likely |
| 6 | `alarm-buzzer-timing` → no button, no stop before timeout | stop 59 s after the first burst | `ALARM_TIMEOUT_MS` = 60 s (last burst 58–59 s, then 1 s silence); `alarm-schedule` says 60 s, so the two specs disagree | `alarm.h`, `alarm.c:alarm_step` | certain |
| 7 | `alarm-buzzer-timing` → pattern | `BUZZER_EN` 1 s on, 1 s off | "on" second is a 2,7 kHz PWM tone, not a steady level (timing matches) | `buzzer.c:buzzer_tone_start` | certain |
| 8 | `alarm-schedule` → UART alarm commands | `alarm?` lists slots that are enabled **or have a non-zero time** | lists enabled slots only | `main.c:accept_line`, `poll_ble` | certain |
| 9 | `display-refresh-policy` → button press refreshes the face | face shows time, weekday **and alarm status** | face draws weekday, HH:MM and seconds; no alarm status | `clock_face.c:clock_face_black` | certain |
| 10 | `display-refresh-policy` → refresh triggers | no boot refresh listed | `main` refreshes once at boot | `main.c:main` | certain |
| 11 | `display-refresh-policy` → alarm start and time changes refresh; no refresh in a DCF window | a request during a refresh is kept; no refresh inside a window | `draw` clears `s_draw` at the end, so a request raised inside the refresh (via `app_poll` in the waits) is lost; `poll_dcf` can open a window during a refresh | `main.c:draw`, `epd.c:wait_idle` | likely |
| 12 | `low-power-sleep-policy` → wake sources limited | RTC1 1 s tick, buttons, INT1, DCF edge, alarm and e-paper timers | no free-running 1 s tick: one RTC1 compare at the earliest deadline; UARTE0 and SoftDevice (SWI2) also wake | `main.c:arm_wake`, `board.c:board_wake_at` | certain |
| 13 | `low-power-sleep-policy` → peripherals powered only while used | UART does not keep the MCU awake when idle | default build `SLEEP_UART=1` keeps UARTE0 RX on; only `SLEEP_UART=0` removes it | `board.c:con_init`, `armgcc/Makefile` | certain |
| 14 | `low-power-sleep-policy`, `motion-dismiss` → powered down | accelerometer (and display) "off" / "powered down" | LIS3DH power-down by register, e-paper deep sleep by command; no supply switch. Only the DCF receiver is power-gated (PON) | `lis3dh.c:lis3dh_power_down`, `epd.c:refresh_panel` | certain |
| 15 | `idle-pin-states` → peripheral traffic unchanged | accelerometer passes its identity check at start | clock app never calls `lis3dh_who_am_i` (only bring-up does) | `lis3dh.c`, `main.c:main` | certain |
| 16 | `ble-schedule-service` → GATT service layout ("advertise the name and the service UUID") | UUID advertised | UUID is in the scan response; the advertising packet has flags + name | `ble_sched.c:ble_sched_init` | wording |

Other as-built notes (no spec covers them):

- `sdk_config.h` says the DCF receiver uses "one IN channel"; `dcf77_sync_open` asks for `hi_accuracy = false`, i.e. GPIOTE PORT/SENSE.
- `firmware/docs/software-flowchart.puml` lists `DCF_OUT` under "GPIOTE IN" and says the e-paper and LIS3DH are "bez zasilania"; see rows 12 and 14.
- `epd.c:refresh_panel`: a BUSY timeout returns before the deep-sleep command 0x10, so on that path the panel is not put to sleep (likely; not run).
