/*
 * lcd.h - Blinky LED Cube
 *
 * Modul pentru controlul LCD-ului 1602 prin adaptorul I2C PCF8574.
 * Principiul e acelasi ca in Lab 5 - lcd.c: interfatare in 4-bit,
 * trimit doi nibble per byte, puls pe EN pentru capturarea datelor.
 * Diferenta fata de lab: in loc de pini GPIO directi, trimit nibble-urile
 * ca bytes I2C catre PCF8574, care le expune ca pini fizici la LCD.
 *
 * Functionalitate din laborator: Lab 5 - LCD (lcd.c: mod 4-bit, nibble
 *                                transfer, comenzi HD44780, LCD_writeInstr)
 */

#ifndef LCD_H
#define LCD_H

#include <avr/io.h>
#include <stdint.h>

/* Adresa I2C a modulului PCF8574 - implicit 0x27 */
#define LCD_I2C_ADDR  0x27

/* Dimensiuni LCD 1602 */
#define LCD_COLS  16
#define LCD_ROWS   2

/* Adresele DDRAM ale celor doua linii - din datasheet HD44780 */
#define LCD_ROW0  0x00
#define LCD_ROW1  0x40

/* Maparea pinilor PCF8574 la semnalele LCD
 * P0=RS, P1=RW, P2=EN, P3=BL, P4=D4, P5=D5, P6=D6, P7=D7 */
#define LCD_RS  (1 << 0)   /* Register Select: 0=comanda, 1=date */
#define LCD_RW  (1 << 1)   /* Read/Write: tinut pe 0 (doar scriem) */
#define LCD_EN  (1 << 2)   /* Enable: puls pentru capturarea datelor */
#define LCD_BL  (1 << 3)   /* Backlight: 1=aprins */

/* Comenzi HD44780 - aceleasi ca in Lab 5 - lcd.h */
#define LCD_CMD_CLEAR       0x01   /* sterge display si pune cursorul la inceput */
#define LCD_CMD_HOME        0x02   /* cursor la pozitia 0 */
#define LCD_CMD_ENTRY_MODE  0x06   /* auto-increment, fara shift display */
#define LCD_CMD_DISPLAY_ON  0x0C   /* display on, cursor off, blink off */
#define LCD_CMD_FUNCTION    0x28   /* 4-bit, 2 linii, font 5x8 */

/* Initializeaza LCD-ul in mod 4-bit
 * Trebuie apelata dupa i2c_init() */
void lcd_init(void);

/* Sterge display-ul si pune cursorul la pozitia 0,0 */
void lcd_clear(void);

/* Pozitioneaza cursorul la linia si coloana specificate
 * @param row - 0 sau 1
 * @param col - 0 pana la 15 */
void lcd_set_cursor(uint8_t row, uint8_t col);

/* Afiseaza un caracter la pozitia curenta */
void lcd_write_char(char c);

/* Afiseaza un string terminat cu '\0' la pozitia curenta */
void lcd_write_string(const char *str);

/* Afiseaza un string la pozitia specificata
 * @param row - 0 sau 1
 * @param col - 0 pana la 15
 * @param str - string-ul de afisat */
void lcd_print_at(uint8_t row, uint8_t col, const char *str);

/* Afiseaza un string pe toata linia specificata
 * Completeaza cu spatii pana la 16 caractere - sterge ce era inainte
 * fara sa apeleze lcd_clear() (care e lent si face flash)
 * @param row - 0 sau 1
 * @param str - string-ul de afisat */
void lcd_print_line(uint8_t row, const char *str);

#endif /* LCD_H */
