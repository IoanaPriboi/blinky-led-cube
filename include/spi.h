/*
 * spi.h - Blinky LED Cube
 *
 * Modul pentru controlul celor doua registre de deplasare 74HC595
 * prin bit-bang SPI (nu folosesc perifericul hardware SPI).
 * Trimit 16 biti pentru cele 16 coloane ale cubului prin doua registre
 * conectate in lant (daisy-chain).
 *
 * Folosesc bit-bang in loc de SPI hardware deoarece SRCLK (ceasul
 * registrului de deplasare) e conectat la PB4, care e pinul MISO al
 * ATmega328P - nu la PB5 (SCK), pinul de ceas hardware SPI.
 *
 * Functionalitate din laborator: Lab 5 - SPI (principiul de transfer
 *                                bit cu bit, MSB first, puls de ceas)
 */

#ifndef SPI_H
#define SPI_H

#include <avr/io.h>
#include <stdint.h>

/* Pinii pentru controlul 74HC595
 * D11 / PB3 = SER   - date seriale (DATA IN)
 * D12 / PB4 = SRCLK - ceasul registrului de deplasare (SHIFT CLOCK)
 * D10 / PB2 = RCLK  - ceasul registrului de iesire (LATCH / STORAGE CLOCK) */
#define SR_DATA_PIN   PB3
#define SR_CLOCK_PIN  PB4
#define SR_LATCH_PIN  PB2

/* Macros pentru controlul direct al pinilor - evit apeluri de functii in ISR */
#define SR_DATA_HIGH()   (PORTB |=  (1 << SR_DATA_PIN))
#define SR_DATA_LOW()    (PORTB &= ~(1 << SR_DATA_PIN))
#define SR_CLOCK_HIGH()  (PORTB |=  (1 << SR_CLOCK_PIN))
#define SR_CLOCK_LOW()   (PORTB &= ~(1 << SR_CLOCK_PIN))
#define SR_LATCH_HIGH()  (PORTB |=  (1 << SR_LATCH_PIN))
#define SR_LATCH_LOW()   (PORTB &= ~(1 << SR_LATCH_PIN))

/* Seteaza pinii DATA, CLOCK si LATCH ca iesiri
 * si ii aduce intr-o stare initiala cunoscuta */
void spi_init(void);

/* Trimite valorile pentru cele 16 coloane catre cele doua 74HC595
 * Bit 0 = coloana 1 ... bit 15 = coloana 16
 * @param columns - masca pe 16 biti a coloanelor active */
void spi_send_columns(uint16_t columns);

#endif /* SPI_H */
