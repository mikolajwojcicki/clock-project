# 2026-10-24 — deep-sleep current measurement (PPK2)

- **Date:** 2026-10-24
- **Done:** task 4.0 leak check. Runs A, B and C follow (see
  `openspec/changes/add-deep-sleep-and-current-measurement/design.md`).
- **Failed:** —
- **Fix:** —
- **Why this part or library:** the PPK2 on P22 supplies only `VDD_nRF`. The
  display, LIS3DH and DCF module run from the USB-fed DK `VDD`, so a P22-only
  run neither isolates the MCU nor counts the clock.
- **Evidence:** this note; packed PPK2 captures (`.ppkz` + `.ppkz.json`) under `notes/measurements/`.

## 4.0 Leak check

Setup: DK on USB, PPK2 disconnected from P22 (SB9 cut), breadboard wired as in
`hardware/docs/breadboard-wiring-table.md`, multimeter to DK GND.

| Point | Voltage |
| --- | --- |
| nRF side of P22 (`VDD_nRF`, no supply connected) | ~1.5 V |
| DK P1 `VDD` | ~3.0 V |

Result: `VDD_nRF` sits at about half of `VDD` with nothing supplying it, so the
nRF is back-powered. The DK LEDs and the LIS3DH board LED light in this state.
The current comes from the USB `VDD` domain through I/O pins and their ESD
diodes. Which pins carry it (breadboard lines, DK LEDs or the interface UART)
is not yet separated; run A answers part of that.

Decision: the PPK2 Source Meter runs at **3.0 V** in A and C, equal to DK
`VDD`, so no line from the `VDD` domain sits above `VDD_nRF` in C.

Repeat with every breadboard wire unplugged from the DK headers: the nRF side
of P22 still reads ~1.5 V. The breadboard is not the leak path; the source is
on the DK. Candidates are the four LEDs (anode on DK `VDD`, cathode on an nRF
GPIO, so LED + pin ESD diode is a forward path into `VDD_nRF`) and the
interface UART. The LEDs light in this state, so the LED path is confirmed
active; whether the UART adds to it is not separated. Roughly 3.0 V minus an
LED drop minus an ESD diode drop gives about 1.5 V, which fits the LED path.

Consequence for run A: the leak needs `VDD_nRF` below DK `VDD`. With the
PPK2 sourcing 3.0 V on `VDD_nRF` and DK `VDD` at ~3.0 V, the LED path has no
forward voltage and should stop. Check during run A: the DK LEDs go dark once
the PPK2 is on, unless firmware drives them.

Run A, PPK2 at 3.0 V on P22: LED1, LED3 and LED4 go dark; LED2 stays dimly
lit. LED2 has been dim since the start of the project, under every firmware.
No project firmware touches P0.18 (LED2); `board_safe_state` leaves it as a
disconnected input. P0.18 is also the SWO trace pin, which the PCA10040 routes
to the interface MCU. Working explanation `[INFERENCE, not checked on the DK
schematic]`: the interface MCU's SWO input sinks a small current from DK `VDD`
through LED2. That path is DK `VDD` to the interface MCU and does not pass
through `VDD_nRF`, so it is outside the run A reading.

## Run A, nRF alone — first capture

Setup: PPK2 Source Meter 3.0 V on the nRF side of P22, every breadboard wire
unplugged from the DK, DK on USB for the interface MCU. Firmware: clock app
`make SLEEP_UART=0 flash`, last firmware change `70b067b` (repo HEAD
`296210a`). Sampling 100 kS/s, 300 s, recording started shortly after the
flash. Raw capture: [`notes/measurements/ppk-20261024T114909.ppkz`](../measurements/ppk-20261024T114909.ppkz)
(packed from `ppk-20261024T114909.csv`, 448 MB, kept local, not in git;
`ppk-archive.sh unpack` gives the CSV back). Plots with `firmware/tools/ppk-plot/ppk-plot.sh`:

- full run: `notes/media/2026-10-24_deep-sleep_ppk2-A_full.png`
- one second inside the first phase: `notes/media/2026-10-24_deep-sleep_ppk2-A_dcf-window-1s.png`
- the step at 283.4 s: `notes/media/2026-10-24_deep-sleep_ppk2-A_window-close.png`

| Interval | Mean | What runs |
| --- | --- | --- |
| 0–283.3 s | 3.977 µA | DCF window open: GPIOTE IN channel on `DCF_OUT`, pin floating (no pull, wire unplugged) |
| 283.6–300 s | 0.584 µA | window closed: System ON, RTC1 on LFXO, no armed peripheral |
| whole file | 3.789 µA | mix of the two |

