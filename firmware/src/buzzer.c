#include "buzzer.h"

#include <stdint.h>

#include "dk_breadboard_pins.h"
#include "nrf.h"
#include "nrf_gpio.h"

/* 16 MHz / 16 = 1 MHz; 1 MHz / 370 = 2.7 kHz, under the 10 kHz limit for P0.31. */
#define TONE_TOP  370
#define STOP_SPIN 100000

/* EasyDMA reads the sequence, so it must be in RAM: not const. */
static uint16_t s_tone_duty = TONE_TOP / 2;

void buzzer_steady_on(void)
{
    nrf_gpio_pin_set(BUZZER_EN);
}

void buzzer_tone_start(void)
{
    NRF_PWM0->PSEL.OUT[0] = BUZZER_EN;
    NRF_PWM0->ENABLE = PWM_ENABLE_ENABLE_Enabled << PWM_ENABLE_ENABLE_Pos;
    NRF_PWM0->MODE = PWM_MODE_UPDOWN_Up << PWM_MODE_UPDOWN_Pos;
    NRF_PWM0->PRESCALER = PWM_PRESCALER_PRESCALER_DIV_16 << PWM_PRESCALER_PRESCALER_Pos;
    NRF_PWM0->COUNTERTOP = TONE_TOP;
    NRF_PWM0->LOOP = 0;
    NRF_PWM0->DECODER = (PWM_DECODER_LOAD_Common << PWM_DECODER_LOAD_Pos) |
                        (PWM_DECODER_MODE_RefreshCount << PWM_DECODER_MODE_Pos);
    NRF_PWM0->SEQ[0].PTR = (uint32_t)&s_tone_duty;
    NRF_PWM0->SEQ[0].CNT = 1;
    NRF_PWM0->SEQ[0].REFRESH = 0;
    NRF_PWM0->SEQ[0].ENDDELAY = 0;
    /* After the one-value sequence ends, PWM0 keeps repeating that value. */
    NRF_PWM0->TASKS_SEQSTART[0] = 1;
}

void buzzer_off(void)
{
    nrf_gpio_pin_clear(BUZZER_EN);
    if (NRF_PWM0->ENABLE)
    {
        NRF_PWM0->EVENTS_STOPPED = 0;
        NRF_PWM0->TASKS_STOP = 1;
        for (uint32_t i = 0; i < STOP_SPIN && !NRF_PWM0->EVENTS_STOPPED; i++)
        {
        }
        NRF_PWM0->ENABLE = 0;
        NRF_PWM0->PSEL.OUT[0] = PWM_PSEL_OUT_CONNECT_Disconnected << PWM_PSEL_OUT_CONNECT_Pos;
    }
}
