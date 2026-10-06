/*
 * Minimal STM32F411 register definitions, hand-written from RM0383 (rev 3+).
 * Only what the generator uses. No vendor headers needed.
 */
#ifndef STM32F411_REGS_H
#define STM32F411_REGS_H

#include <stdint.h>

#define REG32(addr) (*(volatile uint32_t *)(addr))

/* RM0383 section 6: RCC, base 0x4002 3800 */
#define RCC_BASE    0x40023800u
#define RCC_CR      REG32(RCC_BASE + 0x00)
#define RCC_CFGR    REG32(RCC_BASE + 0x08)
#define RCC_AHB1ENR REG32(RCC_BASE + 0x30)
#define RCC_APB1ENR REG32(RCC_BASE + 0x40)
#define RCC_CR_HSEON   (1u << 16)
#define RCC_CR_HSERDY  (1u << 17)
#define RCC_CR_HSEBYP  (1u << 18)
#define RCC_CFGR_SW_HSE  1u
#define RCC_CFGR_SWS_MASK (3u << 2)
#define RCC_CFGR_SWS_HSE  (1u << 2)
#define RCC_AHB1ENR_GPIOAEN (1u << 0)
#define RCC_APB1ENR_USART2EN (1u << 17)

/* RM0383 section 8: GPIOA, base 0x4002 0000 */
#define GPIOA_BASE  0x40020000u
#define GPIOA_MODER REG32(GPIOA_BASE + 0x00)
#define GPIOA_OSPEEDR REG32(GPIOA_BASE + 0x08)
#define GPIOA_BSRR  REG32(GPIOA_BASE + 0x18)
#define GPIOA_AFRL  REG32(GPIOA_BASE + 0x20)
#define MODER_OUT 1u
#define MODER_AF  2u

/* RM0383 section 19: USART2, base 0x4000 4400 (APB1) */
#define USART2_BASE 0x40004400u
#define USART2_SR   REG32(USART2_BASE + 0x00)
#define USART2_DR   REG32(USART2_BASE + 0x04)
#define USART2_BRR  REG32(USART2_BASE + 0x08)
#define USART2_CR1  REG32(USART2_BASE + 0x0C)
#define USART_SR_RXNE (1u << 5)
#define USART_SR_TXE  (1u << 7)
#define USART_CR1_RE     (1u << 2)
#define USART_CR1_TE     (1u << 3)
#define USART_CR1_RXNEIE (1u << 5)
#define USART_CR1_UE     (1u << 13)
#define USART2_IRQn 38u

/* Cortex-M4 generic: NVIC ISER1 (IRQ 32..63) and SysTick */
#define NVIC_ISER1  REG32(0xE000E104u)
#define SYST_CSR    REG32(0xE000E010u)
#define SYST_RVR    REG32(0xE000E014u)
#define SYST_CVR    REG32(0xE000E018u)
#define SYST_CSR_ENABLE  (1u << 0)
#define SYST_CSR_TICKINT (1u << 1)
#define SYST_CSR_CLKSRC  (1u << 2)

/* Board: Nucleo-F411RE. LD2 = PA5; DCF output = PA0 (A0); VCP = PA2/PA3 (AF7) */
#define PIN_DCF 0u
#define PIN_LED 5u

#endif
