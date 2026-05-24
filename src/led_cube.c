/*
 * led_cube.c - Blinky LED Cube
 *
 * Implementarea controlului cubului LED 4x4x4.
 * Multiplexarea o fac prin activarea unui singur strat la un moment dat -
 * ISR-ul comuta intre cele 4 straturi la fiecare 2ms (125Hz total),
 * iar ochiul percepe toate LED-urile ca aprinse simultan.
 *
 * Coloanele le trimit prin doua 74HC595 in lant prin spi.c.
 * Straturile le controlez direct prin PORTD, prin tranzistoare NPN 2N2222:
 *   HIGH pe baza tranzistorului => tranzistorul conduce => stratul e activ.
 */

#include "led_cube.h"
#include "spi.h"

/* Bufferul cubului - modificat din animatii/main, citit din ISR */
volatile uint16_t cube_state[NUM_LAYERS] = {0, 0, 0, 0};

/* Mapez indexul stratului la pinul corespunzator din PORTD
 * Folosesc un array static ca sa pot accesa pinul prin index in loc
 * de un switch - mai curat si fara branch-uri in ISR */
static const uint8_t layer_pins[NUM_LAYERS] = {
    LAYER0_PIN,   /* strat 0 - jos  */
    LAYER1_PIN,   /* strat 1        */
    LAYER2_PIN,   /* strat 2        */
    LAYER3_PIN,   /* strat 3 - sus  */
};

void led_cube_init(void)
{
    /* Setez toti pinii de strat ca iesiri in DDRD */
    DDRD |= (1 << LAYER0_PIN) | (1 << LAYER1_PIN) |
            (1 << LAYER2_PIN) | (1 << LAYER3_PIN);

    /* Sterg toate straturile - tranzistoarele NPN sunt inchise la LOW pe baza */
    led_cube_clear_layers();
}

void led_cube_clear_layers(void)
{
    /* LOW pe toti pinii de strat - niciun tranzistor nu conduce */
    PORTD &= ~((1 << LAYER0_PIN) | (1 << LAYER1_PIN) |
               (1 << LAYER2_PIN) | (1 << LAYER3_PIN));
}

void led_cube_display_layer(uint8_t layer)
{
    /* Sting toate straturile inainte de a trimite datele noi
     * Fara asta as vedea ghosting pe durata transferului SPI */
    led_cube_clear_layers();

    /* Trimit pattern-ul coloanelor pentru stratul curent catre 74HC595 */
    spi_send_columns(cube_state[layer]);

    /* Aprind stratul curent - HIGH pe baza tranzistorului NPN */
    PORTD |= (1 << layer_pins[layer]);
}

void led_cube_set_led(uint8_t layer, uint8_t column, uint8_t state)
{
    if (layer >= NUM_LAYERS || column >= NUM_COLUMNS) return;

    if (state) {
        cube_state[layer] |=  (1u << column);
    } else {
        cube_state[layer] &= ~(1u << column);
    }
}

void led_cube_set_layer(uint8_t layer, uint16_t pattern)
{
    if (layer >= NUM_LAYERS) return;
    cube_state[layer] = pattern;
}

void led_cube_set_column(uint8_t column, uint8_t state)
{
    if (column >= NUM_COLUMNS) return;
    for (uint8_t z = 0; z < NUM_LAYERS; z++) {
        if (state) {
            cube_state[z] |=  (1u << column);
        } else {
            cube_state[z] &= ~(1u << column);
        }
    }
}

void led_cube_clear(void)
{
    for (uint8_t i = 0; i < NUM_LAYERS; i++) {
        cube_state[i] = 0x0000;
    }
}

void led_cube_fill(void)
{
    for (uint8_t i = 0; i < NUM_LAYERS; i++) {
        cube_state[i] = 0xFFFF;
    }
}