Only one CPU wake in 300 s: about 80 µs at 283.389 s, peak 2.77 mA (no other
sample above 100 µA). That is the DCF window deadline: the window opens at
boot and lasts `DCF77_WINDOW_MS` = 300 s, so it closed about 17 s after the
recording started counting from boot. The next retry is 1 h later.

What the capture is not: the clock idle. `poll_clock()` does nothing until the
time is set (`s_set`), and with `SLEEP_UART=0` and no DCF signal nothing sets
it. So there were no 1 s ticks and no minute redraw attempts.

Reading:

- 0.584 µA is the floor of this chip on this board with RTC1 running and
  nothing else armed. Check it against the nRF52832 product specification
  table for System ON, RTC running, before quoting it; it may sit below the
  datasheet figure. A possible reason `[INFERENCE]`: DK `VDD` slightly above
  3.0 V back-feeds `VDD_nRF` through LED2 or the interface pins, so the PPK2
  sources less than the chip uses. The PPK2 trace touches 0 µA between
  regulator pulses, which fits.
- About 3.4 µA extra while the DCF window is open. Part is the GPIOTE IN
  channel; part can be the floating `DCF_OUT` input buffer. On the clock the
  receiver drives the pin, so this number is an upper bound for the window
  cost on the MCU side, not the real one.

## Run A2, nRF alone with time set — anomaly

Setup as run A, plus the Nucleo generator on `P0.25` for the boot sync, then
unplugged after the first minute burst (about 207 s). PPK2 output switched on
at about 13 s into the recording. Whether the nRF actually reset there is not
known: see "Supply toggle does not reset the nRF" below.
100 kS/s, 400 s. Raw capture: [`notes/measurements/ppk-20261024T132153.ppkz`](../measurements/ppk-20261024T132153.ppkz)
(packed from `ppk-20261024T132153.csv`, kept local). Plots: `notes/media/2026-10-24_deep-sleep_ppk2-A2_full.png`,
`…_A2_boot.png`, `…_A2_3s.png`.

| Interval | Mean | What runs |
| --- | --- | --- |
| 13–196 s | 3.04 mA | DCF window open, generator driving `DCF_OUT` |
| 196–207 s | 6.24 mA | first minute redraw: 10 s `EPD_BUSY` timeout, no panel |
| whole file | 3.31 mA | |

The floor is a flat 2.9–3.15 mA band from power-on to the end. It does not
drop when the generator is unplugged. Run A had 0.58–4 µA with the same
firmware. 3 mA matches the CPU running from flash on DC/DC, so the core
never reaches a real sleep, or the debug interface is powered `[INFERENCE]`.
On top: 1 Hz spikes (generator edges, later clock ticks) and four 10 s
minute bursts at about 6.3 mA. The time was set, so the boot sync worked.

Suspects: the debug interface left on (nRF Connect for Desktop was running
and its `nrfutil device list --hotplug` polls J-Link probes), or a busy loop
on the time-set path, which run A never reached. Not yet separated.

## Supply toggle does not reset the nRF

Firmware `b31048a`: `CLOCK_SEC_MARK` (`P0.22`) stays low from reset until init
ends, about 10 s on this bench because the boot refresh waits for the
`EPD_BUSY` timeout. Watched on PPK2 logic `D0`:

- PPK2 output off, then on: `P0.22` never goes low. The nRF does not reset.
- DK RESET button: `P0.22` goes low, then high after init, as intended.

So switching the PPK2 output off is not a power-on reset here. Two reasons can
keep the chip alive `[INFERENCE]`: back-feed from DK `VDD` (task 4.0 measured
~1.5 V on `VDD_nRF` with no supply), and the rail decoupling capacitors, which
a chip sleeping at about 1 µA drains very slowly. Run A2's "power-on at 13 s"
was therefore probably no reset at all, and its 3 mA state may have started
earlier.

Consequence: captures start from the RESET button with the PPK2 output on, and
the `D0` rising edge marks the end of init. A pin reset restarts the firmware
the same way as a power-on reset; it may not clear a debug-interface state,
which only a real power-on clears `[INFERENCE]`.

## USB unplugged: nRF apparently held in reset

