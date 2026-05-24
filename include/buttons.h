/*
 * buttons.h - Blinky LED Cube
 *
 * Modul pentru citirea celor 4 butoane prin intreruperi PCINT.
 * Folosesc flag-uri volatile setate din ISR si procesate in main loop -
 * acelasi pattern cu btn1_flag din Lab 2 (ex2.c).
 * Debounce-ul il fac prin compararea timestamp-urilor systicks,
 * ignorand apasarile mai rapide de DEBOUNCE_MS milisecunde.
 *
 * Functionalitate din laborator: Lab 2 - PCINT (ex2.c: PCICR, PCMSK,
 *                                flag volatile, ISR(PCINT_vect))
 */

#ifndef BUTTONS_H
#define BUTTONS_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

/* Pinii butoanelor
 * Toate butoanele sunt active LOW - conectate la GND la apasare,
 * pull-up intern activat in software */
#define BTN_MODE_PIN        PD7   /* D7  - PCINT23, PCIE2 */
#define BTN_UP_PIN          PB0   /* D8  - PCINT0,  PCIE0 */
#define BTN_DOWN_PIN        PB1   /* D9  - PCINT1,  PCIE0 */
#define BTN_START_STOP_PIN  PC1   /* A1  - PCINT9,  PCIE1 */

/* Timp minim intre doua apasari valide - filtreaza zgomotul mecanic */
#define DEBOUNCE_MS  50

/* Flag-uri setate din ISR, procesate si resetate in main loop
 * Un flag = 1 inseamna ca butonul a fost apasat si debounce-ul a trecut */
extern volatile uint8_t btn_mode_flag;
extern volatile uint8_t btn_up_flag;
extern volatile uint8_t btn_down_flag;
extern volatile uint8_t btn_start_stop_flag;

/* Initializeaza butoanele ca intrari cu pull-up intern
 * si activeaza intreruperile PCINT corespunzatoare fiecarui port */
void buttons_init(void);

/* Citesc starea fizica a fiecarui buton (activ LOW cu pull-up)
 * Folosite in ISR pentru a verifica daca schimbarea e pe front descrescator */
uint8_t btn_mode_pressed(void);
uint8_t btn_up_pressed(void);
uint8_t btn_down_pressed(void);
uint8_t btn_start_stop_pressed(void);

#endif /* BUTTONS_H */
