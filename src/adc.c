/*
 * adc.c - Blinky LED Cube
 *
 * Implementarea modulului ADC.
 * Citesc potentiometrul conectat la A0 si transform valoarea
 * intr-o viteza pentru animatii (50-500ms per frame).
 *
 * Maparea ADC -> viteza e similara cu remap_interval din Lab 4,
 * doar ca folosesc aritmetica pe intregi (fara float).
 *
 * Functionalitate din laborator: Lab 4 - ADC
 */

#include "adc.h"

/* Pastrez ultima viteza returnata pentru a aplica pragul de schimbare */
static uint16_t last_speed = 200;

void adc_init(void)
{
    /* Aleg referinta AVcc (5V) - setez bitul REFS0 in ADMUX.
     * Potentiometrul e alimentat la 5V, deci am nevoie de
     * aceeasi referinta pentru a obtine toata gama 0-1023 */
    ADMUX = (1 << REFS0);

    /* Setez prescaler-ul la 128 - bitii ADPS2:0 pe 1.
     * 16MHz / 128 = 125kHz - in intervalul recomandat 50-200kHz
     * din datasheet. La frecvente mai mari pierd precizie */
    ADCSRA = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    /* Activez ADC-ul - bitul ADEN */
    ADCSRA |= (1 << ADEN);
}

uint16_t adc_read(uint8_t channel)
{
    /* Fortez canalul sa fie intre 0 si 7 (ADC0-ADC7) */
    channel &= 0x07;

    /* Sterg vechiul canal din ADMUX (ultimii 4 biti) */
    ADMUX &= 0xF0;

    /* Selectez noul canal */
    ADMUX |= channel;

    /* Pornesc conversia - setez bitul ADSC */
    ADCSRA |= (1 << ADSC);

    /* Astept sa se termine conversia.
     * ADSC devine 0 automat cand conversia e gata */
    while (ADCSRA & (1 << ADSC));

    /* Returnez valoarea - registrul ADC combina ADCL si ADCH */
    return ADC;
}

uint16_t adc_read_speed(void)
{
    uint16_t adc_val = adc_read(POT_CHANNEL);

    /* Mapez valoarea ADC (0-1023) la intervalul de viteza (50-500ms)
     * Inversez sensul ca sa fie intuitiv:
     * potentiometru la maxim (1023) => viteza minima (50ms = rapid)
     * potentiometru la minim (0)    => viteza maxima (500ms = lent) */
    uint16_t new_speed = ANIM_SPEED_MIN +
        (uint16_t)((uint32_t)(ANIM_SPEED_MAX - ANIM_SPEED_MIN) *
                   (1023 - adc_val) / 1023);

    /* Actualizez viteza doar daca s-a schimbat cu mai mult de
     * SPEED_CHANGE_THRESHOLD ms - altfel pastrez valoarea veche
     * ca sa evit tremuratul cauzat de zgomotul ADC-ului */
    uint16_t diff;
    if (new_speed > last_speed) {
        diff = new_speed - last_speed;
    } else {
        diff = last_speed - new_speed;
    }

    if (diff >= SPEED_CHANGE_THRESHOLD) {
        last_speed = new_speed;
    }

    return last_speed;
}
