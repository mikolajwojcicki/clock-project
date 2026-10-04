# Design

## Context

- Only `firmware/apps/blinky/` exists. Its `armgcc/Makefile` already uses
  `NRF5_SDK_DIR`, `-DBOARD_PCA10040`, `CONFIG_GPIO_AS_PINRESET`, and OpenOCD
  `flash` / `erase` targets. `firmware/src/` and `firmware/boards/` are empty.
- Pin map, safe states, and reserved pins come from guide sections 6 to 6.2.
- DK UART to the interface MCU uses `P0.06` (TX) and `P0.08` (RX); it is a
  reserved pin pair that the firmware may use for its own console.
- Two hardware facts are unconfirmed: the AP-1205V-P1 drive type (self-drive
  DC or external tone) and the DCF-1060N-800 `OUT` stage (push-pull or open
  collector). The guide also keeps `DCF_OUT` disconnected until its level is
  measured.

## Goals / Non-Goals

**Goals:**

- One app that passes every guide checklist item that needs firmware.
- Drivers in `firmware/src/` small enough to reuse in the clock firmware.

**Non-Goals:**

- Full DCF-77 frame decoding (time and date bits) and parity checks.
- Fonts, partial refresh, or a clock UI on the display.
- Low-power sleep tuning or current measurements; the app idles with
  `__WFE()`, but its current is not a thesis number.
- BLE, SoftDevice, web app, or any host program.

## Decisions

### App layout: clone blinky, add `firmware/src/` to the build

`firmware/apps/bringup/` copies blinky's wrapper `Makefile`, `armgcc/Makefile`,
linker script, and `config/sdk_config.h`, then adds `../../../src/*.c` and the
needed SDK nrfx sources. Alternative: a shared top-level Makefile fragment. Not
worth it for two apps; revisit when the clock app arrives.

### Pin map in one header

`firmware/boards/dk_breadboard_pins.h` defines every signal from guide
section 6 with the guide's project names (`SPI_SCK`, `EPD_CS`, ...). Drivers
take pins only from this header, so a later PCB header can replace it.

### Safe states first, in `main()`

The first statements of `main()` set the four control pins (output, safe
level) with `nrf_gpio` before clocks, logging, or SPI. A shared
`board_safe_state()` function is also called at the end of every test and on
every error path. Before `main()` runs, pins are floating inputs for a few
microseconds; the guide already covers that by requiring the wires to stay
disconnected until this firmware is flashed. An erased chip leaves them
floating indefinitely; the README says so.

### UART console: `NRF_LOG` with the UART backend, in-place mode

Blinky already compiles the `nrf_log` frontend. Add
`nrf_log_backend_uart` + `nrfx_uarte` at 115200 baud on the DK default pins,
with `NRF_LOG_DEFERRED 0` so lines appear before a crash or reset.
Alternative: RTT through the J-Link. Rejected because OpenOCD RTT setup is
more fragile than `picocom` on `/dev/ttyACM0`. The banner prints
`NRF_POWER->RESETREAS` and then clears it.

### Buttons and events: `nrfx_gpiote` with a single event flag loop

Buttons 1 to 4 (`P0.13` to `P0.16`, active low, internal pull-up),
`SENSOR_INT1`, and `DCF_OUT` use GPIOTE input events. ISRs only record an
event and a timestamp; the main loop dispatches. A simple software debounce
(ignore presses within 50 ms) is enough for test selection.
`ponytail:` no event queue; one pending flag per source. Back-to-back DCF edges
faster than the loop are counted as invalid. Upgrade to a ring buffer if needed.

### Timebase: RTC1 on the 32.768 kHz crystal

Start LFCLK from the crystal (`NRF_CLOCK` LFCLKSRC = Xtal) and run RTC1 with
no prescaler as a free-running counter. Timestamps for DCF pulse widths,
timeouts, and debounce come from it. Alternative: `app_timer`. Rejected; a raw
counter is fewer SDK modules and is the same timebase the clock firmware will
use.

