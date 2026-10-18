# Design

## Context

- Clock app (`firmware/apps/clock/src/main.c`) is a polled loop. `app_poll()` calls `poll_uart`, `poll_clock`, `poll_alarm`. Civil time is `s_time` advanced from `s_last_ms`, which comes from `board_ms()` (RTC1 on the LFXO through nrfx). There is no sleep yet (stage 5).
- Shared modules live in `firmware/src/` (`clock`, `alarm`, `board`, `dcf77_classify`). `dcf77_classify` only maps pulse width and period to bit/gap. The real decoder (frame assembly, parity, fields) exists only as the bring-up test log and in the generator encoder; it does not exist in product code.
- Product firmware reaches peripherals only through nrfx/SDK (`firmware-sdk-drivers` spec). `bringup` still uses raw `NRF_GPIOTE` in its DCF test; that code is not reused.
- Receiver pins: `DCF_PON` P0.24 (active low, high = off), `DCF_OUT` P0.25, active-high, no pull (`boards/dk_breadboard_pins.h`).
- Bench facts: `OUT` lags the real pulse about 34 ms; widths 108/206 ms; a few invalid pulses while the receiver locks (`firmware/tools/dcf77-generator/README.md`). The generator has a fault key for a wrong date with correct parity.
- See proposal.md for motivation. Stage 6 needs stage 5, but this change is written against the polled loop so it can land first; stage 5 adopts it (see Risks).

## Goals / Non-Goals

**Goals:**
- A `dcf77_sync` module in `firmware/src/` that owns edge capture, frame decode, window state and PON, with a small poll interface the clock app drives.
- Accept a time only from two consistent frames; leave the time unchanged otherwise.
- Verified end to end against the Nucleo generator, including parity and wrong-date faults.

**Non-Goals:**
- Sleeping between edges, window current CSV, or a measurement setup (stage 5 owns them).
- Plausibility windows against the RTC, leap-second or DST-announcement handling, signal-quality metrics.
- Host unit tests for the decoder (author decision); verification is on the bench with the generator.
- Runtime-configurable schedule; times are compile-time constants.

## Decisions

1. **Module and interface.** New `firmware/src/dcf77_sync.{c,h}`, built into `apps/clock` only. Interface: `dcf77_sync_init()`, `dcf77_sync_open(now_ms)` (manual or scheduled start), `dcf77_sync_step(now_ms, &result)` called from `app_poll()`, `dcf77_sync_active()`. `result` carries outcome (`none`, `ok`, `rejected` + reason enum), decoded `clock_time`, and the edge timestamp `edge_ms`. The module never touches `s_time`; `main.c` applies a result, so time ownership stays in one place. Alternative: module sets the time itself via callbacks. Rejected: second owner of `s_time`.

2. **Edge capture via nrfx_gpiote.** Toggle IN event on `DCF_OUT`; the handler stores `board_ms()` and level in a one-slot variable (same pending-pulse idea as bring-up, overrun counted). Width = fall - rise; period = rise - previous rise. Classification reuses `dcf77_classify_width` and `dcf77_is_minute_gap`. Alternative: poll `DCF_OUT` in `app_poll`. Rejected: e-paper refresh blocks the loop for seconds, pulses would be missed; also contradicts the interrupt-driven thesis story. `board_ms()` is RTC1 ticks (about 30 µs); that is enough for 100/200 ms pulses.

3. **Frame state machine.** States: `OFF`, `WAIT_MARKER` (wait for a minute gap, discard partial data), `RECEIVE` (collect 59 bits, indexed by second count from the marker), `WAIT_START` (frame 2 or the post-frame boundary pulse), `OFF` again. Any invalid pulse or a missed/extra second (period not about 1 s) aborts the current frame and returns to `WAIT_MARKER`. After a frame passes checks (bit 0 = 0, bit 20 = 1, parity 28/35/58 even, BCD digits and ranges, bits 17 xor 18) it is stored as candidate; a second candidate must be exactly one minute later (use `clock_add_seconds(&t, 60)`), else the pair is dropped with reason `mismatch` and the new frame becomes the first candidate only if it passed checks on its own. The spec requires no reuse of the rejected frame; implementation therefore clears both and waits for a fresh marker. Reason enum: `parity`, `marker`, `range`, `pulse`, `mismatch`, `timeout`.

