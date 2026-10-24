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

Not recorded yet for task 4.2: room temperature and PPK2 serial. The datasheet
comparison waits for the nRF52832 Product Specification System ON + RTC figure,
which was not reachable on 2026-10-24 (403).
