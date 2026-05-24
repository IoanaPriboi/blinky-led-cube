/*
 * animations.h - Blinky LED Cube
 *
 * Modul pentru animatiile cubului LED 4x4x4.
 * Animatiile sunt non-blocante - fiecare apel la animations_update()
 * returneaza imediat daca nu a trecut destul timp de la ultimul frame.
 * Bufferul cube_state[] e actualizat direct, ISR-ul se ocupa de multiplexare.
 *
 * Functionalitate din laborator: Lab 2 - pattern non-blocant cu TICKS_PASSED,
 *                                uptime_ms() ca baza de timp
 */

#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <stdint.h>

#define CUBE_LAYERS   4
#define CUBE_COLUMNS  16

/* Animatiile disponibile */
typedef enum {
    ANIM_RAIN = 0,
    ANIM_PLANE_SCAN,
    ANIM_EXPAND,
    ANIM_SPIRAL,
    ANIM_FIREWORK,
    ANIM_SPARKLE,
    ANIM_COUNT
} animation_id_t;

/* Initializeaza modulul - reseteaza starea interna si seteaza RAIN ca implicit
 * @param seed - valoare pentru srand(), de obicei adc_read(POT_CHANNEL) la startup */
void animations_init(uint16_t seed);

/* Seteaza animatia curenta si reseteaza starea ei interna
 * @param anim - ID-ul animatiei */
void animations_set(animation_id_t anim);

/* Returneaza ID-ul animatiei curente */
animation_id_t animations_get_current(void);

/* Trece la urmatoarea animatie cu wraparound */
void animations_next(void);

/* Returneaza numele animatiei curente - pentru afisare pe LCD */
const char *animations_get_name(void);

/* Sterge frame-ul (toate straturile pe 0) */
void animations_clear(uint16_t frame[CUBE_LAYERS]);

/* Actualizeaza frame-ul cubului - non-blocant
 * Returneaza imediat daca nu a trecut period_ms de la ultimul update
 * @param now_ms    - timpul curent din timer_get_ticks()
 * @param period_ms - intervalul intre frame-uri (din potentiometru)
 * @param frame     - bufferul de scris */
void animations_update(uint32_t now_ms, uint16_t period_ms,
                       uint16_t frame[CUBE_LAYERS]);

#endif /* ANIMATIONS_H */
