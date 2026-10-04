#ifndef BUZZER_H
#define BUZZER_H

/** BUZZER_EN steady high (for a self-oscillating buzzer). */
void buzzer_steady_on(void);

/** 2.7 kHz 50 % square wave on BUZZER_EN via PWM0 (for an external-drive buzzer). */
void buzzer_tone_start(void);

/** Stop PWM0, release the pin, drive BUZZER_EN low. Safe to call any time. */
void buzzer_off(void);

#endif
