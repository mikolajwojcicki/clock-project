#ifndef SDK_CONFIG_H
#define SDK_CONFIG_H

/* Same UART stack as examples/peripheral/uart (PCA10040): UARTE + EasyDMA.
 * Flow control stays off. The DK virtual COM port echoed with that setup. */

/* nrfx_spim: blocking transfers for the e-paper and the LIS3DH. */
#define NRFX_SPIM_ENABLED 1
#define NRFX_SPIM0_ENABLED 1
#define NRFX_SPIM_MISO_PULL_CFG 0
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

/* nrfx_gpiote: DCF-77 receiver output edges (one IN channel while a sync window is open). */
#define NRFX_GPIOTE_ENABLED 1
#define NRFX_GPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS 4
#define NRFX_GPIOTE_CONFIG_IRQ_PRIORITY 6

#ifndef NRFX_PRS_ENABLED
#define NRFX_PRS_ENABLED 1
#endif
#ifndef NRFX_PRS_BOX_0_ENABLED
#define NRFX_PRS_BOX_0_ENABLED 0
#endif
#ifndef NRFX_PRS_BOX_1_ENABLED
#define NRFX_PRS_BOX_1_ENABLED 0
#endif
#ifndef NRFX_PRS_BOX_2_ENABLED
#define NRFX_PRS_BOX_2_ENABLED 0
#endif
#ifndef NRFX_PRS_BOX_3_ENABLED
#define NRFX_PRS_BOX_3_ENABLED 0
#endif
#ifndef NRFX_PRS_BOX_4_ENABLED
#define NRFX_PRS_BOX_4_ENABLED 1
#endif
#ifndef NRFX_PRS_CONFIG_LOG_ENABLED
#define NRFX_PRS_CONFIG_LOG_ENABLED 0
#endif

#ifndef UART_ENABLED
#define UART_ENABLED 1
#endif
#ifndef UART_DEFAULT_CONFIG_HWFC
#define UART_DEFAULT_CONFIG_HWFC 0
#endif
#ifndef UART_DEFAULT_CONFIG_PARITY
#define UART_DEFAULT_CONFIG_PARITY 0
#endif
#ifndef UART_DEFAULT_CONFIG_BAUDRATE
#define UART_DEFAULT_CONFIG_BAUDRATE 30801920
#endif
#ifndef UART_DEFAULT_CONFIG_IRQ_PRIORITY
#define UART_DEFAULT_CONFIG_IRQ_PRIORITY 6
#endif
#ifndef UART_EASY_DMA_SUPPORT
#define UART_EASY_DMA_SUPPORT 1
#endif
#ifndef UART_LEGACY_SUPPORT
#define UART_LEGACY_SUPPORT 1
#endif
#ifndef UART0_ENABLED
#define UART0_ENABLED 1
#endif
#ifndef UART0_CONFIG_USE_EASY_DMA
#define UART0_CONFIG_USE_EASY_DMA 1
#endif
#ifndef UART_CONFIG_LOG_ENABLED
#define UART_CONFIG_LOG_ENABLED 0
#endif
#ifndef NRF_LOG_ENABLED
#define NRF_LOG_ENABLED 0
#endif

#ifndef APP_FIFO_ENABLED
#define APP_FIFO_ENABLED 1
#endif
#ifndef APP_UART_ENABLED
#define APP_UART_ENABLED 1
#endif
#ifndef APP_UART_DRIVER_INSTANCE
#define APP_UART_DRIVER_INSTANCE 0
#endif

#endif
