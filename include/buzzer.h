/*
 * buzzer.h - Blinky LED Cube
 *
 * Modul pentru generarea de sunete prin buzzerul pasiv de pe D6 (PD6).
 * Folosesc Timer0 in mod CTC cu toggle hardware pe OC0A (PD6) -
 * hardware-ul genereaza frecventa automat, fara ISR dedicat.
 * Frecventele predefinite le-am ales din scara muzicala din Lab 3 (sound.c):
 * ALERT = 880Hz corespunde notei 'aH' din melodia de acolo.
 *
 * Functionalitate din laborator: Lab 3 - Timer0 CTC (Timer0_init_ctc()),
 *                                sunet prin toggle pe pin (sound.c, timers.c)
 */

#ifndef BUZZER_H
#define BUZZER_H

#include <avr/io.h>
#include <stdint.h>

/* Buzzerul e pe D6 = PD6 = OC0A - trebuie sa fie pe acest pin
 * pentru a putea folosi toggle-ul hardware al Timer0 */
#define BUZZER_PIN  PD6

/* Frecvente predefinite in Hz
 * Valorile corespund notelor muzicale din Lab 3 - sound.c:
 *   ALERT = aH = 880Hz (nota 'la' in octava inalta) */
#define BUZZER_FREQ_LOW    500
#define BUZZER_FREQ_MID   1000
#define BUZZER_FREQ_HIGH  2000
#define BUZZER_FREQ_ALERT  880

/* Initializeaza pinul buzzerului ca iesire si configureaza Timer0 CTC
 * Buzzerul porneste oprit - OC0A deconectat de la pin */
void buzzer_init(void);

/* Porneste buzzerul la frecventa specificata - non-blocant
 * Calculeaza OCR0A si conecteaza OC0A la pin
 * @param freq_hz - frecventa dorita in Hz */
void buzzer_start(uint16_t freq_hz);

/* Opreste buzzerul - deconecteaza OC0A si aduce pinul pe LOW */
void buzzer_stop(void);

/* Emite un beep de durata specificata - BLOCANT
 * Folosit doar la initializare (beep de pornire), nu in bucla principala
 * @param freq_hz     - frecventa in Hz
 * @param duration_ms - durata in milisecunde */
void buzzer_beep(uint16_t freq_hz, uint16_t duration_ms);

#endif /* BUZZER_H */
