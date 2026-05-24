/*
 * i2c.c - Blinky LED Cube
 *
 * Comunicatie I2C prin perifericul TWI al ATmega328P.
 * Structura e aceeasi ca in Lab 6 - twi.c: init, start, write, read, stop.
 *
 * Mecanism: scriu in TWCR cu TWINT=1 ca sa sterg flag-ul si sa pornesc
 * operatia, apoi astept ca hardware-ul sa seteze TWINT=1 din nou.
 *
 * Functionalitate din laborator: Lab 6 - TWI (twi.c: twi_init, twi_start,
 *                                twi_write, twi_read_ack, twi_stop, TWBR_VAL)
 */

#include "i2c.h"

void i2c_init(void)
{
    /* Resetez registrul de control */
    TWCR = 0;

    /* Setez TWBR pentru frecventa SCL de 100kHz
     * Referinta: Lab 6 - twi.h, TWBR_VAL */
    TWBR = (uint8_t)TWBR_VAL;

    /* Prescaler 1 - sterg bitii TWPS1:0 din TWSR */
    TWSR = 0;
}

void i2c_start(void)
{
    /* TWINT=1 sterge flag-ul, TWSTA=1 genereaza START, TWEN=1 activeaza TWI */
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

void i2c_stop(void)
{
    /* TWSTO=1 genereaza STOP - nu astept TWINT, STOP nu genereaza intrerupere */
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

void i2c_write(uint8_t data)
{
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

void i2c_read_ack(uint8_t *data)
{
    /* TWEA=1 trimite ACK - mai urmeaza bytes dupa acesta */
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    while (!(TWCR & (1 << TWINT)));
    *data = TWDR;
}

void i2c_read_nack(uint8_t *data)
{
    /* TWEA=0 trimite NACK - acesta e ultimul byte */
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    *data = TWDR;
}
