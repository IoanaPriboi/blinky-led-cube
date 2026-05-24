/*
 * spi.c - Blinky LED Cube
 *
 * Transfer serial catre doua 74HC595 in lant prin bit-bang.
 * SR2 primeste datele primul (coloanele 9-16) si le paseaza la SR1 (col 1-8).
 * La pulsul de LATCH ambele registre isi actualizeaza iesirile simultan.
 *
 * Functionalitate din laborator: Lab 5 - SPI (transfer bit cu bit,
 *                                MSB first, protocol ceas + date)
 */

#include "spi.h"

void spi_init(void)
{
    /* Setez pinii DATA, CLOCK si LATCH ca iesiri */
    DDRB |= (1 << SR_DATA_PIN) | (1 << SR_CLOCK_PIN) | (1 << SR_LATCH_PIN);

    /* Stare initiala: DATA si CLOCK pe LOW, LATCH pe HIGH */
    SR_DATA_LOW();
    SR_CLOCK_LOW();
    SR_LATCH_HIGH();
}

/* Trimite un singur byte prin bit-bang, MSB first
 * La fiecare bit: setez DATA, puls pe CLOCK (front crescator captureaza data)
 * Acelasi principiu ca SPI_exchange() din Lab 5, fara registrul hardware SPDR */
static void spi_shift_byte(uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++) {
        if (data & 0x80) {
            SR_DATA_HIGH();
        } else {
            SR_DATA_LOW();
        }

        /* Puls pe CLOCK - frontul crescator captureaza bitul in registru */
        SR_CLOCK_HIGH();
        SR_CLOCK_LOW();

        data <<= 1;
    }
}

void spi_send_columns(uint16_t columns)
{
    /* Cobor LATCH ca sa blochez iesirile pe durata transferului
     * Altfel iesirile s-ar actualiza la fiecare bit - glitch-uri vizibile */
    SR_LATCH_LOW();

    /* SR2 primeste primul (col 9-16), SR1 primeste al doilea (col 1-8) */
    spi_shift_byte((uint8_t)(columns >> 8));
    spi_shift_byte((uint8_t)(columns & 0xFF));

    /* Ridic LATCH - ambele registre isi actualizeaza iesirile simultan */
    SR_LATCH_HIGH();
}