The nRF52 DK guide (v3.x, sections 4.5 and 5) says to unplug USB and power
cycle the DK before measuring current. Board here: PCA10040 v3.0.1. Same
firmware `b31048a` (`SLEEP_UART=0`), PPK2 3.0 V on `P22`, DK USB unplugged,
PPK2 output switched off and on:

- `P0.22` on PPK2 `D0` never goes high, so init never finishes.
- `TP11` (`P0.21`, nRF RESET) reads about 1.3 V to GND (multimeter).
- PPK2 current about 700 µA average.

1.3 V is below the nRF52832 input-high minimum (0.7 × 3.0 V = 2.1 V), so the
RESET pin is not a valid high. `P0.21` reaches the unpowered interface MCU
through SB26. The likely path `[INFERENCE]`: the nRF's RESET pull-up and other
shared pins feed the interface MCU's unpowered rail through its ESD diodes,
which holds RESET near 1.3 V. The firmware enables pin reset
(`CONFIG_GPIO_AS_PINRESET`), so the chip stays in reset or resets at random,
and the 700 µA is mostly back-feed into the interface MCU side.

Confirmed 2026-10-24: with `TP11` tied to 3.0 V (nRF side of `P22`), `P0.22`
went from low to high about 10 s later, the normal init time. The nRF was held
in pin reset by the unpowered interface MCU.

## Run A3, pin reset off, DK USB unplugged — 1.19 mA floor

Firmware: commit `575150c` (no `CONFIG_GPIO_AS_PINRESET`), `make erase` first
(UICR `PSELRESET[0..1]` read back `0xffffffff`), then `make SLEEP_UART=0 flash`.
DK USB unplugged after flashing, PPK2 Source Meter 3.0 V on `P22`, logic `D0` on
`P0.22` with logic `VCC` on the nRF side of `P22`, breadboard unplugged.
100 kS/s, 210 s. Raw capture: [`notes/measurements/ppk-20261024T142752.ppkz`](../measurements/ppk-20261024T142752.ppkz)
(packed from `ppk-20261024T142752.csv`, kept local).

| Interval | Mean | What runs |
| --- | --- | --- |
| 0-0.70 s | 0.34 µA | before the PPK2 output came on (`D0` reads high, probably floating) |
| 0.70-11.14 s | ~1.2 mA | power-on and init, `D0` low; 28-52 mA inrush at 0.70 s, then a few ms at 3-8 mA |
| 12-210 s | 1189 µA | after init, `D0` high; min 1.04 mA, flat in every 10 s window |

Init now finishes after a real power-on with USB unplugged (`D0` rises 10.4 s
after power-on), so the reset hold is fixed. The floor is not: about 1.19 mA,
2000 times run A's 0.58 µA. On top of it, peaks to about 2.9 mA repeat every
7.33 ms (about 136 Hz), with no gap over the whole capture.

No firmware output pin goes to the interface MCU (`P0.05`-`P0.08`, `P0.18`,
`P0.21` are left disconnected). Run A, 0.58 µA, used the same firmware path with
USB plugged in and no PPK2 logic port. Suspects, not yet separated:
the PPK2 logic port (`VCC` and `D0` are new in this setup and draw from the
measured rail), or the nRF powering the unpowered interface MCU through shared
pins such as SWDIO, which has an internal pull-up.

## Run A4, separating the 1.19 mA — interface MCU is the load

Same firmware and wiring as A3. One capture, 96 s at 100 kS/s, steps done by
hand during the recording. Raw capture: [`notes/measurements/ppk-20261024T143919.ppkz`](../measurements/ppk-20261024T143919.ppkz)
(packed from `ppk-20261024T143919.csv`, kept local). Step times read from the current trace.

| Interval | Mean | State |
| --- | --- | --- |
| 1 s | inrush to 53 mA | PPK2 output on, USB unplugged (real power-on) |
| 2-11 s | 1187 µA | init, `D0` low |
| 11.54 s | | `D0` rises: init done |
| 12-26 s | 1254 → 1340 µA, rising | idle, `D0` high, logic port connected |
| 27-37 s | 1227 → 1198 µA | logic `VCC` and `D0` removed (`D0` then reads high, floating) |
| 38 s | | DK USB plugged in |
| 39-96 s | **5.85 µA** | idle, interface MCU powered; max 26 µA, min 0 |

Removing the logic port took off 100-150 µA, and that part only grew while
`D0` was high. Plugging USB in took off the remaining ~1.19 mA. So the
unpowered interface MCU, fed by the nRF through the shared pins, was the load.
Powered from USB, its pins sit at the same 3.0 V as `VDD_nRF` and draw nothing
from the measured rail `[INFERENCE]`.

