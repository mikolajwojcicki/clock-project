# 2026-10-24 — deep-sleep current measurement (PPK2)

- **Date:** 2026-10-24
- **Done:** task 4.0 leak check. Runs A, B and C follow (see
  `openspec/changes/add-deep-sleep-and-current-measurement/design.md`).
- **Failed:** —
- **Fix:** —
- **Why this part or library:** the PPK2 on P22 supplies only `VDD_nRF`. The
  display, LIS3DH and DCF module run from the USB-fed DK `VDD`, so a P22-only
  run neither isolates the MCU nor counts the clock.
- **Evidence:** this note; CSVs under `notes/measurements/` as the runs land.

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
flash. Raw CSV: `notes/measurements/ppk-20261024T114909.csv` (448 MB, kept
local, not in git). Plots with `firmware/tools/ppk-plot/ppk-plot.sh`:

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
100 kS/s, 400 s. Raw CSV: `notes/measurements/ppk-20261024T132153.csv` (kept
local). Plots: `notes/media/2026-10-24_deep-sleep_ppk2-A2_full.png`,
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
