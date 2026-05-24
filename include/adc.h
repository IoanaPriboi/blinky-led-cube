/*
 * adc.h - Blinky LED Cube
 *
 * Modul pentru citirea potentiometrului prin ADC
 * Folosesc ADC-ul intern al ATmega328P pentru a citi pozitia
 * potentiometrului si a controla viteza animatiilor
 *
 * Functionalitate din laborator: Lab 4 - ADC
 */

#ifndef ADC_H
#define ADC_H

#include <avr/io.h>
#include <stdint.h>

/* Potentiometrul e conectat la pinul A0 = canalul ADC0 */
#define POT_CHANNEL  0

/* Limitele vitezei animatiei (ms/frame)
 * Am ales 50ms ca limita inferioara (animatie foarte rapida)
 * si 500ms ca limita superioara (animatie lenta, vizibila) */
#define ANIM_SPEED_MIN   50
#define ANIM_SPEED_MAX   500

/* Nu schimb viteza decat daca diferenta fata de ultima
 * valoare e mai mare de 10ms - evit tremuratul (jitter) */
#define SPEED_CHANGE_THRESHOLD  10

/* Initializeaza ADC-ul cu referinta AVcc si prescaler 128 */
void adc_init(void);

/* Citeste valoarea ADC de pe un canal (0-7), returneaza 0-1023 */
uint16_t adc_read(uint8_t channel);

/* Citeste potentiometrul si returneaza viteza animatiei in ms
 * Aplica un prag de schimbare ca sa nu tremure viteza */
uint16_t adc_read_speed(void);

#endif
