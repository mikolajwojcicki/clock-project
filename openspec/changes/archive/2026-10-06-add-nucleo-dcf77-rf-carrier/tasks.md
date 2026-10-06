# Tasks

## 1. Clock tree at 77.5 MHz

- [x] 1.1 Add PLL, `FLASH_ACR`, APB1 prescaler, and TIM3 register definitions to `src/stm32f411_regs.h` with RM0383 section references; verify `make` still builds without warnings
- [x] 1.2 Switch `clock_init()` to the PLL (HSE: M=8, N=310, P=4; HSI fallback: M=16) with 2 flash wait states and PCLK1 = HCLK/2; retime SysTick (77 500 - 1) and USART2 (`BRR` from PCLK1); verify after `make flash` that the banner reads correctly at 115200 and `PA0` still measures 1000 ms spacing and 100/200 ms widths on the analyzer
- [x] 1.3 Update the generator README clock description; verify it matches the banner text
- [x] 1.4 Commit: `firmware: run DCF generator at 77.5 MHz PLL for exact carrier`

## 2. Modulated carrier and controls

- [x] 2.1 Start TIM3 CH1 PWM on `PA6` (AF2, high speed, `PSC=0`, `ARR=999`, `OC1PE`), with the level table from the design, and drive `TIM3_CCR1` from `pin_set()`; verify on the analyzer at 12 MS/s: carrier 77 500 Hz within 0.01%, and at 0 dB an idle duty of 50% and a pulse duty of 4.8% that follow `PA0` edges within one carrier period
- [x] 2.2 Add the `l` (level cycle, prints the level and `TIM3_CCR1` values) and `r` (carrier on/off) keys, the carrier state in the banner, and -20 dB at reset; verify the -6 dB duty values on the analyzer, `r` holding `PA6` low while `PA0` keeps pulsing, and the printed `CCR` values for -12 and -20 dB
- [x] 2.3 Verify the faults on the carrier: `m` (no dip for one second), `g` (10 ms dip at ms 500), `s` (unmodulated carrier until the frame start), on the analyzer with `PA0` and `PA6` captured together
- [x] 2.4 Add the `GEN_FORCE_HSI` build flag; verify `make clean && make CFLAGS_EXTRA=-DGEN_FORCE_HSI flash` shows the internal oscillator and a disabled carrier in the banner, `PA6` low, and `PA0` pulsing; then reflash the normal build
- [x] 2.5 Update the README pins table (`PA6` = `D12` carrier), key table, and level table; verify each documented key against the running board
- [x] 2.6 Commit: `firmware: modulate 77.5 kHz carrier on PA6 for receiver tests`

## 3. Receiver test over the air

- [x] 3.1 Add guide subsection 11.5 "Test the receiver with the Nucleo carrier": a wiring table (loop from `PA6` through 10 kΩ to Nucleo GND; module wired per 11.2 with `PON` to `P0.24` and `OUT` to `P0.25`; Nucleo `PA0` disconnected from the DK; common GND), loop placement (about 30 cm, axis in line with the ferrite rod), the analyzer channel map, and a warning to keep other DCF clocks away; verify the pins against guide section 6 and the generator README
- [x] 3.2 Commit: `hardware: document over-the-air DCF receiver bench test`
- [x] 3.3 Run the DK bring-up DCF test (button 3) at -20 dB with the loop at about 30 cm; if it does not decode, raise the level, then move the loop closer, then use 1 kΩ, recording each step; verify a run of at least two minutes with about one valid pulse per second, minute markers 60 s apart, and DK bit classifications matching the generator `bits=` strings
- [x] 3.4 From one analyzer capture of `PA0` versus the module `OUT`, measure the output delay and the pulse widths for `0` and `1` bits; verify the numbers against at least 50 pulses
- [x] 3.5 OBSOLETE (dropped by user 2026-10-06, sweep not needed): ~~Find the lowest level and largest distance that still decode (5 consecutive minutes with zero invalid pulses), and record the setup, results, and capture names in the generator README test record; commit: `firmware: record DCF-1060N receiver response to bench carrier`~~

## Workflow follow-up

- Archive the change after all tasks are verified, and sync the added `dcf77-test-generator` requirements into the main spec.
- The receiver delay and pulse-width numbers feed the DCF decoder's classifier windows and the thesis measurement chapter.
