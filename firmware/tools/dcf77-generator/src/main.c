/* DCF-77 bench generator, Nucleo-F411RE: PA0 pulses, UART time set + faults. */
#include "dcf77_encode.h"
#include "stm32f411_regs.h"

static uint32_t f_cpu, f_pclk1;
static bool clk_ext;

/* Carrier (TIM3 CH1 on PA6). Duty D gives fundamental ~ sin(pi*D); pulse = 15% dip. */
#define CARRIER_ARR 999u
static const uint16_t idle_ccr[4]  = {500, 167, 80, 32};
static const uint16_t pulse_ccr[4] = {48, 24, 12, 5};
static const char *const level_name[4] = {"0 dB", "-6 dB", "-12 dB", "-20 dB"};
static volatile bool pin_high, carrier_on = true;
static volatile uint8_t level = 3;
static bool carrier_ok;

/* ISR <-> main shared state. Main writes next_bits only inside irq_off(). */
static volatile uint64_t cur_bits, next_bits;
static volatile bool frame_flag;
static volatile bool silence_req, miss_req, glitch_req;

static void irq_off(void) { __asm volatile("cpsid i" ::: "memory"); }
static void irq_on(void) { __asm volatile("cpsie i" ::: "memory"); }

static void carrier_apply(void)
{
    TIM3_CCR1 = (carrier_ok && carrier_on) ? (pin_high ? pulse_ccr[level] : idle_ccr[level]) : 0;
}

static void pin_set(bool on)
{
    pin_high = on;
    carrier_apply();
    GPIOA_BSRR = on ? ((1u << PIN_DCF) | (1u << PIN_LED))
                    : ((1u << (PIN_DCF + 16)) | (1u << (PIN_LED + 16)));
}

/*
 * 1 ms tick. Second 0..58 start a 100/200 ms pulse; second 59 stays low.
 * Starts at sec 59 / ms 999 so the first tick opens frame 1 (next_bits).
 */
void SysTick_Handler(void)
{
    static uint16_t ms = 999;
    static uint8_t sec = 59;
    static uint16_t pulse_end;
    static bool pulse_on, glitch_on, silence_active;

    if (++ms >= 1000) {
        ms = 0;
        if (++sec >= 60) {
            sec = 0;
            cur_bits = next_bits;
            frame_flag = true;
            if (!silence_req) {
                silence_active = false; /* silence ends at a frame start */
            }
        }
    }
    if (silence_req) {
        silence_active = true;
    }
    if (silence_active) {
        pulse_on = glitch_on = false;
        pin_set(false);
        return;
    }
    if (ms == 0) {
        if (sec <= 58) {
            if (miss_req) {
                miss_req = false;
            } else {
                pulse_end = ((cur_bits >> sec) & 1) ? 200 : 100;
                pulse_on = true;
                pin_set(true);
            }
        }
        glitch_on = glitch_req; /* one 10 ms glitch at ms 500 of this second */
        glitch_req = false;
    } else if (pulse_on && ms == pulse_end) {
        pulse_on = false;
        pin_set(false);
    } else if (glitch_on && ms == 500) {
        pin_set(true);
    } else if (glitch_on && ms == 510) {
        glitch_on = false;
        pin_set(false);
    }
}

/*
 * RM0383 6.3: PLL to 77.5 MHz (see design.md). HSE bypass: M=8 N=310 P=4; HSI: M=16.
 * Flash wait states and APB1 /2 are set before the switch. Carrier only from HSE.
 */
static void clock_init(void)
{
    bool hse = false;
#ifndef GEN_FORCE_HSI
    RCC_CR |= RCC_CR_HSEBYP | RCC_CR_HSEON;
    for (uint32_t i = 0; i < 100000 && !(RCC_CR & RCC_CR_HSERDY); i++) { }
    hse = RCC_CR & RCC_CR_HSERDY;
#endif
    RCC_PLLCFGR = hse ? PLLCFGR(8u, 310u, 1u, true) : PLLCFGR(16u, 310u, 1u, false);
    FLASH_ACR = FLASH_ACR_2WS_CACHE;
    RCC_CFGR |= RCC_CFGR_PPRE1_DIV2;
    RCC_CR |= RCC_CR_PLLON;
    for (uint32_t i = 0; i < 100000 && !(RCC_CR & RCC_CR_PLLRDY); i++) { }
    if (RCC_CR & RCC_CR_PLLRDY) {
        RCC_CFGR = (RCC_CFGR & ~3u) | RCC_CFGR_SW_PLL;
        for (uint32_t i = 0; i < 100000 && (RCC_CFGR & RCC_CFGR_SWS_MASK) != RCC_CFGR_SWS_PLL; i++) { }
    }
    if ((RCC_CFGR & RCC_CFGR_SWS_MASK) == RCC_CFGR_SWS_PLL) {
        clk_ext = hse;
        f_cpu = 77500000;
        f_pclk1 = f_cpu / 2;
    } else { /* PLL failed: stay on HSI 16 MHz, APB1 undivided */
        RCC_CFGR &= ~RCC_CFGR_PPRE1_DIV2;
        f_cpu = f_pclk1 = 16000000;
    }
    carrier_ok = clk_ext;
}

