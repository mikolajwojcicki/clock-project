/* Vector table and reset handler for STM32F411 (Cortex-M4). */
#include <stdint.h>

extern uint32_t _estack, _sidata, _sdata, _edata, _sbss, _ebss;
int main(void);

void Reset_Handler(void);
void Default_Handler(void) { for (;;) { } }
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));
void USART2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));

/* 16 core entries + 39 IRQs (up to USART2 = IRQ 38) */
__attribute__((section(".isr_vector"), used))
void (*const vectors[16 + 39])(void) = {
    (void (*)(void))&_estack,
    Reset_Handler,
    Default_Handler, /* NMI */
    Default_Handler, /* HardFault */
    Default_Handler, /* MemManage */
    Default_Handler, /* BusFault */
    Default_Handler, /* UsageFault */
    0, 0, 0, 0,
    Default_Handler, /* SVCall */
    Default_Handler, /* DebugMon */
    0,
    Default_Handler, /* PendSV */
    SysTick_Handler,
    [16 + 38] = USART2_IRQHandler,
};

void Reset_Handler(void)
{
    for (uint32_t *s = &_sidata, *d = &_sdata; d < &_edata;) {
        *d++ = *s++;
    }
    for (uint32_t *d = &_sbss; d < &_ebss;) {
        *d++ = 0;
    }
    main();
    for (;;) { }
}
