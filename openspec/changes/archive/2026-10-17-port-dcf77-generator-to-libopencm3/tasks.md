## 1. Branch and library

- [x] 1.1 Create branch `experiment/libopencm3-generator` from the current head
- [x] 1.2 Add libopencm3 as submodule at `firmware/third_party/libopencm3`, pin one commit, note the hash in `firmware/tools/dcf77-generator/README.md`
- [x] 1.3 Build the library for `stm32/f4` with the installed `arm-none-eabi-gcc` and confirm it produces `libopencm3_stm32f4.a`
- [x] 1.4 Commit: `firmware: add libopencm3 submodule for generator experiment`

## 2. Clock spike (go / no-go)

- [x] 2.1 Write a minimal app that sets 77.5 MHz from HSE bypass (M=8, N=310, P=4, APB1 /2, 2 wait states) and starts TIM3 CH1 at 77.5 kHz on `PA6`
- [x] 2.2 Flash it and measure `PA6` with the Saleae; record frequency (limit 0.01%)
- [x] 2.3 If exact 77.5 MHz is not reachable cleanly, stop, write the no-go in `notes/log/`, and drop the branch

## 3. Port

- [x] 3.1 Port `main.c` (clock with HSI fallback, GPIO, TIM3 carrier, SysTick, USART2) to libopencm3 with unchanged behavior
- [x] 3.2 Update the `Makefile` to build the library first, link it, and keep `make flash` on OpenOCD
- [x] 3.3 Confirm the build is clean (`-Werror` on app sources, library sources excluded)
- [x] 3.4 Run `firmware/tests/dcf77_encode_test.c` on the host; it must still pass unchanged
- [x] 3.5 Commit: `firmware: port DCF-77 generator to libopencm3`

## 4. Re-measure against 2026-10-06

- [x] 4.1 Saleae on `PA0`: widths 100/200 ms, spacing 1000 ms, minute gap 2000 ms ±2 ms, 3 frames match the UART log
- [x] 4.2 Saleae on `PA0` and `PA6`: carrier 77.5 kHz within 0.01%, duty switches within one carrier period of each `PA0` edge, levels 0/-6/-12/-20 dB
- [x] 4.3 Fault keys `g m s p` and carrier keys `l r` behave as recorded
- [x] 4.4 DK bring-up DCF test (button 3): valid pulses, 3 minute markers, DK frames equal the generator `bits=` strings (overruns=8, not investigated)
- [x] 4.5 Write `notes/log/` entry with a before/after table (code size, boot code lines, dip lag, results) and the decision

## 5. Decide

- [x] 5.1 If every check matches: remove `stm32f411_regs.h`, `startup.c`, `stm32f411re.ld` in a separate commit, update `README.md` build section, merge the branch
- [x] 5.2 If any check fails and cannot be fixed cleanly: keep the register-level version on `main`, record the reason, delete the branch and the submodule (skipped: experiment merged via 5.1)
