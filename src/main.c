/*
 * main.c - Blinky LED Cube
 * Arduino Nano V3 / ATmega328P
 *
 * Punctul de intrare al proiectului. Initializez toate perifericele,
 * activez intreruperile si intru in bucla principala non-blocanta.
 *
 * Aplicatia are doua moduri, comutate cu BTN_MODE:
 *   MODE_ANIMATION - selectez animatii cu BTN_UP/BTN_DOWN,
 *                    viteza controlata din potentiometru
 *   MODE_TIMER     - countdown configurat cu BTN_UP/BTN_DOWN (+/-10s),
 *                    pornit/oprit cu BTN_START_STOP, alerta buzzer la final
 *
 * Bucla principala urmeaza patternul non-blocant din Lab 2:
 * citesc timestamp-ul curent si execut fiecare task doar daca
 * a trecut intervalul sau (ADC la 50ms, LCD la 250ms, animatii la speed_ms).
 *
 * Functionalitate din laborator: Lab 2 - pattern non-blocant (uptime_ms,
 *                                TICKS_PASSED), Lab 3 - timere, Lab 4 - ADC,
 *                                Lab 5 - LCD, Lab 6 - I2C
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdlib.h>

#include "timer.h"
#include "spi.h"
#include "led_cube.h"
#include "adc.h"
#include "buttons.h"
#include "buzzer.h"
#include "i2c.h"
#include "lcd.h"
#include "animations.h"

/* ------------------------------------------------------------------ */
/* Modurile aplicatiei                                                  */
/* ------------------------------------------------------------------ */

typedef enum {
    MODE_ANIMATION = 0,
    MODE_TIMER,
    MODE_COUNT
} app_mode_t;

/* ------------------------------------------------------------------ */
/* Starea globala                                                       */
/* ------------------------------------------------------------------ */

static app_mode_t current_mode   = MODE_ANIMATION;
static uint16_t   timer_set_secs = 60;    /* durata configurata (secunde) */
static uint16_t   cached_speed   = 200;   /* viteza animatie in ms/frame  */
static uint8_t    alert_active   = 0;     /* 1 cand timerul a expirat     */

/* Timestamp-uri pentru taskurile periodice non-blocante */
static uint32_t last_adc_read   = 0;
static uint32_t last_lcd_update = 0;
static uint32_t last_alert_upd  = 0;

#define ADC_INTERVAL_MS   50    /* citesc potentiometrul la fiecare 50ms  */
#define LCD_INTERVAL_MS  250    /* actualizez LCD-ul la fiecare 250ms     */
#define ALERT_INTERVAL_MS 80    /* animatie alerta la fiecare 80ms        */

/* ------------------------------------------------------------------ */
/* Helpers pentru LCD - evit sprintf ca sa nu trag printf in flash     */
/* ------------------------------------------------------------------ */

/* Scrie exact `digits` cifre in buf, umplute cu '0' la stanga */
static void num_to_str(uint16_t val, char *buf, uint8_t digits)
{
    buf[digits] = '\0';
    for (int8_t i = (int8_t)(digits - 1); i >= 0; i--) {
        buf[i] = '0' + (char)(val % 10);
        val /= 10;
    }
}

/* Copiaza src in dst, returneaza numarul de caractere scrise */
static uint8_t strcopy(char *dst, const char *src, uint8_t max)
{
    uint8_t i = 0;
    while (src[i] && i < max) { dst[i] = src[i]; i++; }
    return i;
}

/* ------------------------------------------------------------------ */
/* Actualizare LCD - apelata periodic, non-blocant                     */
/* ------------------------------------------------------------------ */