The sawtooth pattern stays after USB is plugged in: peaks about 12 µA decaying
over ~0.5 ms, period 7.46 ms. That rules out the "half-powered part browning out"
idea for the A3 136 Hz peaks. The rhythm belongs to the nRF or its supply, for
example regulator refresh in System ON `[INFERENCE]`.

5.85 µA is idle with the DCF window still open (the capture ends at 96 s, the
window closes at 300 s). Run A had 3.98 µA in the same phase and 0.58 µA after.
Not in debug mode: that draws milliamps.

Procedure that follows: flash; unplug USB; PPK2 output off, then on (real
power-on, debug interface off); wait for init; plug USB back in; record. The
next capture must run past 300 s to get the window-closed floor.

## Run A5, nRF alone with the clock ticking — 0.81 µA idle

Pin reset is back on (`c7edb52`), so the A4 procedure would hang in reset
again. DK USB stays connected, as in the PPK2 user guide quick start. Power-on
sequence: PPK2 output off and DK USB unplugged for about 10 s, USB plugged back
in, then PPK2 output on at 3.0 V. Firmware `c7edb52`, `make SLEEP_UART=0`.
Breadboard unplugged; Nucleo generator into `P0.25` (1 kΩ) for the boot sync,
then unplugged. PPK2 logic `D0` on `P0.22`, `VCC` on the nRF side of `P22`,
removed at about 421 s. Only the Power Profiler app open. 100 kS/s, 500 s.
Raw capture: [`notes/measurements/ppk-20261024T151738.ppkz`](../measurements/ppk-20261024T151738.ppkz)
(packed by `ppk-archive.sh pack`, 1.25 GB CSV to 13.8 MB, CSV kept local).

| Interval | Mean | What runs |
| --- | --- | --- |
| 7.2-7.7 s | 2.3 mA, peak 30 mA | PPK2 output on: power-on, LFXO start |
| 17.69 s | | `D0` rises: init done (10.5 s after power-on) |
| 20-140 s | 4.24 µA | DCF window open, generator driving `DCF_OUT`, time not set |
| 147.8-157.0 s | 3.28 mA | time set by the sync, redraw: 9.2 s `EPD_BUSY` timeout (no panel) |
| 157.8 s on | | `D0` toggles every 1.000 s: clock ticking |
| 165-205, 225-265, 285-325, 345-385 s | 0.805-0.826 µA | idle between minutes, window closed, logic port on |
| 465-500 s | 0.820 µA | idle, logic port removed |
| every 60.0 s from 207.8 s | 3.28-3.29 mA for 9.1 s | minute redraw, `EPD_BUSY` timeout (no panel) |

- **Clock-running idle: 0.81 µA**, the mean of the idle windows. The 1 Hz tick
  costs 0.30 µC per tick (peak 5.5 mA, averaged over 20-35 ticks at 165-200 s),
  so 0.30 µA of the 0.81 µA. The rest, about 0.5 µA, matches run A's 0.58 µA
  floor without ticks.
- **Logic port share: none measurable** with USB in (0.805-0.826 µA before
  removal, 0.820 µA after). The 100-150 µA in A4 belonged to the unpowered
  interface MCU setup.
- **DCF window open: 4.24 µA**, close to run A's 3.98 µA. That is 3.4 µA for the
  GPIOTE IN channel on `DCF_OUT` while the receiver pin is driven.
- **Minute redraw without a panel: 3.28 mA for 9.1 s**, about 500 µA averaged
  over a minute; it dominates the whole-capture mean (440 µA for 160-500 s).
  With a panel, BUSY falls after about 3 s and the real refresh is measured in
  runs B and C. Open item: `wait_idle()` in `src/epd.c` calls `board_sleep()`
  until BUSY falls or the deadline passes, yet the chip draws a flat 3.28 mA,
  which looks like the CPU staying awake, not System ON sleep `[INFERENCE]`.
  To check before run B.

Conditions: PPK2 S/N `FACEF4EE8A0A`, Source Meter 3.0 V, room 24 °C.

**Against the datasheet** (`temp-resources/docs/nrf/nRF52832_PS_v1.9.pdf`,
section 17.1.1 "Current consumption: Ultra-low power", typical at VDD 3 V,
25 °C, DC/DC on, which matches `board.c` setting `DCDCEN`):

