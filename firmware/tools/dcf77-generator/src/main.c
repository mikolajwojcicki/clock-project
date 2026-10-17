/* DCF-77 bench generator, Nucleo-F411RE: PA0 pulses, UART time set + faults. */
#include "dcf77_encode.h"
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/usart.h>

/* Board: Nucleo-F411RE. LD2 = PA5; DCF output = PA0 (A0); VCP = PA2/PA3 (AF7) */
#define PIN_DCF 0u
#define PIN_LED 5u
#define PIN_CARRIER 6u /* D12, TIM3_CH1 */

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
    timer_set_oc_value(TIM3, TIM_OC1,
                       (carrier_ok && carrier_on) ? (pin_high ? pulse_ccr[level] : idle_ccr[level]) : 0);
}

static void pin_set(bool on)
{
    pin_high = on;
    carrier_apply();
    if (on) {
        gpio_set(GPIOA, (1u << PIN_DCF) | (1u << PIN_LED));
    } else {
        gpio_clear(GPIOA, (1u << PIN_DCF) | (1u << PIN_LED));
    }
}

/*
 * 1 ms tick. Second 0..58 start a 100/200 ms pulse; second 59 stays low.
 * Starts at sec 59 / ms 999 so the first tick opens frame 1 (next_bits).
 */
void sys_tick_handler(void)
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
 * PLL to 77.5 MHz (see design.md): VCO 310 MHz, P=4. HSE bypass: M=8; HSI: M=16.
 * libopencm3 sets flash wait states and APB1 /2 before the switch. Carrier only from HSE.
 * Unlike the register-level version, the library waits for PLL lock without a timeout;
 * the HSE start-up timeout (the fallback that matters on the bench) is kept.
 */
#define CLK77(m, src)                                                                         \
    {                                                                                         \
        .pllm = (m), .plln = 310, .pllp = 4, .pllq = 7, .pllr = 0, .pll_source = (src),       \
        .flash_config = FLASH_ACR_DCEN | FLASH_ACR_ICEN | FLASH_ACR_LATENCY_2WS,              \
        .hpre = RCC_CFGR_HPRE_NODIV, .ppre1 = RCC_CFGR_PPRE_DIV2,                             \
        .ppre2 = RCC_CFGR_PPRE_NODIV, .voltage_scale = PWR_SCALE1,                            \
        .ahb_frequency = 77500000, .apb1_frequency = 38750000, .apb2_frequency = 77500000,    \
    }
static const struct rcc_clock_scale clk_hse = CLK77(8, RCC_CFGR_PLLSRC_HSE_CLK);
static const struct rcc_clock_scale clk_hsi = CLK77(16, RCC_CFGR_PLLSRC_HSI_CLK);

static void clock_init(void)
{
    bool hse = false;
#ifndef GEN_FORCE_HSI
    rcc_osc_bypass_enable(RCC_HSE);
    rcc_osc_on(RCC_HSE);
    for (uint32_t i = 0; i < 100000 && !hse; i++) {
        hse = rcc_is_osc_ready(RCC_HSE);
    }
#endif
    rcc_clock_setup_pll(hse ? &clk_hse : &clk_hsi);
    clk_ext = hse;
    f_cpu = rcc_ahb_frequency;
    f_pclk1 = rcc_apb1_frequency;
    carrier_ok = clk_ext;
}

static void gpio_init(void)
{
    rcc_periph_clock_enable(RCC_GPIOA);
    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, (1u << PIN_DCF) | (1u << PIN_LED));
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2 | GPIO3);
    gpio_set_af(GPIOA, GPIO_AF7, GPIO2 | GPIO3); /* USART2 via ST-LINK VCP */
}

static void carrier_init(void)
{
    rcc_periph_clock_enable(RCC_TIM3);
    timer_set_prescaler(TIM3, 0);
    timer_set_period(TIM3, CARRIER_ARR);
    timer_set_oc_mode(TIM3, TIM_OC1, TIM_OCM_PWM1);
    timer_enable_oc_preload(TIM3, TIM_OC1);
    carrier_apply(); /* CCR = 0 unless carrier_ok */
    timer_enable_oc_output(TIM3, TIM_OC1);
    timer_enable_preload(TIM3);
    timer_generate_event(TIM3, TIM_EGR_UG); /* load preloaded CCR now */
    timer_enable_counter(TIM3);
    gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, 1u << PIN_CARRIER);
    gpio_set_af(GPIOA, GPIO_AF2, 1u << PIN_CARRIER);
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, 1u << PIN_CARRIER);
}

static void uart_init(void)
{
    rcc_periph_clock_enable(RCC_USART2);
    usart_set_baudrate(USART2, 115200);
    usart_set_databits(USART2, 8);
    usart_set_stopbits(USART2, USART_STOPBITS_1);
    usart_set_parity(USART2, USART_PARITY_NONE);
    usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);
    usart_set_mode(USART2, USART_MODE_TX_RX);
    usart_enable(USART2);
}

static void putc_(char c)
{
    usart_send_blocking(USART2, (uint16_t)c);
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

void usart2_isr(void)
{
    while (usart_get_flag(USART2, USART_SR_RXNE)) { /* reading DR also clears ORE */
        uint8_t c = (uint8_t)usart_recv(USART2);
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

    usart_enable_rx_interrupt(USART2);
    nvic_enable_irq(NVIC_USART2_IRQ);

    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
    systick_set_reload(f_cpu / 1000 - 1);
    systick_clear();
    systick_interrupt_enable();
    systick_counter_enable();

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