static void gpio_init(void)
{
    RCC_AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    GPIOA_MODER = (GPIOA_MODER & ~((3u << 0) | (3u << 4) | (3u << 6) | (3u << 10))) |
                  (MODER_OUT << (2 * PIN_DCF)) | (MODER_AF << 4) | (MODER_AF << 6) |
                  (MODER_OUT << (2 * PIN_LED));
    GPIOA_AFRL = (GPIOA_AFRL & ~0xFF00u) | (7u << 8) | (7u << 12); /* PA2/PA3 = AF7 */
}

static void carrier_init(void)
{
    RCC_APB1ENR |= RCC_APB1ENR_TIM3EN;
    TIM3_PSC = 0;
    TIM3_ARR = CARRIER_ARR;
    TIM3_CCMR1 = TIM_CCMR1_OC1M_PWM1 | TIM_CCMR1_OC1PE;
    carrier_apply(); /* CCR = 0 unless carrier_ok */
    TIM3_CCER = TIM_CCER_CC1E;
    TIM3_CR1 = TIM_CR1_ARPE | TIM_CR1_CEN;
    TIM3_EGR = TIM_EGR_UG; /* load preloaded CCR now */
    GPIOA_OSPEEDR |= 3u << (2 * PIN_CARRIER);
    GPIOA_AFRL = (GPIOA_AFRL & ~(0xFu << (4 * PIN_CARRIER))) | (2u << (4 * PIN_CARRIER));
    GPIOA_MODER = (GPIOA_MODER & ~(3u << (2 * PIN_CARRIER))) | (MODER_AF << (2 * PIN_CARRIER));
}

