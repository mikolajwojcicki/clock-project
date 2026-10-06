# Tasks

## 1. Pure frame encoder and host check

- [x] 1.1 Derive by hand the 59-bit DCF-77 reference frame for 2026-10-06 20:15 CEST (Tuesday) from the published bit table and record it with a per-field breakdown in a comment of `firmware/tests/dcf77_encode_test.c`; verify each BCD field and the three even-parity bits by recounting
- [x] 1.2 Implement `firmware/tools/dcf77-generator/src/dcf77_encode.{c,h}`: time model (minute increment with hour, day, month, year rollover; leap years; weekday from date) and frame builder returning a `uint64_t`; verify the host check matches the reference frame bit for bit
- [x] 1.3 Extend the host check with rollovers (23:59 on 2026-10-31, on 2026-12-31, and on 2028-02-28 / 2028-02-29) and date validation (2026-02-30 rejected); verify `gcc ... && /tmp/dcf_encode_test` prints all checks passed
- [x] 1.4 Add the host-check command to `firmware/README.md` next to the classifier check; verify the command runs as written from the repo root
- [x] 1.5 Commit: `firmware: add DCF-77 frame encoder with host check`

## 2. Nucleo skeleton: build, flash, clock, LED

- [x] 2.1 Create `firmware/tools/dcf77-generator/` with a Makefile (`arm-none-eabi-gcc`, `-mcpu=cortex-m4 -mthumb`, `make`, `make flash` via `openocd -f board/st_nucleo_f4.cfg`, `make clean`), a linker script (512 KB flash, 128 KB RAM), a C startup with vector table and `.data`/`.bss` init, and a register header citing RM0383 sections; verify `make` builds without warnings
- [x] 2.2 Start HSE bypass with a bounded wait and HSI fallback, and run SysTick at 1 ms; blink `LD2` (`PA5`) at 1 Hz; verify after `make flash` that the LED period measures 1000 ms on the logic analyzer
- [x] 2.3 Add `USART2` (`PA2`/`PA3`, 115200 8N1) output with the banner (name, build time, clock source, default time, command list); verify in `picocom -b 115200` on the Nucleo's `/dev/ttyACM*` after a reset
- [x] 2.4 Add `firmware/tools/dcf77-generator/README.md` (build, flash, serial port, board revision note) and list `tools/` in `firmware/README.md`; verify the documented commands run as written
- [x] 2.5 Commit: `firmware: add bare-metal Nucleo-F411RE skeleton for DCF bench`

## 3. Signal output and UART control

- [x] 3.1 Drive `PA0` from the SysTick state machine with the encoder frame (pulse at seconds 0 to 58, 100/200 ms, none at 59), `LD2` mirroring `PA0`, and the per-frame log line; verify on the analyzer over two minutes: 1000 ms spacing, one 2000 ms gap per minute, widths within 2 ms, bits matching the log
- [x] 3.2 Add RX-interrupt line input and the `T YYYY-MM-DD HH:MM S|W` command applied at the next frame start; verify a valid command shows up in the next frame log and an invalid date prints an error with the sequence unchanged
- [x] 3.3 Add fault keys `p`, `d`, `m`, `g`, `s`, `n`, `?` per design; verify each on the analyzer and in the log (one-shot faults clear after one use; silence holds `PA0` low until pressed again)
- [x] 3.4 Document the command table and log format in the generator README; verify each documented command against the running board
- [x] 3.5 Commit: `firmware: generate DCF-77 frames with UART time set and faults`

## 4. Bench wiring and DK integration

- [x] 4.1 Add a section to `hardware/docs/breadboard-prototype-guide.md` (inside or after section 11) with the wiring table: Nucleo `PA0` → 1 kΩ → DK `P0.25`, Nucleo GND → DK GND, DCF module `OUT` disconnected; and add the Nucleo-F411RE to `hardware/bom/proto-bom.md` as owned bench equipment; verify the pins against the design and guide section 6
- [x] 4.2 Commit: `hardware: document Nucleo DCF generator bench wiring`
- [x] 4.3 Run the DK bring-up DCF-77 test (button 3) with the generator connected for at least two minutes; verify the DK logs about one valid pulse per second, minute markers 60 s apart, zero invalid pulses, and bit classifications matching the generator's bit string
- [x] 4.4 Repeat with the `g` and `m` faults; verify the DK logs an invalid pulse for the glitch and a missing second for the dropped pulse, then returns to valid pulses
- [x] 4.5 Record the results (board revision, clock source, pulse counts, analyzer capture name) in the generator README test record; commit: `firmware: verify DCF generator against DK bring-up test`

## Workflow follow-up

- Archive the change after all tasks are verified, and sync `dcf77-test-generator` into the main specs.
- The nRF52832 DCF-77 decoder change uses this generator for its parity and two-frame-agreement tests.
