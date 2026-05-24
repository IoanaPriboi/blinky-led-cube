/*
 * led_cube.h - Blinky LED Cube
 *
 * Modul pentru controlul cubului LED 4x4x4.
 * Gestionez cele 4 straturi (catozi comuni) prin tranzistoare NPN 2N2222
 * si cele 16 coloane (anozi comuni) prin doua registre 74HC595 in daisy-chain.
 *
 * Bufferul cube_state[] e scris din main (animatii, modul timer)
 * si citit din ISR-ul de multiplexare - declarat volatile.
 */

#ifndef LED_CUBE_H
#define LED_CUBE_H

#include <avr/io.h>
#include <stdint.h>

/* Pinii de control ai straturilor - baza tranzistoarelor NPN 2N2222
 * Stratul 0 = jos (L1), stratul 3 = sus (L4) */
#define LAYER0_PIN  PD2
#define LAYER1_PIN  PD3
#define LAYER2_PIN  PD4
#define LAYER3_PIN  PD5

#define NUM_LAYERS   4
#define NUM_COLUMNS  16

/* Bufferul cubului - un uint16_t per strat, fiecare bit = o coloana
 * cube_state[0] = stratul de jos, cube_state[3] = stratul de sus
 * bit 0 = coloana 1 ... bit 15 = coloana 16
 * Scris din animatii/main, citit din ISR - declarat volatile */
extern volatile uint16_t cube_state[NUM_LAYERS];

/* Initializeaza pinii straturilor ca iesiri si stinge tot cubul */
void led_cube_init(void);

/* Afiseaza un singur strat - apelata din ISR la fiecare 2ms
 * Stinge toate straturile, trimite datele coloanelor, aprinde stratul curent
 * @param layer - indicele stratului (0-3) */
void led_cube_display_layer(uint8_t layer);

/* Stinge toate straturile (toti pinii de strat pe LOW) */
void led_cube_clear_layers(void);

/* Aprinde sau stinge un singur LED
 * @param layer  - stratul (0-3)
 * @param column - coloana (0-15)
 * @param state  - 1 = aprins, 0 = stins */
void led_cube_set_led(uint8_t layer, uint8_t column, uint8_t state);

/* Seteaza un strat intreg cu un pattern pe 16 biti
 * @param layer   - stratul (0-3)
 * @param pattern - masca coloanelor */
void led_cube_set_layer(uint8_t layer, uint16_t pattern);

/* Aprinde sau stinge o coloana pe toate cele 4 straturi simultan
 * @param column - coloana (0-15)
 * @param state  - 1 = aprinsa, 0 = stinsa */
void led_cube_set_column(uint8_t column, uint8_t state);

/* Stinge toate LED-urile (cube_state[] = 0 pe toate straturile) */
void led_cube_clear(void);

/* Aprinde toate LED-urile (cube_state[] = 0xFFFF pe toate straturile) */
void led_cube_fill(void);

#endif /* LED_CUBE_H */