static void uart_init(void)
{
    RCC_APB1ENR |= RCC_APB1ENR_USART2EN;
    USART2_BRR = (f_pclk1 + 115200 / 2) / 115200; /* oversampling 16 */
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

static const char help[] =
    "commands (single key, no Enter):\n"
    "  p  parity fault in next frame\n"
    "  d  wrong-date fault in next frame (day+1, parity correct)\n"
    "  m  missing pulse in next second\n"
    "  g  10 ms glitch pulse in next second\n"
    "  s  toggle silence (PA0 low until pressed again, resumes at frame start)\n"
    "  l  cycle carrier level 0 / -6 / -12 / -20 dB\n"
    "  r  carrier on/off (PA6)\n"
    "  n  clear pending faults\n"
    "  ?  this list\n"
    "  T YYYY-MM-DD HH:MM S|W + Enter  set time (S=CEST, W=CET), next frame\n";

static void carrier_report(void)
{
    puts_("carrier: 77.5 kHz on PA6, ");
    puts_(carrier_on ? "on, level " : "off, level ");
    puts_(level_name[level]);
    puts_(", CCR idle=");
    put_num(idle_ccr[level], 3);
    puts_(" pulse=");
    put_num(pulse_ccr[level], 3);
    putc_('\n');
}

static void banner(const dcf77_time_t *t)
{
    puts_("\nDCF-77 generator, Nucleo-F411RE\n");
    puts_("build: " __DATE__ " " __TIME__ "\n");
    puts_(clk_ext ? "clock: 77.5 MHz PLL from external 8 MHz (HSE bypass, ST-LINK MCO)\n"
                  : f_cpu == 77500000 ? "clock: 77.5 MHz PLL from internal HSI (HSE off or not ready)\n"
                                      : "clock: internal 16 MHz HSI (PLL not locked)\n");
    if (!carrier_ok) {
        puts_("carrier: DISABLED (needs external clock), PA6 low\n");
    } else {
        carrier_report();
    }
    puts_("default time: ");
    put_time(t);
    putc_('\n');
    puts_(help);
}

/* ---- UART RX: ISR fills a ring, main drains it ---- */
static volatile uint8_t rx_buf[64];
static volatile uint8_t rx_head, rx_tail;

void USART2_IRQHandler(void)
{
    while (USART2_SR & USART_SR_RXNE) { /* reading DR also clears ORE */
        uint8_t c = (uint8_t)USART2_DR;
        uint8_t n = (uint8_t)((rx_head + 1) % sizeof rx_buf);
        if (n != rx_tail) {
            rx_buf[rx_head] = c;
            rx_head = n;
        }
    }
}

/* ---- frame sequence (main only) ---- */
static dcf77_time_t next_t;      /* true time of the next frame */
static dcf77_time_t next_enc_t;  /* date actually encoded (differs on wrong-date fault) */
static bool fault_p, fault_d;    /* one-shot, consumed when the frame starts */

static void build_next(void)
{
    next_enc_t = next_t;
    if (fault_d) {
        next_enc_t.day++;
        if (!dcf77_time_valid(&next_enc_t)) {
            next_enc_t.day = 1;
        }
    }
    uint64_t b = dcf77_encode(&next_enc_t);
    if (fault_p) {
        b ^= 1ull << 28; /* P1 */
    }
    irq_off();
    next_bits = b;
    irq_on();
}

static void log_frame(const dcf77_time_t *t, uint64_t bits, bool p, bool d)
{
    static const char wd[] = "MonTueWedThuFriSatSun";
    uint8_t w = dcf77_weekday(t->year, t->month, t->day);
    puts_("frame ");
    put_time(t);
    putc_(' ');
    for (unsigned i = 0; i < 3; i++) {
        putc_(wd[3 * (w - 1) + i]);
    }
    puts_(" bits=");
    for (unsigned i = 0; i < 59; i++) {
        putc_(((bits >> i) & 1) ? '1' : '0');
    }
    if (p) {
        puts_(" [parity fault]");
    }
    if (d) {
        puts_(" [wrong-date fault]");
    }
    putc_('\n');
}

/* ---- command parsing ---- */
static bool digits(const char *s, unsigned n, unsigned *out)
{
    unsigned v = 0;
    for (unsigned i = 0; i < n; i++) {
        if (s[i] < '0' || s[i] > '9') {
            return false;
        }
        v = v * 10 + (unsigned)(s[i] - '0');
    }
    *out = v;
    return true;
}

/* "T YYYY-MM-DD HH:MM S|W" */
static void set_time_cmd(const char *l, unsigned len)
{
    dcf77_time_t t;
    unsigned y, mo, d, h, mi;
    if (len == 20 && l[1] == ' ' && l[6] == '-' && l[9] == '-' && l[12] == ' ' && l[15] == ':' &&
        l[18] == ' ' && (l[19] == 'S' || l[19] == 'W') && digits(l + 2, 4, &y) &&
        digits(l + 7, 2, &mo) && digits(l + 10, 2, &d) && digits(l + 13, 2, &h) &&
        digits(l + 16, 2, &mi)) {
        t = (dcf77_time_t){(uint16_t)y, (uint8_t)mo, (uint8_t)d, (uint8_t)h, (uint8_t)mi,
                           l[19] == 'S'};
        if (dcf77_time_valid(&t)) {
            next_t = t;
            build_next();
            puts_("OK: next frame will encode ");
            put_time(&t);
            putc_('\n');
            return;
        }
    }
    puts_("ERR: expected T YYYY-MM-DD HH:MM S|W with a real date; sequence unchanged\n");
}

static void key_cmd(char c)
{
    switch (c) {
    case 'p': fault_p = true; build_next(); puts_("fault armed: parity (next frame)\n"); break;
    case 'd': fault_d = true; build_next(); puts_("fault armed: wrong date (next frame)\n"); break;
    case 'm': miss_req = true; puts_("fault armed: missing pulse (next second)\n"); break;
    case 'g': glitch_req = true; puts_("fault armed: glitch (next second)\n"); break;
    case 's':
        silence_req = !silence_req;
        puts_(silence_req ? "silence ON\n" : "silence OFF (resumes at next frame start)\n");
        break;
    case 'l':
    case 'r':
        if (!carrier_ok) {
            puts_("carrier disabled (needs external clock)\n");
            break;
        }
        irq_off();
        if (c == 'l') {
            level = (uint8_t)((level + 1) % 4);
        } else {
            carrier_on = !carrier_on;
        }
        carrier_apply();
        irq_on();
        carrier_report();
        break;
    case 'n':
        fault_p = fault_d = false;
        miss_req = glitch_req = false;
        build_next();
        puts_("pending faults cleared\n");
        break;
    case '?': puts_(help); break;
    default: break;
    }
}

static void rx_char(char c)
{
    static char line[24];
    static unsigned len;

    if (len == 0) {
        if (c == 'T' || c == 't') {
            line[len++] = 'T';
            putc_('T');
        } else {
            key_cmd(c);
        }
    } else if (c == '\r' || c == '\n') {
        putc_('\n');
        set_time_cmd(line, len);
        len = 0;
    } else if ((c == 0x7f || c == 0x08) && len > 0) {
        len--;
        puts_("\b \b");
    } else if (len < sizeof line - 1) {
        line[len++] = c;
        putc_(c);
    }
}

int main(void)
{
    clock_init();
    gpio_init();
    uart_init();
    carrier_init();

    next_t = build_time();
    banner(&next_t);
    build_next();

    USART2_CR1 |= USART_CR1_RXNEIE;
    NVIC_ISER1 = 1u << (USART2_IRQn - 32);

    SYST_RVR = f_cpu / 1000 - 1;
    SYST_CVR = 0;
    SYST_CSR = SYST_CSR_CLKSRC | SYST_CSR_TICKINT | SYST_CSR_ENABLE;

    for (;;) {
        if (frame_flag) {
            /* The frame in next_t/next_enc_t is now on the wire. Log it, then queue the next. */
            frame_flag = false;
            log_frame(&next_enc_t, cur_bits, fault_p, fault_d);
            fault_p = fault_d = false;
            dcf77_time_next_minute(&next_t);
            build_next();
        }
        while (rx_tail != rx_head) {
            char c = (char)rx_buf[rx_tail];
            rx_tail = (uint8_t)((rx_tail + 1) % sizeof rx_buf);
            rx_char(c);
        }
        __asm volatile("wfi");
    }
}
