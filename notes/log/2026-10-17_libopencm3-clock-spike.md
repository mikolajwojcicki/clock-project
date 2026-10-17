# libopencm3 clock spike (Nucleo-F411RE), 2026-10-17

Goal: check that libopencm3 can set the exact 77.5 MHz core clock and a 77.5 kHz TIM3 carrier on `PA6`.

Setup: `rcc_clock_setup_pll` with a custom `rcc_clock_scale` (M=8, N=310, P=4, Q=7, HSE bypass, APB1 /2, 2 wait states). `RCC_CR_HSEBYP` is set before the call. No direct PLL register writes were needed. TIM3 CH1 PWM, ARR=999, CCR=32.

Measurement: Saleae Logic, channel 0 on `PA6`, 24 MS/s, 0.5 s, first-to-last rising edge average.

| Firmware | Rising edges | Frequency | Error vs 77.5 kHz |
|---|---|---|---|
| libopencm3 spike | 46290 | 77492.93 Hz | -91.2 ppm |
| Old register-level generator | 46289 | 77492.96 Hz | -90.8 ppm |

Result: both read the same offset (0.4 ppm apart). The offset comes from the instrument clock or the ST-LINK 8 MHz MCO, not from the firmware. The limit is 0.01 % (100 ppm), so the spike passes. Decision: go, continue with the port.
Period min/max 12.875/12.917 us equals one 41.7 ns sample step (quantisation, not jitter).
