/*
 * lcd.c - Blinky LED Cube
 *
 * Implementarea controlului LCD 1602 prin I2C si adaptorul PCF8574.
 * Trimit datele in mod 4-bit - doi nibble per byte, cu puls pe EN
 * dupa fiecare nibble, exact ca in Lab 5 - lcd.c.
 * Diferenta fata de lab: nibble-urile ajung la LCD printr-un byte I2C
 * catre PCF8574, nu prin GPIO direct.
 *
 * Functionalitate din laborator: Lab 5 - LCD (lcd.c: mod 4-bit,
 *                                nibble transfer, puls EN, comenzi HD44780)
 */

#include "lcd.h"
#include "i2c.h"
#include <util/delay.h>

/* ------------------------------------------------------------------ */
/* Nivel 1 - scriere directa la PCF8574                                */
/* ------------------------------------------------------------------ */

/* Trimite un byte catre PCF8574 prin I2C
 * Fiecare bit din byte corespunde unui pin fizic al PCF8574:
 * P0=RS, P1=RW, P2=EN, P3=BL, P4=D4, P5=D5, P6=D6, P7=D7 */
static void pcf_write(uint8_t data)
{
    i2c_start();
    i2c_write(LCD_I2C_ADDR << 1);
    i2c_write(data);
    i2c_stop();
}

/* ------------------------------------------------------------------ */
/* Nivel 2 - trimitere nibble si byte catre LCD                        */
/* ------------------------------------------------------------------ */

/* Trimite un nibble (4 biti superiori) catre LCD cu puls pe EN
 * mode = 0 pentru comanda, LCD_RS pentru date
 * LCD-ul captureaza datele pe frontul descrescator al EN */
static void lcd_send_nibble(uint8_t nibble, uint8_t mode)
{
    uint8_t data = (nibble & 0xF0) | mode | LCD_BL;
    pcf_write(data | LCD_EN);   /* EN = 1 */
    _delay_us(1);
    pcf_write(data);            /* EN = 0 - LCD captureaza nibble-ul */
    _delay_us(50);
}

/* Trimite un byte complet ca doi nibble - MSB primul, LSB al doilea */
static void lcd_send_byte(uint8_t byte, uint8_t mode)
{
    lcd_send_nibble(byte & 0xF0, mode);
    lcd_send_nibble((byte << 4) & 0xF0, mode);
}

/* ------------------------------------------------------------------ */
/* Nivel 3 - API public                                                 */
/* ------------------------------------------------------------------ */

void lcd_init(void)
{
    _delay_ms(50);

    /* Secventa de initializare 4-bit din datasheet HD44780
     * Trimit 0x30 de trei ori ca sa resetez LCD-ul din orice stare,
     * apoi trec la 4-bit cu 0x20 */
    lcd_send_nibble(0x30, 0); _delay_ms(5);
    lcd_send_nibble(0x30, 0); _delay_us(150);
    lcd_send_nibble(0x30, 0); _delay_us(150);
    lcd_send_nibble(0x20, 0); _delay_us(150);

    /* Configurare: 4-bit, 2 linii, font 5x8 */
    lcd_send_byte(LCD_CMD_FUNCTION,   0);
    /* Display on, cursor off */
    lcd_send_byte(LCD_CMD_DISPLAY_ON, 0);
    /* Sterge display */
    lcd_send_byte(LCD_CMD_CLEAR,      0); _delay_ms(2);
    /* Auto-increment cursor */
    lcd_send_byte(LCD_CMD_ENTRY_MODE, 0);
}

void lcd_clear(void)
{
    lcd_send_byte(LCD_CMD_CLEAR, 0);
    _delay_ms(2);
}

void lcd_set_cursor(uint8_t row, uint8_t col)
{
    uint8_t addr = (row == 0) ? (LCD_ROW0 + col) : (LCD_ROW1 + col);
    lcd_send_byte(0x80 | addr, 0);
}

void lcd_write_char(char c)
{
    lcd_send_byte((uint8_t)c, LCD_RS);
}

void lcd_write_string(const char *str)
{
    while (*str) lcd_write_char(*str++);
}

void lcd_print_at(uint8_t row, uint8_t col, const char *str)
{
    lcd_set_cursor(row, col);
    lcd_write_string(str);
}

void lcd_print_line(uint8_t row, const char *str)
{
    lcd_set_cursor(row, 0);

    /* Scriu string-ul si completez cu spatii pana la 16 caractere
     * Evit lcd_clear() care face flash vizibil pe display */
    uint8_t i = 0;
    while (*str && i < LCD_COLS) { lcd_write_char(*str++); i++; }
    while (i < LCD_COLS)         { lcd_write_char(' ');    i++; }
}
