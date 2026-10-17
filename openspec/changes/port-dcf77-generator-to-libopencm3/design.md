## Context

The generator is about 575 lines of C: `main.c` (446), `dcf77_encode.c` (88), plus `startup.c` (41), `stm32f411_regs.h` (91) and `stm32f411re.ld` (37). It sets the core to 77.5 MHz so TIM3 divides to an exact 77.5 kHz carrier (HSE bypass 8 MHz from ST-LINK MCO: M=8, N=310, P=4, APB1 /2, 2 flash wait states). SysTick runs at 1 ms and drives the pulses. USART2 on PA2/PA3 carries the console.

libopencm3 is not in nixpkgs. The `firmware/` tree already uses the installed Nordic SDK and system toolchain; this adds the first third-party source dependency.

## Goals / Non-Goals

**Goals:** remove hand-written boot and register code; keep behavior and the recorded measurements; make the experiment cheap to undo.

**Non-Goals:** any feature change to the generator; STM32Cube or any ST HAL; an RTOS; touching nRF52 product firmware; moving other tools to libopencm3.

## Decisions

1. **Submodule, pinned.** `firmware/third_party/libopencm3`, pinned to one commit chosen at apply time (record the hash in the README). Reason: reproducible, no Nix packaging work. Alternative `fetchFromGitHub` in a `shell.nix` was rejected by the author; the submodule is chosen. Cost: nested repo, one `git submodule update --init` step.
2. **Build.** Top-level `make` runs `make -C <lib> TARGETS=stm32/f4` once, then links with `-lopencm3_stm32f4` and `-T` libopencm3's `lib/stm32/f4/stm32f411?e.ld` or the project's own one-line include of `cortex-m-generic.ld`. Pick whichever builds with the existing flags; check that the library builds with the installed GCC 15.2 and that `-Werror` in the app does not fail on library headers.
3. **Clock is the one risk.** libopencm3's `rcc_clock_setup_pll()` takes a struct of fixed fields (`pllm`, `plln`, `pllp`, `pllq`, `flash_config`, `ahb_ppre`, `apb1_ppre`, `apb2_ppre`, `hpre`, `power_save`). A custom struct with M=8, N=310, P=4 should give 77.5 MHz, but the library's preset path assumes HSE crystal, not bypass. HSE bypass and the HSI fallback (M=16) may need direct `RCC_CR`/`RCC_PLLCFGR` writes via libopencm3 register macros. Do this first, measure `PA6` before porting anything else. If an exact 77.5 MHz cannot be set cleanly, the experiment stops there (no-go).
4. **Same structure, same logic.** Keep the SysTick ISR, `pin_set`, `carrier_apply`, level tables and encoder interface as they are. Only register pokes change: `TIM3_CCR1 = x` becomes `timer_set_oc_value(TIM3, TIM_OC1, x)`; `GPIOA_BSRR` becomes `gpio_set`/`gpio_clear` or direct `GPIO_BSRR(GPIOA)`; USART via `usart_*`. Prefer the lowest-latency call in the ISR: the dip must still switch within one carrier period (about 12.9 us).
5. **Experiment on a branch.** Work on `experiment/libopencm3-generator`. Merge only after the acceptance checks match the 2026-10-06 record. The register-level version remains in `main` and its history.
6. **Delete the old files only on merge,** in a separate commit after the check passes. No parallel copy kept in-tree.
7. **Thesis wording.** Say "open-source register-abstraction library", not "bare register level", for the generator. Lab tool only; the nRF52 product firmware stays bare-metal on nRF5 SDK.

## Risks / Trade-offs

- Exact 77.5 MHz with HSE bypass may not fit libopencm3's PLL helper. Mitigation: custom struct or direct register writes; no-go if neither is clean.
- Library call overhead in the SysTick ISR may lengthen dip lag beyond the recorded 12 us. Mitigation: measure; use `GPIO_BSRR` and `TIM_CCR1` macros directly if needed.
- GCC 15.2 may warn on library code. Mitigation: keep library sources out of `-Werror`; app files stay at `-Werror`.
- Adds a submodule to a repo that already has one. Accepted for an experiment; revert by dropping the branch.
- Net code saving is about 170 lines of boot and register code, in exchange for a dependency. If measured results differ, the saving is not worth it.
