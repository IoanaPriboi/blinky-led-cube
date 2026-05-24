/*
 * timer.h - Blinky LED Cube
 *
 * Modul pentru gestiunea bazei de timp si a countdown-ului.
 * Folosesc Timer1 in mod CTC pentru a genera o intrerupere la fiecare 1ms,
 * care incrementeaza systicks, declanseaza multiplexarea cubului si
 * decrementeaza countdown-ul.
 *
 * Functionalitate din laborator: Lab 2 - Uptime (systicks, citire atomica),
 *                                Lab 3 - Timer1 CTC (configurare prescaler)
 */

#ifndef TIMER_H
#define TIMER_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

/* Frecventa CPU - necesara pentru calculul OCR1A */
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/* Timer1 CTC - 1ms per intrerupere
 * OCR1A = F_CPU / (prescaler * f_int) - 1
 * OCR1A = 16000000 / (8 * 1000) - 1 = 1999
 * Referinta: Lab 3 - Timer1_init_systicks() */
#define TIMER1_PRESCALER  8
#define TIMER1_OCR1A_1MS  1999

/* Multiplexare - schimb stratul la fiecare MUX_PERIOD_MS milisecunde
 * 4 straturi x 2ms = 8ms per ciclu = 125Hz, suficient pentru a evita flickerul */
#define MUX_PERIOD_MS  2

/* Systicks - milisecunde de la pornire, incrementat in ISR
 * Acelasi concept ca g_uptime_ms din Lab 2 - uptime.c */
extern volatile uint32_t systicks;

/* Countdown timer - secunde ramase si starea curenta */
extern volatile uint16_t timer_seconds_left;
extern volatile uint8_t  timer_running;
extern volatile uint8_t  timer_expired;

/* Indexul stratului curent pentru multiplexare - scris in ISR, citit in led_cube */
extern volatile uint8_t  current_layer;

/* Macro pentru verificarea timpului scurs fara blocare
 * Identic cu SYSTICKS_PASSED din Lab 3 - timers.h
 * Folosesc scaderea fara semn ca sa fie corect si la overflow pe 32 biti */
#define TICKS_PASSED(last, interval) \
    ((uint32_t)(systicks - (last)) >= (interval))

/* Initializeaza Timer1 in mod CTC cu prescaler 8
 * Genereaza intrerupere la fiecare 1ms pentru:
 *   - systicks (baza de timp globala)
 *   - multiplexarea cubului LED (125Hz)
 *   - countdown in secunde
 */
void timer_init(void);

/* Porneste countdown-ul de la numarul de secunde specificat
 * @param seconds - durata in secunde */
void timer_start(uint16_t seconds);

/* Opreste countdown-ul */
void timer_stop(void);

/* Returneaza valoarea curenta a systicks
 * Citire atomica - dezactiveaza intreruperile pe durata citirii
 * Necesar deoarece systicks e pe 32 biti, iar AVR e pe 8 biti */
uint32_t timer_get_ticks(void);

#endif /* TIMER_H */
