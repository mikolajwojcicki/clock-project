# Design

## Context

See `proposal.md` for motivation and `specs/dcf77-test-generator/spec.md` for
the added requirements.

Current generator (`firmware/tools/dcf77-generator/`):

- It runs at 8 MHz straight from HSE bypass (the ST-LINK `MCO` on
  `MB1136 C-04`), or at 16 MHz from HSI as a fallback.
- The 1 ms SysTick ISR drives `PA0` and `LD2` through `pin_set()`. All faults
  and silence already flow through that one function.
- The UART baud divisor assumes PCLK1 = HCLK.

The receiver (DCF-1060N-800, SP6007) is an AM receiver behind a ferrite
antenna tuned to 77.5 kHz with a narrow crystal filter. The carrier therefore
has to be close to 77.5 kHz. Only the amplitude dip matters, not the phase.

## Goals / Non-Goals

**Goals:**

- Exact 77.5 kHz from an integer division of a crystal-derived clock.
- Modulation driven by the existing `pin_set()` path, so every fault behaves
  on the carrier the same way as on `PA0`, with no second timing source.
- A level range wide enough to find where reception stops, from the bench.

**Non-Goals:**

- Sine-wave output, filters, or an amplifier. The square wave's fundamental is
  what the tuned antenna picks up; harmonics at 232.5 kHz and above fall
  outside the receiver's passband.

## Decisions

### Clock: PLL to 77.5 MHz

`77 500 = 2² · 5⁴ · 31`, so the timer clock needs a factor of 31, which 8 MHz
and 16 MHz do not have. The PLL settings:

| Setting | Value | Result |
| --- | --- | --- |
| Source | HSE 8 MHz | |
| `PLLM` | 8 | VCO input 1 MHz (allowed: 1 to 2 MHz) |
| `PLLN` | 310 | VCO 310 MHz (allowed: 100 to 432 MHz) |
| `PLLP` | 4 | SYSCLK 77.5 MHz (scale-2 limit is 84 MHz, so no `PWR_CR` change) |

The bus and flash settings follow from that clock:

- APB1 prescaler 2 gives PCLK1 = 38.75 MHz (limit 50 MHz). The APB1 timer
  clock is twice PCLK1 = 77.5 MHz.
- Flash needs 2 wait states at 3.3 V up to 90 MHz. Set `FLASH_ACR` before
  switching, with prefetch and caches on.

The derived timings:

- TIM3 with `PSC = 0`, `ARR = 999` runs at exactly 77.5 kHz. The duty
  resolution is 0.1%.
- SysTick reload is 77 500 - 1 for the 1 ms tick.
- USART2 `BRR = 38.75 MHz / 115200 = 336` (0.1% error).

If HSE does not start, the PLL runs from HSI with `PLLM = 16`, giving the same
77.5 MHz. All code then sees one clock. The carrier stays off, because HSI is
about ±1% (±775 Hz) off, far outside the receiver's crystal filter.

- Rejected: keep 8 MHz and use `ARR = 102`. That gives 77.67 kHz, 170 Hz off,
  which is outside the crystal filter.
- Rejected: run TIM3 alone from a separate clock. No STM32F411 timer has an
  independent clock input of the right frequency without external parts.

### Modulation by duty cycle

For a rectangular wave with duty D, the fundamental amplitude is proportional
to `sin(π·D)`. Full level is `D = 50%`. A 15% dip needs `sin(π·D) = 0.15`,
which gives `D = 4.8%`.

| Level | Idle `CCR` | Pulse `CCR` | Dip (computed) |
| --- | --- | --- | --- |
| 0 dB | 500 | 48 | 15.0% |
| -6 dB | 167 | 24 | 15.0% |
| -12 dB | 80 | 12 | 15.2% |
| -20 dB | 32 | 5 | 15.7% |

`pin_set(on)` additionally writes `TIM3_CCR1 = on ? pulse : idle` for the
current level, or 0 when the carrier is off or disabled. With `OC1PE`
preload, the new duty starts at the next carrier period (≤12.9 µs), so no
runt cycles occur. PA6 runs at high output speed so that the 64 ns pulses at
-20 dB keep their shape.

- Rejected: switch between two GPIO drive resistors for the two levels. It
  needs more wiring and gives fixed levels only.
- Rejected: DAC sine output. The STM32F411 has no DAC.

### Default level -20 dB at reset

It starts weak so that the receiver's AGC is not driven into overload at the
first try. The operator raises the level with `l`.

### Coupling loop

The loop is 5 to 10 turns of hookup wire, about 3 cm across (for example,
wound on a marker pen), connected from `PA6` through a series resistor to
Nucleo GND.

- The loop's reactance at 77.5 kHz is a few ohms, so the resistor sets the
  current: 10 kΩ gives 0.33 mA peak, and 1 kΩ gives 3.3 mA peak. Both are well
  inside the GPIO limit.
- Start with 10 kΩ and the loop about 30 cm from the ferrite rod, with the
  loop axis in line with the rod. Move it closer, or switch to 1 kΩ, only if
  nothing decodes.
- The loop is not wired to the DK. Nucleo GND to DK GND stays as the common
  reference for the analyzer.
- `PA0` must be disconnected from DK `P0.25`, because the module's `OUT`
  drives that pin in this test.

### Verification resolution

Logic 8 is reliable at up to 12 MS/s here (83 ns per sample).

- The 0 dB and -6 dB duty values (≥24 counts, ≥310 ns) can be measured
  directly.
- At -12 dB and -20 dB, the pulse width is 155 ns and 64 ns, too close to the
  sample period. Those levels are verified through the DK reception result and
  a UART readback of `TIM3_CCR1`.

### HSI-fallback check

A build flag `GEN_FORCE_HSI` skips the HSE start, so the fallback path can be
tested once without changing solder bridges.

## Risks / Trade-offs

- [ST-LINK crystal tolerance (tens of ppm, ±2 to 4 Hz) versus a narrow
  receiver filter] → Measure the carrier frequency on the analyzer. If the
  module does not lock at any level or distance, record it as a finding
  before suspecting the modulation.
- [Strong near field overloads the AGC, and the dip no longer decodes] →
  Start at -20 dB, 10 kΩ, about 30 cm. The level sweep is part of the test.
- [Square-wave harmonics] → They sit at 232.5 kHz and above, outside the tuned
  antenna's passband, and are tiny at this current and range.
- [Nearby DCF clocks, such as the user's bedside clock, sync to the generated
  time] → Keep them out of the room during tests, or set the generator to the
  real time.
- [Radio rules] → This is an inductive near-field source with sub-milliampere
  to a few milliamperes of loop current and a range of under a meter. It is
  far below the field limits for inductive short-range devices in this band
  (ERC Recommendation 70-03, Annex 9). Run it only while testing.

## Migration Plan

This is a firmware update of the bench tool only.

- `PA0` behavior is unchanged, so the logic-level bench test in guide section
  11.4 still works.
- Rollback: revert the generator commits.