### Shared SPI: SPIM0 at 1 MHz, chip select in software

`nrfx_spim` on `SPI_SCK` `P0.03`, `SPI_MOSI` `P0.04`, `SPI_MISO` `P0.02`,
SPI mode 0 for the e-paper and mode 3 (clock idle high) for the LIS3DH, per
their datasheets. The bus driver switches mode per device before asserting
CS. Alternative: one mode for both. Rejected because it relies on behavior
outside the LIS3DH datasheet. The driver asserts exactly
one CS around each transfer and asserts both high between transfers.
1 MHz tolerates breadboard jumpers; speed tuning is not a goal.

### E-paper driver: Waveshare 2.13" V4 (SSD1680) command set

Port the init, full-refresh, and deep-sleep sequence from Waveshare's
`EPD_2in13_V4` reference: hardware reset on `EPD_RST`, wait for `EPD_BUSY`
low, write the 250 x 122 frame (4000 bytes, 1 bit per pixel) generated
on the fly for border + checkerboard, so no frame buffer is needed. Busy wait
has a 10 s timeout.

### LIS3DH: SPI, high-pass motion interrupt on INT1

Read `WHO_AM_I` (`0x0F`, expect `0x33`) with the SPI read bit. Configure
100 Hz all axes, high-pass filter on interrupt 1 so gravity does not trigger
it, OR of X/Y/Z high events, threshold about 250 mg, latched; read `INT1_SRC`
to clear and report the axes. Test timeout 60 s.

### DCF-77: edge timestamps and a pure classifier

On each `DCF_OUT` edge, store the RTC1 timestamp. Pulse width = active-level
duration. A pure function classifies a width (and the gap before it) as `0`
(about 100 ms), `1` (about 200 ms), invalid, or minute marker (gap about
1.8 s or longer). The pure function lives in `firmware/src/dcf77_classify.c`
and gets one host-side check: `firmware/tests/dcf77_classify_test.c`, an
`assert`-based program built with the host `gcc` (from `nix-shell -p gcc` if
not on `PATH`). Active level and input pull are constants in the pin header:
default active-high, no pull. Test window 10 minutes.

### Buzzer: steady step, then tone step, both bounded

Step 1: `BUZZER_EN` high for 500 ms. Step 2: 2.7 kHz square wave for 500 ms
using `nrfx_pwm` (below the 10 kHz limit for `P0.31`). Whichever step sounds
tells the operator the drive type, which the README asks them to record. The
pin returns low and the PWM is stopped in `board_safe_state()`.

### VS Code tasks

Add `firmware: bringup build`, `firmware: bringup flash`, and
`firmware: serial console` (`picocom -b 115200 /dev/ttyACM0`) to
`.vscode/tasks.json`, next to the blinky tasks.

## Risks / Trade-offs

- Panel revision differs from V4 → The display test fails with busy timeout or
  garbage. Mitigation: README tells the operator to read the FPC label; init
  sequence is isolated in one function.
- LIS3DH SPI mode mismatch on a shared bus → Reconfigure SPIM mode per device
  in the bus driver; both devices are never selected together.
- `DCF_OUT` floating when disconnected → noise edges logged as invalid pulses.
  Mitigation: the DCF test prints a reminder that unmeasured `OUT` must stay
  disconnected and that invalid counts with no wire are expected.
- DCF output may be open collector → no pulses with "no pull". Mitigation: one
  constant switches to internal pull-up; README documents it.
- Buzzer draws enough current to reset the DK → Steps are 500 ms; the next
  banner prints the reset reason.

## Migration Plan

Additive. Blinky is unchanged and stays the controller-only test in guide
section 7. Rollback: delete `firmware/apps/bringup/` and the new files.

## Open Questions

- Exact e-paper panel revision label (V3 or V4); resolved by reading the FPC
  during the first display test.
- AP-1205V-P1 drive type; resolved by which buzzer step sounds.