| Symbol | Condition | Typ. |
| --- | --- | --- |
| `ION_RAMOFF_EVENT` | System ON, no RAM retention, wake on any event | 1.2 µA |
| `ION_RAMON_EVENT` | System ON, full RAM retention, wake on any event | 1.5 µA |
| `ION_RAMOFF_RTC` | System ON, no RAM retention, wake on RTC | 1.9 µA |
| `IGPIOTE,IN` | GPIOTE input channel active (adds) | 0.1 µA (max 0.5) |

The firmware keeps full RAM and wakes on RTC1 with LFXO, so the closest
expectation is about 1.9-2.2 µA (the PS has no RAM-on + RTC row; `ION_RAMOFF_RTC`
plus the 0.3 µA RAM-retention difference between the two EVENT rows). Measured
0.81 µA with ticks and 0.58 µA without (run A) are **below** even the lowest
System ON figure. A chip drawing less than its typical sleep current is not
plausible, so part of the nRF supply is likely not passing through the PPK2
`[INFERENCE]`. Candidate: back-feed from the USB-powered interface MCU through
the SWD, RESET and UART lines into `VDD_nRF` via the pin protection diodes,
if the interface drives them at 3.3 V (OpenOCD reported `VTarget = 3.300 V`)
against the PPK2's 3.0 V. The DK guide's back-feed warning fits this. Check
before trusting A-numbers: repeat the idle at PPK2 3.3 V (no voltage
difference, no back-feed); if idle rises to about 2 µA, back-feed was it.

Still open: the bib entry `nRF52832PS` cites v1.4 (2017); the figures above
come from v1.9 (2023-12-06).

## Run A6, same as A5 at 3.3 V — 14.7 µA idle

Capture `notes/measurements/ppk-20261024T160837.ppkz` (500 s, 100 kS/s, packed
41.3 MB). Same sequence as A5; only change: PPK2 Source Meter 3.3 V. PPK2 S/N
`FACEF4EE8A0A`, room 24 °C, firmware `c7edb52` (no UART, pin reset on).

- `D0` rises at 11.39 s (init done). No ticks until 171 s, so the DCF sync took
  about 160 s; 20-160 s mean 23.2 µA with the DCF window open.
- After sync: idle median **14.7 µA**, 10 s means 16.2 µA including ticks.
  Tick charge 0.25 µC (A5: about 0.3 µC), so the firmware behaves the same.
- Minute redraw unchanged: 3.08 mA for 9.1 s, every 60 s.
- Logic port removed at about 421 s (last `D0` edge); idle 420-500 s stays at
  16.2 µA, so the logic port is not the extra load.

Reading: 0.3 V more supply turned 0.58-0.81 µA into 14.7 µA. The datasheet
expects about 2 µA for either voltage, and with DC/DC on the chip's own draw
barely depends on VDD. So both A5 and A6 are dominated by current between
`VDD_nRF` and the USB-powered interface MCU, in opposite directions
`[INFERENCE]`: at 3.0 V it feeds the nRF (reading too low), at 3.3 V the nRF
side leaks into it (reading too high). Neither run is a valid nRF figure.

The DK guide v3.x.x solder-bridge table documents the isolation: SB20/SB21
(SWDIO/SWDCLK), SB22-SB25 (`P0.05`-`P0.08`, UART), SB26 (`P0.21`, RESET) and
SB27 (`P0.18`, SWO) are "Cut to isolate ... from nRF52832 to the interface
MCU". Check before cutting: sweep the Source Meter 3.0 → 3.1 → 3.2 → 3.3 V
during one idle capture; a strong voltage dependence confirms the coupling.

## Run A7, live voltage sweep — coupling confirmed, no valid plateau

Date 2026-10-25. Same setup and sequence as A5/A6 (DK USB connected, SB9 cut,
Source Meter on `P22`, firmware `c7edb52`, no UART, pin reset on). PPK2 S/N
`FACEF4EE8A0A`. Recording started with the output at 3.3 V, DCF generator into
`P0.25` until the sync, then unplugged. Voltage stepped by hand during the
recording; step times from the PPK2 app log (UTC, sampling started 16:48:49.3):
3.2 V at 243.8 s, 3.1 V at 305.0 s, 3.0 V at 364.4 s, 2.9 V at 424.0 s.
100 kS/s, 484 s. Raw capture: [`notes/measurements/ppk-20261025T165720.ppkz`](../measurements/ppk-20261025T165720.ppkz)
(packed from the 1.23 GB CSV, CSV kept local). `D0` rises at 11.99 s, ticks
start at 183.4 s (sync done about 173 s).

