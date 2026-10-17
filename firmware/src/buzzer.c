#include "buzzer.h"

#include <stdbool.h>
#include <stdint.h>

#include "dk_breadboard_pins.h"
#include "nrf_gpio.h"
#include "nrfx_pwm.h"

/* 16 MHz / 16 = 1 MHz; 1 MHz / 370 = 2.7 kHz, under the 10 kHz limit for P0.31. */
#define TONE_TOP 370

static const nrfx_pwm_t s_pwm = NRFX_PWM_INSTANCE(0);
static bool s_pwm_on;

/* EasyDMA reads the sequence, so it must be in RAM: not const. */
static nrf_pwm_values_common_t s_tone_duty = TONE_TOP / 2;

static const nrf_pwm_sequence_t s_tone_seq = {
    .values.p_common = &s_tone_duty,
    .length          = 1,
    .repeats         = 0,
    .end_delay       = 0,
};

void buzzer_steady_on(void)
{
    nrf_gpio_pin_set(BUZZER_EN);
}

void buzzer_tone_start(void)
{
    if (s_pwm_on)
    {
        return;
    }
    const nrfx_pwm_config_t cfg = {
        .output_pins  = { BUZZER_EN, NRFX_PWM_PIN_NOT_USED,
                          NRFX_PWM_PIN_NOT_USED, NRFX_PWM_PIN_NOT_USED },
        .irq_priority = NRFX_PWM_DEFAULT_CONFIG_IRQ_PRIORITY,
        .base_clock   = NRF_PWM_CLK_1MHz,
        .count_mode   = NRF_PWM_MODE_UP,
        .top_value    = TONE_TOP,
        .load_mode    = NRF_PWM_LOAD_COMMON,
        .step_mode    = NRF_PWM_STEP_AUTO,
    };
    if (nrfx_pwm_init(&s_pwm, &cfg, NULL) != NRFX_SUCCESS)
    {
        return;
    }
    s_pwm_on = true;
    /* LOOP repeats the one-value sequence until nrfx_pwm_stop. */
    nrfx_pwm_simple_playback(&s_pwm, &s_tone_seq, 1, NRFX_PWM_FLAG_LOOP);
}

void buzzer_off(void)
{
    if (s_pwm_on)
    {
        nrfx_pwm_stop(&s_pwm, true);
        nrfx_pwm_uninit(&s_pwm);
        s_pwm_on = false;
    }
    nrf_gpio_pin_clear(BUZZER_EN);
    nrf_gpio_cfg_output(BUZZER_EN);
}
