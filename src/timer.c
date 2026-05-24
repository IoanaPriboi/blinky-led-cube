/*
 * timer.c - Blinky LED Cube
 *
 * Implementarea modulului de timp.
 * Configurez Timer1 in mod CTC cu prescaler 8 si OCR1A = 1999
 * pentru a obtine o intrerupere la exact 1ms la 16MHz.
 *
 * ISR-ul face trei lucruri in ordine:
 *   1. Incrementeaza systicks - baza de timp globala
 *   2. Declanseaza multiplexarea cubului la fiecare 2ms (125Hz)
 *   3. Decrementeaza countdown-ul in secunde
 *
 * Functionalitate din laborator:
 *   Lab 2 - uptime.c: patternul systicks + citire atomica cu cli/SREG
 *   Lab 3 - timers.c: configurarea Timer1 CTC cu prescaler 8, ISR systick
 */

#include "timer.h"
#include "led_cube.h"

/* Baza de timp globala - milisecunde de la pornire */
volatile uint32_t systicks = 0;

/* Starea countdown-ului */
volatile uint16_t timer_seconds_left = 0;
volatile uint8_t  timer_running      = 0;
volatile uint8_t  timer_expired      = 0;

/* Indexul stratului curent pentru multiplexare - avansat in ISR */
volatile uint8_t current_layer = 0;

/* Contor intern pentru multiplexare */
static volatile uint16_t mux_ticks = 0;

/* Contor intern pentru countdown - decrementez seconds_left la fiecare 1000ms */
static volatile uint16_t countdown_ticks = 0;

void timer_init(void)
{
    /* Resetez registrele Timer1 - pornesc dintr-o stare cunoscuta
     * Referinta: Lab 3 - Timer1_init_systicks() */
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;

    /* Mod CTC - WGM12 = 1
     * Numaratorul se reseteaza automat la OCR1A, nu am nevoie sa fac asta manual */
    TCCR1B |= (1 << WGM12);

    /* Prescaler 8 - CS11 = 1
     * 16MHz / 8 = 2MHz ceas timer => rezolutie 0.5us per tick */
    TCCR1B |= (1 << CS11);

    /* Valoarea de comparatie pentru perioada de 1ms
     * OCR1A = 16000000 / (8 * 1000) - 1 = 1999 */
    OCR1A = TIMER1_OCR1A_1MS;

    /* Activez intreruperea Compare Match A - se declanseaza la fiecare 1ms */
    TIMSK1 |= (1 << OCIE1A);
}

void timer_start(uint16_t seconds)
{
    timer_seconds_left = seconds;
    timer_expired      = 0;
    countdown_ticks    = 0;
    timer_running      = 1;
}

void timer_stop(void)
{
    timer_running = 0;
}

uint32_t timer_get_ticks(void)
{
    /* Citire atomica - dezactivez intreruperile pe durata citirii
     * systicks e pe 32 biti, iar AVR are registre de 8 biti, deci
     * citirea se face in 4 pasi - fara cli() risc sa citesc valori
     * inconsistente daca ISR-ul modifica variabila la mijlocul citirii
     * Acelasi pattern ca uptime_ms() din Lab 2 - uptime.c */
    uint32_t t;
    uint8_t sreg = SREG;
    cli();
    t = systicks;
    SREG = sreg;
    return t;
}

/* ISR Timer1 Compare Match A - apelata la fiecare 1ms */
ISR(TIMER1_COMPA_vect)
{
    /* ---- 1. Baza de timp globala ---- */
    systicks++;

    /* ---- 2. Multiplexare cub LED ---- */
    /* Schimb stratul activ la fiecare MUX_PERIOD_MS milisecunde
     * 4 straturi x 2ms = 8ms per ciclu complet = 125Hz */
    mux_ticks++;
    if (mux_ticks >= MUX_PERIOD_MS) {
        mux_ticks     = 0;
        current_layer = (current_layer + 1) % 4;
        led_cube_display_layer(current_layer);
    }

    /* ---- 3. Countdown timer ---- */
    if (timer_running) {
        countdown_ticks++;

        if (countdown_ticks >= 1000) {
            countdown_ticks = 0;

            if (timer_seconds_left > 0) {
                timer_seconds_left--;
            }

            /* Setez expired si opresc timer-ul cand ajunge la zero */
            if (timer_seconds_left == 0) {
                timer_running = 0;
                timer_expired = 1;
            }
        }
    }
}
