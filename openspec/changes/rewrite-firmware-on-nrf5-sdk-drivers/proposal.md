## Why

The firmware drives SPI, PWM and the RTC through direct register writes (`spi_bus.c`, `buzzer.c`, `board.c`), with spin loops around events. The project decision is: bare-metal on nRF5 SDK 17.1.0, preferring NRFx and SDK examples. If the audit in `fix-alarm-buzzer-pulse-blocking` finds that product code does not use the nrfx drivers, the thesis firmware chapter would describe hand-rolled peripheral code instead of the SDK it names. That is both a thesis risk and a maintenance risk.

**This change is conditional.** It runs only if task 4.2 of `fix-alarm-buzzer-pulse-blocking` records the verdict `rewrite`. If the verdict is `keep`, this change is dropped without implementation.

## What Changes

- Replace hand-written peripheral access in `firmware/src/` with nrfx drivers (`nrfx_spim`, `nrfx_pwm`, `nrfx_rtc` or `app_timer`, `nrfx_gpiote`, `nrfx_uarte` or the existing `app_uart`, `nrfx_clock`).
- Replace spin-wait loops with driver events or interrupts where the driver offers them.
- Keep the module boundaries (`alarm`, `clock`, `clock_face`, `epd`, `lis3dh`, `buzzer`, `board`) and every observable behavior.
- Re-run the bring-up and clock checks on the DK so nothing regresses.

## Capabilities

### New Capabilities
- `firmware-sdk-drivers`: product firmware reaches hardware only through nrfx drivers or nRF5 SDK libraries.

### Modified Capabilities
<!-- none -->

## Impact

- `firmware/src/*.c`, `firmware/apps/*/armgcc/Makefile` and `sdk_config.h`, `firmware/README.md`.
- Needs: the audit verdict from `fix-alarm-buzzer-pulse-blocking`. Runs after it, because the buzzer timing fix touches the same files.
- Locked architecture unchanged (bare-metal, nRF5 SDK 17.1.0, no RTOS, no Zephyr).
- Ordering against roadmap stage 5 (deep sleep) and stage 4 (motion dismiss) is a decision for the author; this change SHOULD land before them to avoid rewriting new code twice.