Idle between minute redraws, windows chosen after each step settled and clear
of the 9 s redraw bursts (35 s each, 1 Hz ticks included in the mean):

| Source Meter | Mean | Median | Samples at 0 µA |
| --- | --- | --- | --- |
| 3.3 V, DCF window open (20-165 s) | 22.8 µA | 20.0 µA | 0 % |
| 3.3 V | 16.3 µA | 14.9 µA | 0 % |
| 3.2 V | 8.5 µA | 7.0 µA | 0 % |
| 3.1 V | 2.5 µA | 1.1 µA | 0.6 % |
| 3.0 V | 0.66 µA | 0.00 µA | 93 % |
| 2.9 V | 0.30 µA | 0.00 µA | 97 % |

Reading:

- The idle current falls by about 8 µA per 0.1 V near 3.3 V and reaches zero by
  3.0 V. The datasheet figure for the chip itself is about 2 µA and does not
  depend on VDD like this, so the curve is a leakage path between `VDD_nRF` and
  the USB-fed DK side, as proposed after A6. Coupling confirmed.
- From about 3.0 V down the PPK2 reads 0 µA most of the time. The net current
  into `VDD_nRF` is then negative or zero: the DK side supplies the chip
  `[INFERENCE]: the PPK2 reads no negative current, so a zero is a clamp`.
- No voltage gives a trustworthy nRF figure. Above about 3.1 V the reading is
  the chip plus leakage out of the nRF rail, below it the reading is the chip
  minus current fed in. The crossover is near 3.1 V, where 2.5 µA mean is close
  to the expected 2 µA by coincidence, not by isolation. A5 (0.81 µA) and A6
  (14.7 µA) are the two sides of this curve.
- DCF window open costs about 6.5 µA over idle at 3.3 V (22.8 vs 16.3 µA, means),
  more than the 3.4 µA seen at 3.0 V (A5); the extra is part of the same leak
  with a driven `DCF_OUT` `[INFERENCE]`. Not usable as a window cost either.

Decision: the DK cannot give the idle current while the interface MCU is wired
to the nRF. Next step is cutting SB20-SB27 (SWDIO/SWDCLK, UART `P0.05`-`P0.08`,
RESET `P0.21`, SWO `P0.18`) and repeating the idle capture at 3.0 V. After the
cut, flash and reset need a different path (external SWD on the DK debug-out
header, or SB re-bridged); to be decided before cutting.

## 2026-10-25 — run A8: idle after cutting SB20, SB21, SB26, SB27

Setup: SB20, SB21, SB26 and SB27 cut on the DK (PCA10040 v3.0.1), following
`scratch/sb-isolation-research/PLAN.md` (research branch, not tracked here).
After the cut the on-board J-Link reports `cannot read IDR` (expected, SWD
path open); the nRF kept its clock firmware, so no flashing was needed. DCF
generator disconnected, `D0` on `P0.22` connected. Raw capture:
[`notes/measurements/ppk-20261025T184005.ppkz`](../measurements/ppk-20261025T184005.ppkz)
(CSV kept local), 100 kS/s, 430 s, PPK2 log next to it.

Supply steps from the PPK2 log (capture starts 18:32:49): 3.0 V until
18:35:42 (about 173 s), 3.3 V until 18:38:59 (about 370 s), 3.0 V to the end.
The DCF sync was still running at the start, so 3.0 V covers the first window
as well, but sync ended at about 310 s, so the 3.3 V segment has a sync part
and an idle part.

| Window | Supply | Median | Mean | Samples at 0 µA |
| --- | --- | --- | --- | --- |
| 0-170 s, DCF sync running | 3.0 V | 7.2 µA | 9.5 µA | 0 % |
| 175-305 s, DCF sync running | 3.3 V | 8.2 µA | 10.8 µA | 0 % |
| 315-365 s, idle | 3.3 V | 2.2 µA | 3.5 µA | 0 % |
| 375-430 s, idle | 3.0 V | 0.68 µA | 2.0 µA | 4 % |

Reading:

- The cut removed most of the leak: idle at 3.3 V was 16.3 µA in A7 and is
  3.5 µA mean here.
- Some voltage dependence remains (3.5 µA at 3.3 V, 2.0 µA at 3.0 V, means).
  SB22-SB25 (UART) are still bridged and are the next suspect `[INFERENCE]`.
