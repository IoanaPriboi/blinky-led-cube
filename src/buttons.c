/*
 * buttons.c - Blinky LED Cube
 *
 * Implementarea citirii butoanelor prin PCINT (Pin Change Interrupt).
 * PCINT se declanseaza la orice schimbare de stare a pinului - atat la
 * apasare (LOW) cat si la eliberare (HIGH). Filtrez doar apasarea
 * verificand ca pinul e LOW in momentul intreruperii.
 * Debounce-ul il fac comparand systicks cu timestamp-ul ultimei apasari
 * valide - acelasi pattern ca in Lab 2 (ex2.c) dar cu filtrare temporala.
 *
 * BTN_UP si BTN_DOWN impart acelasi ISR (PCINT0_vect) deoarece sunt
 * amandoua pe PORTB - le verific pe fiecare separat in ISR.
 *
 * Functionalitate din laborator: Lab 2 - PCINT (ex2.c: PCICR, PCMSK,
 *                                PCIFR, flag volatile, ISR(PCINT_vect))
 */

#include "buttons.h"
#include "timer.h"

/* Flag-uri setate din ISR, resetate in main loop */
volatile uint8_t btn_mode_flag       = 0;
volatile uint8_t btn_up_flag         = 0;
volatile uint8_t btn_down_flag       = 0;
volatile uint8_t btn_start_stop_flag = 0;

/* Timestamp-uri pentru debounce - retin momentul ultimei apasari valide */
static uint32_t last_mode_press       = 0;
static uint32_t last_up_press         = 0;
static uint32_t last_down_press       = 0;
static uint32_t last_start_stop_press = 0;

void buttons_init(void)
{
    /* BTN_MODE - PD7: input cu pull-up intern */
    DDRD  &= ~(1 << BTN_MODE_PIN);
    PORTD |=  (1 << BTN_MODE_PIN);

    /* BTN_UP - PB0: input cu pull-up intern */
    DDRB  &= ~(1 << BTN_UP_PIN);
    PORTB |=  (1 << BTN_UP_PIN);

    /* BTN_DOWN - PB1: input cu pull-up intern */
    DDRB  &= ~(1 << BTN_DOWN_PIN);
    PORTB |=  (1 << BTN_DOWN_PIN);

    /* BTN_START_STOP - PC1: input cu pull-up intern */
    DDRC  &= ~(1 << BTN_START_STOP_PIN);
    PORTC |=  (1 << BTN_START_STOP_PIN);

    /* Activez PCINT pentru PORTD - BTN_MODE pe PCINT23
     * Referinta: Lab 2 ex2.c - PCICR, PCMSK, PCIFR */
    PCICR  |= (1 << PCIE2);
    PCMSK2 |= (1 << PCINT23);
    PCIFR  |= (1 << PCIF2);    /* sterg flag-ul pending ca sa nu am fals trigger */

    /* Activez PCINT pentru PORTB - BTN_UP (PCINT0) si BTN_DOWN (PCINT1)
     * Ambele pe PCIE0 - vor declansa acelasi ISR */
    PCICR  |= (1 << PCIE0);
    PCMSK0 |= (1 << PCINT0) | (1 << PCINT1);
    PCIFR  |= (1 << PCIF0);

    /* Activez PCINT pentru PORTC - BTN_START_STOP pe PCINT9 */
    PCICR  |= (1 << PCIE1);
    PCMSK1 |= (1 << PCINT9);
    PCIFR  |= (1 << PCIF1);
}

/* Citesc starea fizica - LOW inseamna apasat (pull-up + buton la GND) */
uint8_t btn_mode_pressed(void)       { return !(PIND & (1 << BTN_MODE_PIN));       }
uint8_t btn_up_pressed(void)         { return !(PINB & (1 << BTN_UP_PIN));         }
uint8_t btn_down_pressed(void)       { return !(PINB & (1 << BTN_DOWN_PIN));       }
uint8_t btn_start_stop_pressed(void) { return !(PINC & (1 << BTN_START_STOP_PIN)); }

/* ISR pentru PORTD - BTN_MODE */
ISR(PCINT2_vect)
{
    /* Verific ca butonul e apasat (LOW), pt ca PCINT se declanseaza si la eliberare */
    if (!btn_mode_pressed()) return;

    uint32_t now = timer_get_ticks();
    if ((uint32_t)(now - last_mode_press) >= DEBOUNCE_MS) {
        last_mode_press = now;
        btn_mode_flag   = 1;
    }
}

/* ISR pentru PORTB - BTN_UP si BTN_DOWN (impart acelasi vector) */
ISR(PCINT0_vect)
{
    uint32_t now = timer_get_ticks();

    /* Verific fiecare buton separat - oricare dintre ele putea sa declanseze ISR-ul */
    if (btn_up_pressed()) {
        if ((uint32_t)(now - last_up_press) >= DEBOUNCE_MS) {
            last_up_press = now;
            btn_up_flag   = 1;
        }
    }

    if (btn_down_pressed()) {
        if ((uint32_t)(now - last_down_press) >= DEBOUNCE_MS) {
            last_down_press = now;
            btn_down_flag   = 1;
        }
    }
}

/* ISR pentru PORTC - BTN_START_STOP */
ISR(PCINT1_vect)
{
    if (!btn_start_stop_pressed()) return;

    uint32_t now = timer_get_ticks();
    if ((uint32_t)(now - last_start_stop_press) >= DEBOUNCE_MS) {
        last_start_stop_press = now;
        btn_start_stop_flag   = 1;
    }
}