static void lcd_update(uint32_t now)
{
    if (!TICKS_PASSED(last_lcd_update, LCD_INTERVAL_MS)) return;
    last_lcd_update = now;

    char line0[LCD_COLS + 1];
    char line1[LCD_COLS + 1];

    /* Initializez liniile cu spatii */
    for (uint8_t i = 0; i < LCD_COLS; i++) { line0[i] = ' '; line1[i] = ' '; }
    line0[LCD_COLS] = '\0';
    line1[LCD_COLS] = '\0';

    if (current_mode == MODE_ANIMATION) {
        /* Linia 0: "Anim: <nume>" */
        uint8_t p = strcopy(line0, "Anim: ", LCD_COLS);
        strcopy(line0 + p, animations_get_name(), LCD_COLS - p);

        /* Linia 1: "Speed: <val>ms" */
        p = strcopy(line1, "Speed: ", LCD_COLS);
        char num[4];
        num_to_str(cached_speed, num, 3);
        p += strcopy(line1 + p, num, LCD_COLS - p);
        strcopy(line1 + p, "ms", LCD_COLS - p);

    } else {
        /* Linia 0: "Timer: MM:SS" */
        uint16_t secs = timer_running ? timer_seconds_left : timer_set_secs;
        char num[3];
        uint8_t p = strcopy(line0, "Timer: ", LCD_COLS);
        num_to_str(secs / 60, num, 2); p += strcopy(line0 + p, num, LCD_COLS - p);
        line0[p++] = ':';
        num_to_str(secs % 60, num, 2); strcopy(line0 + p, num, LCD_COLS - p);

        /* Linia 1: starea curenta */
        if (alert_active)       strcopy(line1, "  TIME'S UP!",  LCD_COLS);
        else if (timer_running) strcopy(line1, "  [Running]",   LCD_COLS);
        else                    strcopy(line1, "  UP/DN: Set",  LCD_COLS);
    }

    lcd_print_line(0, line0);
    lcd_print_line(1, line1);
}

/* ------------------------------------------------------------------ */
/* Logica modului Animatie                                              */
/* ------------------------------------------------------------------ */

static void mode_animation_handle(uint32_t now, uint16_t frame[NUM_LAYERS])
{
    /* BTN_UP - animatia urmatoare */
    if (btn_up_flag) {
        btn_up_flag = 0;
        animations_next();
        last_lcd_update = 0;
    }

    /* BTN_DOWN - animatia anterioara */
    if (btn_down_flag) {
        btn_down_flag = 0;
        animation_id_t cur = animations_get_current();
        animations_set(cur == 0 ? (animation_id_t)(ANIM_COUNT - 1)
                                : (animation_id_t)(cur - 1));
        last_lcd_update = 0;
    }

    /* Actualizez animatia - non-blocant, intern verifica period_ms */
    animations_update(now, cached_speed, frame);

    /* Copiez frame-ul in cube_state - ISR-ul il citeste pentru multiplexare */
    for (uint8_t i = 0; i < NUM_LAYERS; i++) {
        cube_state[i] = frame[i];
    }
}

/* ------------------------------------------------------------------ */
/* Logica modului Timer                                                 */
/* ------------------------------------------------------------------ */