- Quote means, not medians: the 0.68 µA median is below the datasheet sleep
  figure and PPK2 resolution is poor at this level.
- `D0` has one edge only (rise at 11.3 s), so redraw bursts cannot be
  separated from idle; the mean-median gap of about 1.3 µA is probably the
  redraws. Idle windows are only 50-55 s long, so few redraws are included.
- Accepted as is for now; no longer idle run planned.

## 2026-10-25 — runs C2-C4: whole board on one measured rail

Setup: breadboard `3V3` rail fed from PPK2 `VOUT` (source meter, 3.0 V), so
nRF, display, LIS3DH and DCF receiver sit on one measured rail. DK USB
plugged in, DK `P1 VDD` not connected to the breadboard. SB20, SB21, SB26,
SB27 cut (A8). `D0` on `P0.22` (low in init, high after init, 1 Hz toggle
after DCF sync). DCF generator: Nucleo, antenna next to the receiver.
LIS3DH powered through breakout `3Vo` (the wiring docs say `VIN`; the board
has been wired this way since the start). Bench photo:
[`notes/media/2026-10-25_deep-sleep_ppk2-C.jpg`](../media/2026-10-25_deep-sleep_ppk2-C.jpg).
All captures 100 kS/s, CSVs kept local.

### C2: baseline

[`ppk-20261025T195017.ppkz`](../measurements/ppk-20261025T195017.ppkz), 300 s.

| Phase | Window | Current |
| --- | --- | --- |
| Boot init | 0-12 s | about 7.2 mA |
| DCF receiver on | 12-135 s | 4.21 mA |
| Idle after sync | 150-190 s | 3.87 mA (p1-p99 3.78-3.97) |
| Minute refresh | about 19 s | mean 6.0 mA, peak 20-21 mA |

Receiver cost: 4.21 - 3.87 = 0.34 mA. Idle is about 2000 times the nRF-only
figure from A8, so the peripherals hold almost all of it.

### C3: unplugging peripheral power

[`ppk-20261025T201314.ppkz`](../measurements/ppk-20261025T201314.ppkz), 436 s.
Unplug times from recording start: LIS3DH power 3:10, DCF `VDD` 5:10, display
`VCC` 6:05.

- LIS3DH power off: 3.87 → 2.08 mA (-1.8 mA).
- DCF `VDD` off: no change. Display `VCC` off: -0.015 mA. 2.06 mA left.
- Glitch: 230-288 s at about 11.8 mA (one refresh lasting about 55 s), 35 s
  after the LIS3DH unplug. Cause not found; refreshes in this run took about
  10 s against 19 s in C2, also not explained.

The breakout power LED was checked against the Adafruit schematic (product
2809 rev B): green LED + 10 kΩ from 3.3 V, about 0.1 mA at 3.0 V. It cannot
explain 1.8 mA. Rev B also has a BSS138 level shifter with 10 kΩ pull-ups on
both sides of `SCL` and `SDA`.

### C4: unplugging LIS3DH signal wires, power kept

[`ppk-20261025T204608.ppkz`](../measurements/ppk-20261025T204608.ppkz), 440 s.
Wires pulled at the breakout end, one at a time in idle windows. The user
swapped the planned order of `SDA` and `SDO` by accident, so the second pull
was `SDO` and the third `SDA`. The recording ended before `CS`, `INT1` and
power were pulled. Step times read from the 1 s means.

| Window | State | Idle current | Step |
| --- | --- | --- | --- |
| 20-185 s | DCF receiver on, all wired | 4.23 mA | |
| 200-285 s | Idle after sync (`D0` toggling from about 193 s) | 3.87 mA | baseline, same as C2 |
| 288-305 s | `SCL` (SPI clock) pulled at 286 s | 3.30 mA | -0.57 mA |
| 331-365 s | `SDO` (MISO) pulled at 329 s | 2.73 mA | -0.57 mA |
| 397-425 s | `SDA` (MOSI) pulled at 395 s | 2.07 mA | -0.66 mA |

Reading:

- Three signal wires account for the whole 1.8 mA that C3 put on the LIS3DH;
  with them off, idle equals the C3 end value with LIS3DH power removed. The
  breakout itself (sensor, regulator, LED) draws little at idle.
