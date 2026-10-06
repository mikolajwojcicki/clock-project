/* DCF-77 bench generator, Nucleo-F411RE. Skeleton: clock, SysTick, LED, UART banner. */
#include "dcf77_encode.h"
#include "stm32f411_regs.h"

static uint32_t f_cpu;
static bool clk_ext;
static volatile uint32_t ms_ticks;

void SysTick_Handler(void)
{
    ms_ticks++;
    if (ms_ticks % 500 == 0) {
        GPIOA_BSRR = (ms_ticks % 1000 == 0) ? (1u << (PIN_LED + 16)) : (1u << PIN_LED);
    }
}

/* RM0383 6.3.1: HSEBYP+HSEON, bounded wait for HSERDY, else stay on HSI (16 MHz). */
static void clock_init(void)
{
    RCC_CR |= RCC_CR_HSEBYP | RCC_CR_HSEON;
    for (uint32_t i = 0; i < 100000 && !(RCC_CR & RCC_CR_HSERDY); i++) { }
    if (RCC_CR & RCC_CR_HSERDY) {
        RCC_CFGR = (RCC_CFGR & ~3u) | RCC_CFGR_SW_HSE;
        for (uint32_t i = 0; i < 100000 && (RCC_CFGR & RCC_CFGR_SWS_MASK) != RCC_CFGR_SWS_HSE; i++) { }
    }
    if ((RCC_CFGR & RCC_CFGR_SWS_MASK) == RCC_CFGR_SWS_HSE) {
        clk_ext = true;
        f_cpu = 8000000;
    } else {
        RCC_CR &= ~(RCC_CR_HSEON | RCC_CR_HSEBYP);
        f_cpu = 16000000;
    }
}

static void gpio_init(void)
{
    RCC_AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    GPIOA_MODER = (GPIOA_MODER & ~((3u << 0) | (3u << 4) | (3u << 6) | (3u << 10))) |
                  (MODER_OUT << (2 * PIN_DCF)) | (MODER_AF << 4) | (MODER_AF << 6) |
                  (MODER_OUT << (2 * PIN_LED));
    GPIOA_AFRL = (GPIOA_AFRL & ~0xFF00u) | (7u << 8) | (7u << 12); /* PA2/PA3 = AF7 */
}

static void uart_init(void)
{
    RCC_APB1ENR |= RCC_APB1ENR_USART2EN;
    USART2_BRR = (f_cpu + 115200 / 2) / 115200; /* oversampling 16, PCLK1 = HCLK */
    USART2_CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

static void putc_(char c)
{
    while (!(USART2_SR & USART_SR_TXE)) { }
    USART2_DR = (uint32_t)c;
}

static void puts_(const char *s)
{
    while (*s) {
        if (*s == '\n') {
            putc_('\r');
        }
        putc_(*s++);
    }
}

static void put_num(unsigned v, unsigned digits)
{
    char b[5];
    for (unsigned i = digits; i-- > 0; v /= 10) {
        b[i] = (char)('0' + v % 10);
    }
    for (unsigned i = 0; i < digits; i++) {
        putc_(b[i]);
    }
}

static void put_time(const dcf77_time_t *t)
{
    put_num(t->year, 4); putc_('-'); put_num(t->month, 2); putc_('-'); put_num(t->day, 2);
    putc_(' '); put_num(t->hour, 2); putc_(':'); put_num(t->minute, 2);
    puts_(t->cest ? " CEST" : " CET");
}

/* __DATE__ = "Oct  6 2026", __TIME__ = "20:15:00" */
static dcf77_time_t build_time(void)
{
    static const char mon[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    const char *d = __DATE__, *t = __TIME__;
    dcf77_time_t r = {0};
    for (uint8_t m = 0; m < 12; m++) {
        if (d[0] == mon[3 * m] && d[1] == mon[3 * m + 1] && d[2] == mon[3 * m + 2]) {
            r.month = (uint8_t)(m + 1);
        }
    }
    r.day = (uint8_t)((d[4] == ' ' ? 0 : d[4] - '0') * 10 + (d[5] - '0'));
    r.year = (uint16_t)((d[7] - '0') * 1000 + (d[8] - '0') * 100 + (d[9] - '0') * 10 + (d[10] - '0'));
    r.hour = (uint8_t)((t[0] - '0') * 10 + (t[1] - '0'));
    r.minute = (uint8_t)((t[3] - '0') * 10 + (t[4] - '0'));
    r.cest = true;
    return r;
}

static void banner(const dcf77_time_t *t)
{
    puts_("\nDCF-77 generator, Nucleo-F411RE\n");
    puts_("build: " __DATE__ " " __TIME__ "\n");
    puts_(clk_ext ? "clock: external 8 MHz (HSE bypass, ST-LINK MCO)\n"
                  : "clock: internal 16 MHz HSI (HSE not ready)\n");
    puts_("default time: ");
    put_time(t);
    puts_("\ncommands: (none yet)\n");
}

int main(void)
{
    clock_init();
    gpio_init();
    uart_init();

    dcf77_time_t t = build_time();
    banner(&t);

    SYST_RVR = f_cpu / 1000 - 1;
    SYST_CVR = 0;
    SYST_CSR = SYST_CSR_CLKSRC | SYST_CSR_TICKINT | SYST_CSR_ENABLE;

    for (;;) {
        __asm volatile("wfi");
    }
}
