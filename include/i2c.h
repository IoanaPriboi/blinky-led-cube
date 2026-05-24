/*
 * i2c.h - Blinky LED Cube
 *
 * Modul pentru comunicatia I2C (TWI) in mod master.
 * Folosesc perifericul TWI hardware al ATmega328P pentru a comunica
 * cu modulul LCD 1602 I2C (adaptor PCF8574).
 * API-ul e identic ca structura cu cel din Lab 6 - twi.h:
 * start, write, read_ack, read_nack, stop.
 *
 * Functionalitate din laborator: Lab 6 - TWI (twi.c: twi_init, twi_start,
 *                                twi_write, twi_read_ack, twi_stop, TWBR_VAL)
 */

#ifndef I2C_H
#define I2C_H

#include <avr/io.h>
#include <stdint.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/* Frecventa SCL dorita - 100kHz (mod standard I2C) */
#define F_SCL  100000UL

/* Valoarea registrului TWBR pentru frecventa SCL dorita
 * Formula din datasheet: SCL = F_CPU / (16 + 2 * TWBR * prescaler)
 * Cu prescaler 1 (TWSR = 0): TWBR = (F_CPU / F_SCL - 16) / 2
 * La 16MHz si 100kHz: TWBR = (160 - 16) / 2 = 72
 * Referinta: Lab 6 - twi.h, TWBR_VAL */
#define TWBR_VAL  ((F_CPU / F_SCL - 16) / 2)

/* Initializeaza TWI la 100kHz cu prescaler 1
 * Trebuie apelata inainte de orice operatie I2C */
void i2c_init(void);

/* Trimite conditia START pe magistrala - incepe o tranzactie */
void i2c_start(void);

/* Trimite conditia STOP pe magistrala - incheie tranzactia */
void i2c_stop(void);

/* Scrie un byte pe magistrala I2C (adresa slave sau date)
 * @param data - byte-ul de trimis */
void i2c_write(uint8_t data);

/* Citeste un byte si trimite ACK - mai urmeaza bytes in aceasta tranzactie
 * @param data - pointer unde se salveaza byte-ul citit */
void i2c_read_ack(uint8_t *data);

/* Citeste un byte si trimite NACK - ultimul byte din tranzactie
 * @param data - pointer unde se salveaza byte-ul citit */
void i2c_read_nack(uint8_t *data);

#endif /* I2C_H */