static void mode_timer_handle(uint32_t now)
{
    /* BTN_START_STOP - porneste/opreste timerul sau inchide alerta */
    if (btn_start_stop_flag) {
        btn_start_stop_flag = 0;

        if (alert_active) {
            /* Opresc alerta */
            alert_active  = 0;
            timer_expired = 0;
            buzzer_stop();
            led_cube_clear();
        } else if (timer_running) {
            timer_stop();
            led_cube_clear();
        } else {
            timer_expired = 0;
            led_cube_fill();
            timer_start(timer_set_secs);
        }
        last_lcd_update = 0;
    }

    /* BTN_UP/DOWN - ajustez durata (+/-10s) doar cand timerul e oprit */
    if (btn_up_flag) {
        btn_up_flag = 0;
        if (!timer_running && !alert_active && timer_set_secs < 5990) {
            timer_set_secs += 10;
            last_lcd_update = 0;
        }
    }

    if (btn_down_flag) {
        btn_down_flag = 0;
        if (!timer_running && !alert_active && timer_set_secs > 10) {
            timer_set_secs -= 10;
            last_lcd_update = 0;
        }
    }

    /* Detectez expirarea - pornesc alerta (buzzer continuu + LED-uri haotice) */
    if (timer_expired && !alert_active) {
        alert_active    = 1;
        last_lcd_update = 0;
        buzzer_start(BUZZER_FREQ_ALERT);
    }

    /* ---- Alerta activa: LED-uri aleatoare la fiecare 80ms ---- */
    if (alert_active) {
        if (TICKS_PASSED(last_alert_upd, ALERT_INTERVAL_MS)) {
            last_alert_upd = now;
            for (uint8_t z = 0; z < NUM_LAYERS; z++) {
                cube_state[z] = (uint16_t)rand();
            }
        }
        return;
    }

    /* ---- Countdown activ: LED-urile se sting progresiv ---- */
    if (timer_running && timer_set_secs > 0) {
        /* Calculez numarul de LED-uri proportional cu timpul ramas
         * La inceput: 64 LED-uri aprinse, la final: 0 */
        uint8_t leds_on = (uint8_t)(
            ((uint32_t)timer_seconds_left * 64 + timer_set_secs - 1)
            / timer_set_secs);
        if (leds_on > 64) leds_on = 64;

        /* Umplu straturile de jos in sus cu LED-urile ramase */
        for (uint8_t z = 0; z < NUM_LAYERS; z++) {
            if (leds_on >= 16) {
                cube_state[z] = 0xFFFF;
                leds_on -= 16;
            } else if (leds_on > 0) {
                cube_state[z] = (uint16_t)((1u << leds_on) - 1);
                leds_on = 0;
            } else {
                cube_state[z] = 0x0000;
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* main                                                               */
/* ------------------------------------------------------------------ */

int main(void)
{
    /* Initializez perifericele in ordinea dependentelor:
     * SPI si cubul inainte de timer (ISR-ul apeleaza display_layer) */
    spi_init();
    led_cube_init();
    timer_init();
    adc_init();
    buttons_init();
    i2c_init();
    lcd_init();
    buzzer_init();

    /* Seed-uiesc rand() din ADC inainte de animations_init()
     * Pozitia potentiometrului la pornire da varietate animatiei Rain */
    animations_init(adc_read(POT_CHANNEL));

    /* Activez intreruperile globale - de acum ISR-ul ruleaza */
    sei();

    /* Beep scurt de confirmare pornire */
    buzzer_beep(BUZZER_FREQ_HIGH, 100);

    /* Mesaj de bun venit pe LCD */
    lcd_print_line(0, "Blinky LED Cube");
    lcd_print_line(1, "  Starting...  ");

    /* Astept 1.5s sa se vada mesajul - blocant doar la pornire */
    uint32_t t0 = timer_get_ticks();
    while (!TICKS_PASSED(t0, 1500));

    lcd_clear();

    /* Buffer local pentru frame-ul animatiei */
    uint16_t cube_frame[NUM_LAYERS] = {0, 0, 0, 0};

    /* ---- Bucla principala non-blocanta ---- */
    while (1) {
        uint32_t now = timer_get_ticks();

        /* Citesc potentiometrul la fiecare ADC_INTERVAL_MS */
        if (TICKS_PASSED(last_adc_read, ADC_INTERVAL_MS)) {
            last_adc_read = now;
            cached_speed  = adc_read_speed();
        }

        /* BTN_MODE - comut intre moduri */
        if (btn_mode_flag) {
            btn_mode_flag = 0;
            current_mode  = (app_mode_t)((current_mode + 1) % MODE_COUNT);

            led_cube_clear();
            animations_clear(cube_frame);

            /* Opresc timer-ul si alerta daca parasesc modul Timer */
            if (current_mode != MODE_TIMER && alert_active) {
                alert_active = 0;
                buzzer_stop();
            }
            if (current_mode != MODE_TIMER) {
                timer_stop();
                timer_expired = 0;
            }

            /* Opresc buzzerul la parasirea modului Animatie */
            if (current_mode == MODE_TIMER) {
                buzzer_stop();
            }

            /* Reinitializez animatia la intrarea in modul Animatie */
            if (current_mode == MODE_ANIMATION) {
                animations_init(adc_read(POT_CHANNEL));
            }

            last_lcd_update = 0;
        }

        /* Execut logica modului curent */
        switch (current_mode) {
            case MODE_ANIMATION:
                mode_animation_handle(now, cube_frame);
                break;
            case MODE_TIMER:
                mode_timer_handle(now);
                break;
            default:
                break;
        }

        /* Actualizez LCD-ul periodic */
        lcd_update(now);
    }

    return 0;
}
