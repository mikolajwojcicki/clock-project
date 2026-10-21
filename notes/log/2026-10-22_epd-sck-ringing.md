# 2026-10-22 — e-paper freezes without probes: SCK ringing

- **Date:** 2026-10-21/22 (bench session over midnight)
- **Done:**
  - Symptom: the clock face refreshed while logic-analyser hooks were on every
    display header pin. It froze when they were removed and worked again when
    they were put back. Dangling hooks with the analyser unplugged also fixed it.
  - Found a separate firmware bug in `firmware/src/epd.c` `wait_idle()`:
    `nrfx_gpiote_in_uninit()` calls `nrf_gpio_cfg_default()`, which disconnects
    the input buffer. BUSY then read 0, so later waits returned at once and the
    firmware reported success for refreshes that never ran. Fixed in `b7e7cdc`
    (re-enable BUSY input with pull-up after uninit).
  - Added `display ok|FAIL <ms>` on UART for each draw (`70b067b`).
  - Bisect with one dangling probe at a time, firmware `70b067b`, DK on J-Link
    USB power, ~room temperature.
- **Failed:**
  - No probes: `display FAIL 10043 ms` at 22:10 and 22:11 (DK clock). BUSY
    stayed high for the full 10 s timeout, so the SSD1680 controller hung.
- **Fix:**
  - One dangling probe on display `CLK` only: `display ok 7752 ms` and
    `display ok 7750 ms` at 22:14 and 22:15, panel follows the minutes.
  - Cause: `SPI_SCK` (`P0.03`) runs over long breadboard wires. The fast edge
    rings and the panel counts extra clock pulses, so it receives corrupted
    commands and hangs. The probe wire adds capacitance and damps the ringing.
  - Planned hardware fix: about 100 Ω in series with SCK at the DK pin
    (47–220 Ω acceptable). Not fitted yet: only 470 Ω / 1 kΩ / 5 kΩ on hand.
    The probe stays on CLK until then. Rejected: 470 Ω+ (with wire and input
    capacitance it rounds the 250 kHz clock more than needed; not tried).
  - Firmware cannot fix it: the clock is already slow (250 kHz) and standard
    drive; ringing comes from edge speed and wiring, not the clock rate.
- **Why this part or library:** a series resistor at the source is the usual
  damping for a single-ended clock line. It costs no static current, unlike a
  capacitor to GND, which charges on every edge. Lesson for the perfboard and
  any PCB: keep SCK short and put a series resistor footprint on it.
- **Evidence:** UART logs in this session; commits `b7e7cdc`, `70b067b`.
  To check after fitting the resistor: remove all probes, expect
  `display ok ~7750 ms` for 3 minutes and after one power-cycle.
