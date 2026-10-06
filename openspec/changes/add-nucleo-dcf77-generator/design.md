# Design

## Context

See `proposal.md` (Why) for motivation and
`specs/dcf77-test-generator/spec.md` for the required behavior.

Current state that shapes the approach:

- The DK side already measures DCF pulses. `firmware/apps/bringup` treats
  `DCF_OUT` (`P0.25`) as push-pull, active high, with no internal pull, and
  classifies widths with `firmware/src/dcf77_classify.c`: a `0` bit is 40 to
  140 ms, a `1` bit is 150 to 260 ms, and a minute gap is 1700 to 2300 ms.
- Installed tools: `arm-none-eabi-gcc` 15.2 and OpenOCD 0.12.0 with
  `board/st_nucleo_f4.cfg`. `/etc/udev/rules.d/60-openocd.rules` already
  grants ST-LINK access. STM32Cube and the STM32F4 CMSIS device headers are not
  installed.
- Nucleo-F411RE (MB1136): STM32F411RE Cortex-M4F with 512 KB flash at
  `0x08000000` and 128 KB RAM at `0x20000000`. The ST-LINK/V2-1 provides SWD,
  a virtual COM port on `USART2` (`PA2` TX / `PA3` RX), and, on board revision
  C-02 or later, an 8 MHz `MCO` clock into `OSC_IN`. User LED `LD2` is on
  `PA5`.

## Goals / Non-Goals

**Goals:**

- A firmware small enough to read in one sitting: one register header, one
  startup file, one linker script, a pure encoder module, and a main loop.
- Pulse timing that is better than the DK's classifier tolerance by more than
  an order of magnitude, so a decoder failure points at the decoder.

**Non-Goals:**

- Automatic CET/CEST switching and the bit 16 changeover announcement. The
  operator picks the zone. A changeover test can be added later as another
  fault command.
- Leap-second insertion, civil-warning bits 1 to 14, and the call bit 15.
  These stay `0`.
- Low power. The generator runs from USB.

## Decisions

### Placement: `firmware/tools/dcf77-generator/`

`firmware/apps/` holds nRF52832 targets that share the nRF5 SDK Makefile. A
`tools/` directory keeps lab equipment visibly separate from product firmware,
so nobody mistakes the STM32 for a product MCU.

### Bare registers, no STM32Cube or libopencm3

The firmware needs about ten registers: `RCC`, `GPIOA`, `USART2`, `SysTick`,
`FLASH->ACR`, and the NVIC enable for `USART2`. Hand-written definitions from
RM0383 take less space than a vendored header pack and need no download, so
the build works offline with the installed toolchain only.

- Rejected: STM32CubeF4. It is a large download, it is not in nixpkgs, and it
  hides the register work that the bring-up firmware deliberately exposes.
- Rejected: libopencm3 through `nix-shell`. It adds a dependency for a few
  lines of setup.
- Rejected: CMSIS `core_cm4.h` from `NRF5_SDK_DIR`. It would tie the STM32
  build to the Nordic SDK path for one `SysTick` struct.

### Timebase: HSE bypass from ST-LINK MCO, HSI fallback

At reset, the firmware enables `HSEBYP` + `HSEON` and waits a bounded time
for `HSERDY`.

- If the external clock is ready, the system runs at 8 MHz from HSE, with
  crystal accuracy of tens of ppm.
- Otherwise it stays on the 16 MHz HSI, which is about ±1% at room
  temperature. That is still inside the classifier windows.

The banner names the source in use. No PLL is needed: 8 MHz is plenty for a
1 ms tick, and it keeps the flash at zero wait states.

### 1 ms SysTick state machine drives the pin

The `SysTick` interrupt runs every 1 ms with a millisecond-in-second counter.
It sets `PA0` high at ms 0 of a pulsed second, and low at ms 100 or 200 per
the current bit.

- Pin writes use `GPIOA->BSRR` inside the ISR, so jitter is a few CPU cycles.
- The glitch fault adds one 10 ms high pulse at ms 500.
- A missing-pulse fault skips the next pulse.
- `LD2` mirrors `PA0` so the operator can see the pulses.

The main loop only parses UART input and prepares the next frame.

- Rejected: timer PWM or output compare. That would be more registers for
  no measurable gain at 1 ms resolution.
- Rejected: busy-wait delays. The UART parser would add drift.

### Frame preparation and hand-off

At second 0 of each minute, the ISR switches to a pre-built 59-bit frame
(`uint64_t`) and sets a flag. The main loop then:

1. advances the time model by one minute;
2. applies any pending command (a new time or a one-shot fault);
3. builds the next frame;
4. prints the log line.

This takes far less than the 59 seconds available. A new time therefore takes
effect at the next frame start, as the spec requires.

### Pure encoder module with host check

`dcf77_encode.c` holds the time model (date and time increment, weekday by
Zeller or Sakamoto, days per month with leap years) and the frame builder.
It has no register access, so `firmware/tests/dcf77_encode_test.c` builds it
with host `gcc`, the same way as `dcf77_classify_test.c`.

The reference frame for 2026-10-06 20:15 CEST is derived by hand from the
DCF-77 bit table and written as a literal. The check does not regenerate it
with the encoder itself.

### UART protocol

`USART2` runs at 115200 8N1, RX-interrupt into a small line buffer.

| Input | Effect |
| --- | --- |
| `T YYYY-MM-DD HH:MM S\|W` + Enter | Set the time; `S` = CEST, `W` = CET |
| `p` | Parity fault in the next frame |
| `d` | Wrong-date fault: day +1, wrapped inside the month, parity correct |
| `m` | Missing pulse in the next second |
| `g` | Glitch in the next second |
| `s` | Toggle silence |
| `n` | Clear pending faults |
| `?` | Reprint the banner and command list |

Single letters act immediately and need no Enter. A line that starts with
`T` is parsed on Enter.

### Default time

The default start time is `__DATE__` / `__TIME__` from the build, with zone
CEST.

It is deterministic per build and close to real time after a fresh flash,
which is enough for a test signal.

### Wiring to the DK

`PA0` connects through a 1 kΩ series resistor to DK `P0.25`, with a GND jumper
between the boards. The DCF module's `OUT` must be disconnected from `P0.25`
first, because two push-pull outputs would fight. The resistor limits the
current to about 3.3 mA if that step is forgotten or if either board is
unpowered (it protects against back-powering through the ESD diodes).

## Risks / Trade-offs

- [Older Nucleo revision has no MCO into `OSC_IN`] → HSI fallback still meets
  the classifier windows. The banner shows which source runs, and the guide
  records the board revision.
- [Hand-written register addresses can be wrong] → Each block cites its RM0383
  section. Bring-up order is LED blink, then UART banner, then pulse timing
  measured on the analyzer.
- [Hand-built reference frame can share a mistake with the encoder] → The
  reference is checked bit by bit against the published DCF-77 table and
  cross-checked by decoding the analyzer capture by hand once.
- [Two USB-powered boards, ground loop or back-powering] → The common GND
  jumper and the 1 kΩ series resistor. The operator powers both boards from the
  same host.
- [DK test window is 10 minutes] → The generator runs continuously. The
  operator restarts the DK test as needed. No change to the bring-up firmware.

## Migration Plan

None. This adds a new tool and changes no existing firmware or hardware. To
roll back, delete `firmware/tools/dcf77-generator/` and its guide section.

## Open Questions

- The board revision (sticker on the Nucleo, for example `MB1136 C-02`). It
  only decides which clock source the banner will report.