4. **Acceptance and phase.** A frame encodes the minute that starts at the next minute mark. After frame 2 passes, the module waits for the next pulse start (the minute-start edge, about 1-2 s later), then reports `ok` with `time = decoded minute, sec = 0` and `edge_ms` = that edge. `main.c` sets `s_time` and `s_last_ms = edge_ms`, so the clock second boundary equals the pulse start (no 34 ms compensation, spec: within 50 ms). The receiver stays on until this edge, then PON goes high. Alternative: set the time at frame end and add 1 s of arithmetic. Rejected: `edge_ms` alignment is one assignment and is the reason to wait.

5. **Schedule and retry.** `main.c` checks each minute (inside `poll_clock`) whether `hour == 3 && min == 0` or a retry hour is due. Constants: `DCF_SYNC_HOUR 3`, `DCF_WINDOW_MS 300000`, `DCF_RETRIES 3`. After a failed window the next window time is the next full hour; after 3 failed retries the schedule goes idle until 03:00. Also one window at start when `!s_set`. State kept in `main.c`: `s_dcf_retries_left`, `s_dcf_pending`. Alternative: schedule inside `dcf77_sync`. Rejected: needs civil time, which `main.c` owns. Alarm precedence (spec) = `if (alarm_active()) keep pending, retry the check each minute`; drop the window if more than 5 minutes late.

6. **PON gating.** `board_safe_state()` already drives `DCF_PON` high. `dcf77_sync_open` sets PON low and enables the GPIOTE channel; every exit path (ok, reject, timeout, abort) goes through one `close()` that disables the channel and sets PON high. Alternative: leave the GPIOTE channel enabled while off. Rejected: the carrier couples spikes onto idle wires; unneeded wakeups later.

7. **Operator interface.** `dcf sync` is parsed in `accept_line` before the clock parser; it calls `dcf77_sync_open`. Every window ends with one line: `dcf: ok <before> -> <after>`, `dcf: rejected <reason> (time unchanged <time>)`, or `dcf: timeout (time unchanged <time>)`. `main.c` prints; the module only returns data.

8. **Verification with the generator.** Bench: Nucleo `PA0` -> 1 kΩ -> `P0.25` with common GND (as in the generator README), DCF-1060N not needed on the line. Cases: valid sync from a set generator time; `p` (parity) and wrong-date fault; `s` (silence) -> timeout; 5 min window length; PON level on the analyzer or meter after each. Notes entry under `notes/log/`.

## Risks / Trade-offs

- **Stage ordering.** Stage 6 is planned after stage 5 (sleep). This change keeps the polled loop, so when stage 5 introduces sleep, `dcf77_sync_step` must still be called on each wake; GPIOTE edges are already wake sources. Risk is low; the interface does not assume polling cadence faster than 1 s.
- **Blocking e-paper refresh.** A refresh (seconds) can overlap edges. The ISR stores timestamps, but one pending slot loses a pulse if `app_poll` is not called for more than about 1 s. Mitigation: overruns count as `pulse` rejection, so a lost second aborts that frame instead of corrupting it; the retry covers it. Window retries during a refresh-heavy minute are acceptable (one refresh per minute).
- **Two-frame cost.** About one more minute of receiver-on time per sync than one frame. Accepted for the wrong-date case; chapter 5 should report both.
- **First pulses after PON low are noisy** (lock time, 4 invalid pulses seen). They only delay the first `WAIT_MARKER`; the 5-minute window has room for it.
- **No host test.** Decoder bugs show up only on the bench. Mitigation: the generator fault keys cover parity, wrong date, and silence.
