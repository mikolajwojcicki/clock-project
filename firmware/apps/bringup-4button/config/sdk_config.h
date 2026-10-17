#ifndef SDK_CONFIG_H
#define SDK_CONFIG_H

/* nrfx drivers used by product firmware. Enable only what is linked. */

#define NRFX_SPIM_ENABLED 1
#define NRFX_SPIM0_ENABLED 1
#define NRF_LOG_ENABLED 0
#define NRFX_SPIM_DEFAULT_CONFIG_IRQ_PRIORITY 6

/* nrfx_pwm: 2.7 kHz buzzer tone. */
#define NRFX_PWM_ENABLED 1
#define NRFX_PWM0_ENABLED 1
#define NRFX_PWM_DEFAULT_CONFIG_IRQ_PRIORITY 6

/* nrfx_clock + nrfx_rtc: LFXO and RTC1 timebase. */
#define NRFX_CLOCK_ENABLED 1
#define NRFX_CLOCK_CONFIG_LF_SRC 1
#define NRFX_CLOCK_CONFIG_IRQ_PRIORITY 6
#define NRFX_RTC_ENABLED 1
#define NRFX_RTC1_ENABLED 1
#define NRFX_RTC_DEFAULT_CONFIG_IRQ_PRIORITY 6
/* The old register code left MISO without a pull. */
#define NRFX_SPIM_MISO_PULL_CFG 0

#endif
