# 2026-10-28 — deep-sleep current: configurations A, B, C and the four states

Close-out runs for tasks 4.2-4.6 of `add-deep-sleep-and-current-measurement`.
History, failed attempts and the pin-leak fixes that led here are in
[`2026-10-24_deep-sleep-current.md`](2026-10-24_deep-sleep-current.md)
(runs A-A8, C2-C6f).

## Conditions (all runs)

- Firmware: clock app at `53ccb2c` (last firmware change; same build as C6f).
  `SLEEP_UART=0` for A9, B1, C8; UART build for B2, D, C7 (the only way to
  set an alarm is the UART `alarm` command). Repo HEAD while packing:
  `750b3a7`.
- DK PCA10040 v3.0.1, SB20/SB21/SB26/SB27 cut (A8), LIS3DH breakout "ON" LED
  removed (C6f), breakout fed on `VIN`.
- PPK2 S/N `FACEF4EE8A0A` (the project's only PPK2), Data Logger,
  100 kS/s. Room 23 °C.
- Logic: `D0` `P0.22` (`CLOCK_SEC_MARK`), `D1` `P0.24` (`DCF_PON`, active
  low), `D2` `P0.31` (`BUZZER_EN`), `D3` `P0.29` (`EPD_BUSY`). In A9 `D1` and
  `D2` are swapped: `D2` falls with the window start and rises at its end,
  `D1` falls at init and stays low like `BUZZER_EN` `[INFERENCE]`.
- All windows below come from 10 ms bin means of the packed captures.
  Margins of 1 s are left at each state edge for idle figures.

| Run | Capture | Config | PPK2 mode, supply | Build | Length |
| --- | --- | --- | --- | --- | --- |
| A9 | [`ppk-20261028T174816`](../measurements/ppk-20261028T174816.ppkz) | A, nRF alone on `P22` | Source Meter 3.0 V | `SLEEP_UART=0` | 360 s |
| B1 | [`ppk-20261028T183744`](../measurements/ppk-20261028T183744.ppkz) | B, peripheral rail only | Ampere Meter, DK `VDD` (~3.0 V) | `SLEEP_UART=0` | 360 s |
| B2 | [`ppk-20261028T185453`](../measurements/ppk-20261028T185453.ppkz) | B | Ampere Meter, DK `VDD` | UART | 180 s |
| D | [`ppk-20261028T191801`](../measurements/ppk-20261028T191801.ppkz) | buzzer `+` and flyback diode only | Ampere Meter, DK `5V` | UART | 180 s |
| C7 | [`ppk-20261028T194655`](../measurements/ppk-20261028T194655.ppkz) | C, one rail for nRF and peripherals | Source Meter 3.0 V | UART | 180 s |
| C8 | [`ppk-20261028T195828`](../measurements/ppk-20261028T195828.ppkz) | C | Source Meter 3.0 V | `SLEEP_UART=0` | 360 s |
| A10 | [`ppk-20261028T212701`](../measurements/ppk-20261028T212701.ppkz) | A | Source Meter 3.0 V | `SLEEP_UART=0`, firmware `d0685ee` | 240 s |
| A9b | [`ppk-20261028T205549`](../measurements/ppk-20261028T205549.ppkz) | A | Source Meter 3.0 V | `SLEEP_UART=0`, pre-`d0685ee` spin fix | 240 s |
| C9 | [`ppk-20261028T232632`](../measurements/ppk-20261028T232632.ppkz) | C, panel on | Source Meter 3.0 V | `SLEEP_UART=0`, firmware `c6faefd`, DCF generator on `P0.25` | 360 s |

Wiring photos: C is
[`2026-10-25_deep-sleep_ppk2-C.jpg`](../media/2026-10-25_deep-sleep_ppk2-C.jpg);
A and B are [`2026-10-28_measurement_A.jpeg`](../media/2026-10-28_measurement_A.jpeg) and
[`2026-10-28_measurement_B.jpeg`](../media/2026-10-28_measurement_B.jpeg).

## Results

### A9, nRF alone

| Phase | Window | Current |
| --- | --- | --- |
| DCF window (`D2` low) | 11.3-146.9 s | 291.0 µA (1 s means 290.5-291.9) |
| Redraw, no panel: `EPD_BUSY` timeout | 9.0 s each, 4 times | 3.33 mA, 30.1-30.5 mC each |
| Idle between minutes | 4 windows of 49 s | **2.07 µA** (1 s means 2.00-2.11) |

Against the nRF52832 PS v1.9 (section 17.1.1, 3 V, 25 °C, DC/DC on): the
expected figure for System ON, full RAM, RTC wake is about 1.9-2.2 µA
(`ION_RAMOFF_RTC` 1.9 µA plus the 0.3 µA RAM-retention step). 2.07 µA is
inside that range. The first A runs (A5 0.81 µA) were below the datasheet
because the interface MCU back-fed `VDD_nRF`; with SB20/21/26/27 cut that
path is gone and A is now a valid nRF figure.

### A10 and A9b, nRF alone after the refresh spin fix (`d0685ee`)

Firmware `d0685ee` removes `snprintf` from the per-pixel face path and skips
the 1 s clock wake deadline while `s_drawing`. SWD PC samples during the boot
refresh then show about 0.75 s of drawing and SPI, then `board_sleep` for the
rest of the BUSY wait (no panel on A).

| Phase | Run | Window | Current |
| --- | --- | --- | --- |
| Boot draw plus BUSY wait, no panel | A10 | 3.30-3.73 s | **2.58 mA** mean, 1.11 mC (not a 9 s plateau) |
| DCF window (`D1` low; only `D0` on logic) | A10 | 3.73-154.34 s | **291.3 µA** (1 s means 290.9-292.3) |
| Idle after sync | A10 | 154.34-204.22 s | **2.15 µA** (1 s means 1.9-2.5) |
| DCF retry window | A10 | 204.24-214.25 s | 291.1 µA |
| Idle | A10 | 214.25-240.00 s | 2.20 µA |


### A11: DCF window on PORT/SENSE (`c6faefd`)

`ppk-20261028T225102` ([`.ppkz`](../measurements/ppk-20261028T225102.ppkz)),
configuration A, ribbon unplugged, only `D0` on logic, `SLEEP_UART=0`,
240 s, generator on `P0.25`. The UART build had printed `dcf: window open`
before this run.

| Phase | Window | Current |
| --- | --- | --- |
| Boot refresh, then BUSY timeout | 0-20 s | 544 µA mean of first 10 s, 291.6 µA at 10-19 s |
| DCF window open | 20-190 s | **2.1-2.7 µA** (10 s means) |
| Sync accepted, refresh BUSY timeout | 190-200 s | 291.7 µA |
| Idle, clock running | 200-240 s | 2.3-2.5 µA |

`D0` first toggles at 197.09 s, then every 1.00 s: time was set from DCF.

A9b (same wiring intent, firmware before `clock_face` fix): three refreshes at
**4.07 mA** for 7.5-7.9 s each (flat plateau, CPU spin). Idle after sync was
**369 µA**, not 2 µA, with extra PPK2 logic lines still connected; see item 5
below.

**PPK2 logic on configuration A:** for A10 the author removed all PPK2 logic
leads except **`D0`** (`CLOCK_SEC_MARK`). With `D1`-`D3` attached, the nRF
stayed powered when the PPK2 source was off (leak from the DK side). That
fight is **not analysed here**; solder-bridge research is tracked separately.

### B1, B2, peripherals alone

| Phase | Run | Window | Current |
| --- | --- | --- | --- |
| Idle | B1 | 227.6-277.7, 289.7-337.7 s | **52.6 µA** (1 s means 47.5-57.7) |
| Idle, UART build | B2 | 1-46, 116-166 s | 54.0 µA |
| DCF window (`D1` low) | B1 | 18.7-218.8 s (200 s; 178 s used after the boot refresh) | 129.7 µA |
| Minute refresh | B1 | 7.7, 10.0, 7.7 s | 0.70-0.86 mA mean, 6.6-7.0 mC each, 10 ms peaks 10.6 mA |
| Alarm, buzzer phases (refresh excluded) | B2 | 58.4-106.4 s | 89.8 µA |

- Receiver cost in the window: 129.7 - 52.6 = 77 µA.
- Alarm cost on the peripheral rail: 89.8 - 54.0 = 36 µA (LIS3DH active
  with INT1 armed).

### C7, C8, shared rail

| Phase | Run | Window | Current |
| --- | --- | --- | --- |
| Idle | C8 | 3 windows of 49-50 s | **74.6 µA** (1 s means 74.5-74.9) |
| DCF window (`D1` low) | C8 | 21.5-172.1 s (150.5 s) | 425.8 µA |
| Minute refresh | C8 | 8.7 and 7.7 s (one more at 8.0 s cut by the file end) | 6.87 and 6.15 mA, 59.7 and 47.5 mC |
| Idle, UART build | C7 | 1-50, 122-170 s | 1.281 mA |
| Alarm, start refresh plus 60 s buzzer | C7 | 50.9-109.9 s (59.0 s) | 2.859 mA |
| Alarm, buzzer phases only | C7 | 62.0-109.9 s | 2.136 mA (1 s means 1.75 off, 2.52 on) |

- C8 idle equals C6f (74.7 µA), so the setup reproduces.
- The UART build costs 1.281 - 0.075 = 1.21 mA in idle. Alarm figures in C
  are therefore taken as C7 minus 1.21 mA.
- Alarm cost on the shared rail (buzzer phases): 2.136 - 1.281 = 0.85 mA.
  B2 shows 0.04 mA of it on the peripheral rail, so about 0.82 mA is the
  nRF. 1 s means are 1.75 mA off and 2.52 mA on, so the on phase adds about
  0.8 mA: PWM0 at 2.7 kHz (which keeps HFCLK running) plus `BUZZER_EN`
  driving the NPN base through 1 kΩ, about 2.3 mA while high, 50 % duty
  `[INFERENCE]`.

### C9, shared rail on `c6faefd` (first whole-device run after both fixes)

| Phase | Window | Current |
| --- | --- | --- |
| Boot refresh | 0.59-4.72 s | 4.37 mA mean, 18.1 mC |
| DCF window open | 10-160 s | **138.3 µA** (1 s means 138.2-138.4) |
| Refresh (first after sync, then at 60 s spacing) | 169.4, 229.3, 289.3, 349.3 s | 4.50 mA mean, **16.73-16.75 mC each**, 3.7 s |
| Idle, clock running | between refreshes | **74.7 µA** |

- The refresh is not a CPU spin: the shape is about 5.7 mA for 0.8 s, 4.2 mA
  for 1.6 s, 3.9 mA for 0.2 s, 5.7 mA for 0.7 s, then 75 µA. Four refreshes
  repeat within 0.1 mC.
- DCF window: 138.3 - 74.7 = 63.6 µA above idle (B1 receiver cost: 77 µA).
  The 425.8 µA of C8 was the GPIOTE IN channel (`c6faefd`).
- **Refresh charge is 16.7 mC, about twice the 7.9 mC of the revised model.**
  B1 panel charge (6.8 mC) plus the A10 nRF spike (1.1 mC) does not add up
  to it. Cause open.

### D, buzzer on `5V`

| Phase | Window | Current |
| --- | --- | --- |
| Off | 1-58, 119-180 s | 0.06 µA (meter floor) |
| Alarm, 1 s on / 1 s off for 59.0 s | 59.4-118.4 s | **7.08 mA** mean, 418 mC per alarm |
| First on phase | 59.4-60.4 s | 13.9 mA |

The on phase draws about 14 mA, not the about 50 mA assumed in AGENTS.md:
PWM0 drives `BUZZER_EN` with a 2.7 kHz square wave (`buzzer.c`, `TONE_TOP`
370 at 1 MHz), so the coil sees about 50 % duty inside the on phase.

## C against A + B

| State | A | B | A + B | C | C - (A + B) |
| --- | --- | --- | --- | --- | --- |
| Idle | 2.07 µA | 52.6 µA | 54.7 µA | 74.6 µA | +19.9 µA (27 %) |
| DCF window | 291.0 µA | 129.7 µA | 420.7 µA | 425.8 µA | +5.1 µA (1.2 %) |
| Refresh | 3.33 mA (no panel, timeout) | 0.70-0.86 mA | about 4.1 mA | 6.15-6.87 mA | not comparable, see below |
| Alarm | not measured | 36 µA over idle | | 0.85 mA over idle | nRF share about 0.82 mA |

- DCF window: C matches A + B within 1.2 %.
- Idle: C is 20 µA above A + B. In B the nRF sits on DK `VDD` and the
  peripherals sit behind the PPK2 Ampere Meter, whose shunt drops the rail a
  little below the nRF. The nRF drives `SCK`, `MOSI` and the chip selects
  high into the breakout's 10 kΩ level-shifter pull-ups; C6e showed that
  this path can power the breakout by itself. Part of the peripheral idle
  current then flows from the nRF pins, past the meter, so B reads low
  `[INFERENCE]`. C has one rail and no such path, so C is the figure to use.
- Refresh: A has no panel, so `EPD_BUSY` never falls and A9 shows a 9 s
  timeout instead of a real refresh. The two rows describe different
  firmware paths and are not summed.

## What failed or is open

1. **E-paper refresh CPU spin (fixed in `d0685ee`).** A9 and A9b showed a flat
   3.33-4.07 mA for the full BUSY wait because `clock_face_black()` called
   `snprintf` twice per pixel (~250k calls per refresh). After the fix, A10
   shows a **0.43 s** boot spike at 2.58 mA mean (1.11 mC) and then sleep
   during the rest of the wait. C9 re-measured the whole device: refresh is
   3.7 s and 16.7 mC. The B1 + A10 sum (about 8 mC) does not explain it;
   candidates are the regulator, LIS3DH breakout and DCF module on the same
   rail during the refresh. To check: read the B1 refresh charge from its pack.
2. **DCF window drew about 290 µA (fixed in `c6faefd`).** A9 had 291 µA,
   C8 minus B1 gave 296 µA, and C2 and C6e showed the same (0.34 mA). Cause:
   `dcf77_sync.c` used `SENSE_TOGGLE(true)`, a GPIOTE IN channel (`CONFIG[0]`
   active, SWD read 2026-10-28). With `false` (PORT/SENSE) the window drew
   2.1-2.7 µA in A11. The 2026-10-24 runs A and A5 had 4-10 µA with `true`,
   so the IN channel cost was not constant across runs; why is unexplained.
   The first `false` build failed: the 4 buttons took all 4 low-power
   (PORT) slots, so `dcf77_sync_open` returned `NO_MEM` ("dcf: open failed",
   window never opened; run `ppk-20261028T223009` showed 291 µA only for the
   10 s `epd.c` BUSY timeout, then 1.9 µA, and no sync). Fix:
   `NRFX_GPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS` 4 to 6.
   `epd.c:61` still uses `SENSE_HITOLO(true)`; with no panel, BUSY never
   falls and costs 291 µA for 10 s per refresh. With a panel the wait is
   about 0.75 s. Not changed.
3. The A and B wiring photos are missing.
4. The buzzer current is about 14 mA in its on phase, not the about 50 mA
   assumed in the design.
5. **DK back-feed with PPK2 logic attached (configuration A).** With several
   PPK2 digital inputs on DK pins, the nRF could stay biased when the PPK2
   source was off; only **`D0`** remained for A10. Which solder bridges or
   header paths cause this is **out of scope for this note** (separate
   autoresearch on SB cuts).

## Daily energy estimate

### As measured on C8 firmware `53ccb2c` (refresh spin still present)

Model: C8 idle, 1440 minute refreshes, one DCF window, one 60 s alarm
without motion (timeout), buzzer from D. Refresh charge: mean of two C8
minute refreshes (53.6 mC, 8.2 s). Alarm on the shared rail: C7 minus 1.21
mA UART share.

| State | Time per day | Charge per day (3.0 V rail) | Share |
| --- | --- | --- | --- |
| Idle, 74.6 µA | 74 382 s | 5.55 C (1.54 mAh) | 6.7 % |
| Refresh, 53.6 mC each | 1440 × 8.2 s | 77.15 C (21.43 mAh) | 93.1 % |
| DCF window, 425.8 µA | 150.5 s | 0.064 C (0.018 mAh) | 0.1 % |
| Alarm, nRF and peripherals | 59.0 s | 0.097 C (0.027 mAh) | 0.1 % |
| **3.0 V rail total** | | **82.86 C = 23.0 mAh = 249 J** | |
| Buzzer on `5V`, 7.08 mA | 59.0 s | 0.418 C = 2.1 J | |

That total is **obsolete** after `d0685ee`; most of the refresh charge was
the spinning nRF.

### Revised model after `d0685ee` (panel charge from B1, nRF from A10; C not re-run)

Assume each minute refresh adds **B1** panel charge (~6.8 mC mean) plus a
short nRF active phase like A10 boot (**1.1 mC**), and the nRF sleeps at
**2 µA** for the rest of an ~8 s refresh window. Idle, DCF and alarm rows
stay on the C8/B1/C7 numbers until C is re-measured.

| State | Charge per day (3.0 V rail) | Share |
| --- | --- | --- |
| Idle, 74.6 µA | 1.54 mAh | 32 % |
| Refresh, ~7.9 mC each × 1440 | **3.16 mAh** | 66 % |
| DCF + alarm | ~0.05 mAh | 1 % |
| **3.0 V rail (estimate)** | **~4.8 mAh ≈ 52 J** | |
| Buzzer on `5V` | 2.1 J | |

Minute refreshes drop from **93 %** to about **66 %** of the 3.0 V budget;
the dominant term is now the panel and regulator idle, not the nRF spin.
Re-measured as C9 (below): the refresh is 16.75 mC, so this model is superseded.

### C9 measured, minute refreshes (`c6faefd`)

Supersedes the revised model above (its 7.9 mC refresh was a B1 + A10
composite, not a C measurement). Idle 74.7 µA, refresh 16.75 mC.

| State | Charge per day (3.0 V rail) | Share |
| --- | --- | --- |
| Idle, 74.7 µA | 1.79 mAh | 21 % |
| Refresh, 16.75 mC × 1440 | **6.70 mAh** | 79 % |
| **3.0 V rail (estimate, DCF and alarm omitted)** | **~8.5 mAh ≈ 92 J** | |

Refresh on button press and alarm only (about 20 a day, `[INFERENCE]`):
0.093 mAh, so the budget is idle, 1.79 mAh. Idle is 37 times the 2 µA of the
nRF alone (A10, A11); the 72 µA gap is the peripherals on the rail.
