/*
 * buzzer.c - Blinky LED Cube
 *
 * Generez sunet prin Timer0 CTC cu toggle hardware pe OC0A (PD6).
 * Setez COM0A0 = 1 si hardware-ul comuta pinul automat la fiecare
 * compare match - nu am nevoie de ISR separat pentru buzzer.
 *
 * Formula frecventei: f = F_CPU / (2 * prescaler * (OCR0A + 1))
 * Cu prescaler 64: OCR0A = 16000000 / (2 * 64 * freq) - 1
 *                        = 125000 / freq - 1
 *
 * Functionalitate din laborator: Lab 3 - Timer0 CTC (Timer0_init_ctc()),
 *                                toggle pin pentru generare sunet (timers.c)
 */

#include "buzzer.h"
#include <util/delay.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

void buzzer_init(void)
{
    /* Setez PD6 (OC0A) ca iesire */
    DDRD  |=  (1 << BUZZER_PIN);
    PORTD &= ~(1 << BUZZER_PIN);

    /* Configurez Timer0 in mod CTC cu toggle pe OC0A
     * WGM01 = 1 => mod CTC, COM0A0 = 1 => toggle la compare match
     * Referinta: Lab 3 - Timer0_init_ctc() */
    TCCR0A = (1 << WGM01) | (1 << COM0A0);
    TCCR0B = 0;

    /* Deconectez OC0A - buzzerul porneste oprit */
    TCCR0A &= ~(1 << COM0A0);
}

void buzzer_start(uint16_t freq_hz)
{
    /* OCR0A = 125000 / freq - 1 (prescaler 64) */
    OCR0A = (uint8_t)(125000UL / freq_hz - 1);

    /* Resetez contorul ca sa incep de la zero */
    TCNT0 = 0;

    /* Conectez OC0A la pin si pornesc Timer0 cu prescaler 64 */
    TCCR0A |= (1 << COM0A0);
    TCCR0B = (1 << CS01) | (1 << CS00);
}

void buzzer_stop(void)
{
    /* Deconectez OC0A si opresc Timer0 */
    TCCR0A &= ~(1 << COM0A0);
    TCCR0B = 0;

    /* Aduc pinul la LOW - evit sa ramana blocat pe HIGH */
    PORTD &= ~(1 << BUZZER_PIN);
}

void buzzer_beep(uint16_t freq_hz, uint16_t duration_ms)
{
    buzzer_start(freq_hz);

    /* Astept durata - blocant, folosit doar la pornire */
    while (duration_ms--) {
        _delay_ms(1);
    }

    buzzer_stop();
}