- About 0.6 mA per line at 3.0 V is about 5 kΩ to the rail, which matches two
  10 kΩ level-shifter pull-ups in parallel on a line the nRF holds low
  (`SCK` and `MOSI` are driven low between transfers in `src/spi_bus.c`)
  `[INFERENCE]`. The `SDO` step is not explained by this: `MISO` is an nRF
  input with no pull, so that line is driven from the breakout side, or the
  `SDA`/`SDO` wires were also swapped on the board `[INFERENCE]`.
- 2.07 mA is still unexplained and is not on the LIS3DH. Next: run C5
  (display and DCF signal wires).

### C5: unplugging display and DCF, then their signal wires

[`ppk-20261025T210658.ppkz`](../measurements/ppk-20261025T210658.ppkz), 543 s
recorded; packed window 180-543 s (full pack was 55 MB, over the 50 MB
limit), so pack-relative time = table time - 180 s.
LIS3DH fully removed (power and signal wires). Display and DCF powered and
wired at the start, generator on. Steps follow the planned order (user
confirmed); times read from the 1 s means, one change per idle window.

| Window | State | Idle current | Step |
| --- | --- | --- | --- |
| 15-180 s | DCF receiver on | 2.42 mA | |
| 195-235 s | Idle after sync (`D0` toggling from about 190 s) | 2.06 mA | baseline = C4 end |
| 369-433 s | Display `VCC` off at 368 s | 2.25 mA | +0.18 mA |
| 435-446 s | DCF `VDD` off at 434 s | 2.245 mA | -0.008 mA |
| 448-479 s | Display wires `P0.03`, `P0.04`, `P0.11`, `P0.12` off by 447 s | 2.23 mA | -0.02 mA |
| 490-539 s | `P0.28` (`EPD_RST`) off at 489 s | 2.0 µA | -2.22 mA |

Reading:

- `EPD_RST` carries the whole 2.07 mA. With it off, the board idles at about
  2 µA, the nRF-only figure from A8. `P0.29`, `P0.24` and `P0.25` were not
  needed.
- The firmware drove `EPD_RST` high from the first refresh onward and never
  released it (`src/epd.c`), so 3.0 V sat on the HAT `RST` input through
  every idle period.
- The current rose when display `VCC` went off, so the path does not need
  the display rail: `RST` feeds the unpowered HAT and pulls more `[INFERENCE]`.
  The user measured 19.0 kΩ on the HAT `RST` pin unpowered, which accounts for
  only about 0.16 mA at 3.0 V; the rest of the path was not found.
- Refreshes in this run took about 8 s at 6.8 mA (C2: about 19 s at 6.0 mA).
- Fix: firmware `07dd2de` returns `EPD_RST` to the reset default (input,
  buffer off, no pull) after every refresh. Not flashed yet; SWD cable
  pending. Expected idle with display wired: about 2 µA plus the LIS3DH
  share.
- Open: the LIS3DH 1.8 mA from C4 (level-shifter pull-ups against `SCK` and
  `MOSI` held low) still needs a fix.

## 2026-10-27 — run C6: fixed firmware, whole board

### Before the run: flash and function check

- External SWD: 10-pin IDC ribbon from DK `P19` (Debug out) to `P18`
  (Debug in), PPK2 source meter on `VDD_nRF`. The first try read
  `SWD DPIDR 0x00000000` with the PPK2 output off (target unpowered);
  with it on, `0x2ba01477` and flashing worked. The ribbon pin 1 joins the
  interface-MCU domain to `VDD_nRF`, so it comes off before any current run.
- Firmware `a762be5` (adds the `SCK`/`MOSI` idle-high fix to `07dd2de`).
  `SLEEP_UART=0` build: with the DCF generator next to the antenna, the
  clock synced and the panel redrew every minute (task 2.1). Without a sync
  the panel never changes, because the clock does not run until the time
  is set.
- UART build, LIS3DH rewired: the clock app does not read `WHO_AM_I`, so the
  SPI path is checked through the motion dismiss. Alarm set over UART for
  18:03, board shaken, no button pressed:

```text
[18:03:00] ALARM 18:03 wtorek
[18:03:02] dcf: rejected pulse (time unchanged 2026-10-27 18:03:02)
[18:03:04] MOTION event 1
  ... (INT1 edges; the counter rises once per debounced event)
[18:03:10] MOTION event 20 (stop)
[18:03:10] ALARM stop (motion)
```

- 20 motion events in about 6 s silenced the alarm, so the LIS3DH answers
  over SPI with the bus idling high, and INT1 wakes the CPU.
