## 1. Build prerequisites

- [x] 1.1 Enable `NRFX_GPIOTE_ENABLED` (and `GPIOTE_ENABLED` if the legacy path needs it) in `firmware/apps/clock/config/sdk_config.h`; add `nrfx_gpiote.c` to `firmware/apps/clock/armgcc/Makefile`; add `dcf77_sync.c` and `dcf77_classify.c` to the sources
- [x] 1.2 Build `firmware/apps/clock` (`make`) and confirm it still links and the clock/alarm/e-paper behave as before on the DK

## 2. Sync module (`firmware/src/dcf77_sync.{c,h}`)

- [x] 2.1 Edge capture on `DCF_OUT` through `nrfx_gpiote` (toggle, one pending slot, overrun count); width and period from `board_ms()`; classify with `dcf77_classify_width` / `dcf77_is_minute_gap`
- [x] 2.2 Frame assembly: wait for a minute gap, collect 59 bits, abort to wait-marker on an invalid pulse or a period that is not about 1 s
- [x] 2.3 Frame checks: bit 0 = 0, bit 20 = 1, even parity at 28/35/58, BCD ranges for minute/hour/day/weekday/month/year, bits 17/18 not equal; reasons `parity`, `marker`, `range`, `pulse`
- [x] 2.4 Pairing: second frame must be exactly one minute after the first (via `clock_add_seconds`); otherwise reason `mismatch` and both frames cleared
- [x] 2.5 After the second valid frame wait for the minute-start edge and return `ok` with decoded minute, seconds 0 and `edge_ms`
- [x] 2.6 `dcf77_sync_open`, `dcf77_sync_step`, `dcf77_sync_active`; one `close()` path that disables the GPIOTE channel and drives `DCF_PON` high; 5 min window timeout with reason `timeout`
- [ ] 2.7 Commit: `firmware: add DCF-77 frame decoder and sync window module`

## 3. Clock app integration (`firmware/apps/clock/src/main.c`)

- [x] 3.1 Call `dcf77_sync_step` from `app_poll`; on `ok` set `s_time`, `s_last_ms = edge_ms`, `s_set = true`, `s_draw = true`, run the alarm clock-set hook, print `dcf: ok <before> -> <after>`
- [x] 3.2 Print `dcf: rejected <reason> (time unchanged <time>)` and `dcf: timeout (time unchanged <time>)`
- [x] 3.3 Parse `dcf sync` in `accept_line` and open a window; refuse and print a message if one is already open
- [x] 3.4 Schedule: window at 03:00, one at start while time is not set, hourly retry after a failed window, at most 3 retries, then idle until 03:00; skip while `alarm_active()`, drop a window more than 5 min late
- [x] 3.5 Update `firmware/apps/clock/README.md` with `dcf sync`, the wiring from the generator, and the schedule constants
- [x] 3.6 Commit: `firmware: sync clock from DCF-77 daily and on dcf sync command`

## 4. Bench verification with the Nucleo generator

- [ ] 4.1 Wire generator `PA0` -> 1 kΩ -> `P0.25`, GND common; flash the generator; set a known time over its UART
- [ ] 4.2 `dcf sync` with valid frames: clock time and weekday equal the generator, one `dcf: ok` line with before/after, PON high afterwards
- [ ] 4.3 Second boundary: Saleae on `PA0` and a clock-second marker, or the UART second line stamp, confirm the clock second changes within 50 ms of the pulse start
- [ ] 4.4 Generator key `p` (parity) and the wrong-date key: time unchanged, log reasons `parity` and `mismatch`
- [ ] 4.5 Generator key `s` (silence): `dcf: timeout` after 5 min, time unchanged, PON high
- [ ] 4.6 Schedule: set the clock to 02:59:30, confirm the window opens at 03:00 and, with silence, retries at 04:00, 05:00, 06:00 and then stops
- [ ] 4.7 Alarm at the window time: window opens after the alarm stops
- [ ] 4.8 Window current from the stage 5 setup if available; otherwise state in the note that it is taken by stage 5
- [ ] 4.9 Write `notes/log/YYYY-MM-DD_dcf-periodic-sync.md` from `notes/TEMPLATE.md` with commands, results, failures and decisions; commit: `docs: log DCF-77 periodic sync bench results`
