/*
 * animations.c - Blinky LED Cube
 *
 * Implementarea animatiilor non-blocante pentru cubul LED 4x4x4.
 * Fiecare animatie are o functie interna de update care scrie in frame[],
 * iar animations_update() le apeleaza doar daca a trecut period_ms -
 * acelasi pattern non-blocant ca in Lab 2 (ex1.c: uptime_ms + last_msg).
 *
 * Animatii implementate:
 *   Rain       - picaturi care cad de sus in jos pe coloane aleatoare (rand())
 *   Plane Scan - un strat complet se aprinde pe rand, de jos in sus
 *   Expand     - LED-urile se extind din centrul fiecarui strat spre exterior
 *   Spiral     - coloanele se aprind in spirala, din interior spre exterior
 *   Firework   - racheta urca, explodeaza si se stinge cu scantei
 *
 * Functionalitate din laborator: Lab 2 - pattern non-blocant cu timestamp
 *                                (now - last >= interval), uptime_ms()
 */

#include "animations.h"
#include "buzzer.h"
#include <stdlib.h>

/* ------------------------------------------------------------------ */
/* Stare interna a modulului                                           */
/* ------------------------------------------------------------------ */

static animation_id_t current_anim = ANIM_RAIN;
static uint8_t        anim_step    = 0;
static uint32_t       last_update  = 0;

/* ------------------------------------------------------------------ */
/* Animatia Rain                                                        */
/* ------------------------------------------------------------------ */

#define RAIN_DROPS  4

typedef struct {
    uint8_t col;   /* coloana activa (0-15) */
    int8_t  z;     /* stratul curent (3=sus ... 0=jos, -1=inactiv) */
} drop_t;

static drop_t drops[RAIN_DROPS];

static void rain_init(void)
{
    for (uint8_t i = 0; i < RAIN_DROPS; i++) {
        drops[i].col = (uint8_t)(rand() % CUBE_COLUMNS);
        /* Decalez picaturile ca sa nu inceapa toate de sus in acelasi moment */
        drops[i].z = (int8_t)(CUBE_LAYERS - 1 - i);
    }
}

static void rain_update(uint16_t frame[CUBE_LAYERS])
{
    for (uint8_t i = 0; i < RAIN_DROPS; i++) {
        /* Aprind picatura la pozitia curenta */
        if (drops[i].z >= 0) {
            frame[drops[i].z] |= (1u << drops[i].col);
        }

        /* Cobor picatura cu un strat */
        drops[i].z--;

        /* Daca a iesit din cub, o resetez pe o coloana noua la nivelul de sus */
        if (drops[i].z < -1) {
            drops[i].col = (uint8_t)(rand() % CUBE_COLUMNS);
            drops[i].z   = CUBE_LAYERS - 1;
        }
    }
}

/* ------------------------------------------------------------------ */
/* Animatia Plane Scan                                                  */
/* ------------------------------------------------------------------ */

static void plane_scan_update(uint16_t frame[CUBE_LAYERS])
{
    /* Aprind toate cele 16 coloane pe stratul curent */
    frame[anim_step % CUBE_LAYERS] = 0xFFFF;
    anim_step = (anim_step + 1) % CUBE_LAYERS;
}

/* ------------------------------------------------------------------ */
/* Animatia Expand / Contract - extindere 3D din centrul cubului       */
/*                                                                      */
/* Expandez simultan pe axa coloanelor SI pe axa straturilor.           */
/* Incep cu centrul 2x2 pe straturile interioare (1 si 2),             */
/* apoi extind spre exterior pana la tot cubul, apoi contract inapoi.  */
/*                                                                      */
/* Fiecare rand din tabel = [L0, L1, L2, L3] pentru un pas             */
/*                                                                      */
/*           L0       L1       L2       L3                              */
/* Step 0:  EMPTY    EMPTY    EMPTY    EMPTY   (0 LED-uri)              */
/* Step 1:  EMPTY   CENTER   CENTER    EMPTY   (8 LED-uri)              */
/* Step 2:  CENTER   MID      MID     CENTER   (24 LED-uri)             */
/* Step 3:  MID      FULL     FULL    MID      (48 LED-uri)             */
/* Step 4:  FULL     FULL     FULL    FULL     (64 LED-uri)             */
/* Step 5-7: inapoi (simetric)                                        */
/* ------------------------------------------------------------------ */

#define MASK_EMPTY   0x0000u
#define MASK_CENTER  0x0660u   /* bits 5,6,9,10 - centru 2x2 */
#define MASK_MID     0x0FF0u   /* bits 4-11 - cele doua randuri centrale */
#define MASK_FULL    0xFFFFu

#define EXPAND_STEPS  6

/* Tabel [pas][strat] - defineste masca fiecarui strat la fiecare pas */
static const uint16_t expand_frames[EXPAND_STEPS][CUBE_LAYERS] = {
    { MASK_EMPTY,  MASK_EMPTY,  MASK_EMPTY,  MASK_EMPTY  }, /* 0: nimic   */
    { MASK_EMPTY,  MASK_CENTER, MASK_CENTER, MASK_EMPTY  }, /* 1: expand  */
    { MASK_EMPTY,  MASK_MID,    MASK_MID,    MASK_EMPTY  }, /* 2: expand  */
    { MASK_FULL,   MASK_FULL,   MASK_FULL,   MASK_FULL   }, /* 3: plin    */
    { MASK_EMPTY,  MASK_MID,    MASK_MID,    MASK_EMPTY  }, /* 4: expand  */
    { MASK_EMPTY,  MASK_CENTER, MASK_CENTER, MASK_EMPTY  }, /* 5: contract*/
};


static void expand_update(uint16_t frame[CUBE_LAYERS])
{
    /* Copiez masca corespunzatoare pasului curent pentru fiecare strat */
    for (uint8_t z = 0; z < CUBE_LAYERS; z++) {
        frame[z] = expand_frames[anim_step][z];
    }

    anim_step = (anim_step + 1) % EXPAND_STEPS;
}

/* ------------------------------------------------------------------ */
/* Animatia Spiral                                                      */
/* Aprind coloanele in ordinea spiralei, din interior spre exterior,   */
/* pe toate 4 straturile simultan. Creste de la 1 la 16 coloane,       */
/* apoi scade inapoi la 0.                                              */
/*                                                                      */
/* Grid 4x4:  [ 0][ 1][ 2][ 3]                                         */
/*            [ 4][ 5][ 6][ 7]                                         */
/*            [ 8][ 9][10][11]                                         */
/*            [12][13][14][15]                                         */
/*                                                                      */
/* Ordinea: 9->10->6->5->4->8->12->13->14->15->11->7->3->2->1->0      */
/* ------------------------------------------------------------------ */

/* Ordinea coloanelor in spirala - din interior spre exterior */
static const uint8_t spiral_cols[16] = {
    9, 10, 6, 5, 4, 8, 12, 13, 14, 15, 11, 7, 3, 2, 1, 0
};

/* Ciclul complet: 16 pasi creste + 16 pasi scade = 32 pasi
 * step  0: 1 coloana aprinsa  (doar 9)
 * step  1: 2 coloane aprinse  (9, 10)
 * ...
 * step 15: 16 coloane aprinse (tot cubul)
 * step 16: 15 coloane aprinse (incepe sa se contracte)
 * ...
 * step 30: 1 coloana aprinsa  (doar 9)
 * step 31: 0 coloane          (pauza scurta inainte de reluat) */
#define SPIRAL_STEPS  32

static void spiral_update(uint16_t frame[CUBE_LAYERS])
{
    uint8_t step = anim_step % SPIRAL_STEPS;
    uint8_t num_lit;

    if (step < 16) {
        num_lit = step + 1;       /* creste: 1 -> 16 */
    } else {
        num_lit = 31 - step;      /* scade: 15 -> 0  */
    }

    /* Aprind primele num_lit coloane din spirala pe toate straturile */
    for (uint8_t i = 0; i < num_lit; i++) {
        uint8_t col = spiral_cols[i];
        for (uint8_t z = 0; z < CUBE_LAYERS; z++) {
            frame[z] |= (1u << col);
        }
    }

    anim_step = (anim_step + 1) % SPIRAL_STEPS;
}

/* ------------------------------------------------------------------ */
/* Animatia Firework                                                    */
/* O coloana aleatoare urca de la layer 0 la layer 3 cu note           */
/* ascendente, explodeaza in tot cubul, apoi se stinge treptat.        */
/*                                                                      */
/* Step 0-3: racheta urca layer cu layer (note joase -> inalte)        */
/* Step 4:   explozie - tot cubul aprins (sunet puternic)              */
/* Step 5-7: stingere progresiva (note descrescatoare)                 */
/* Step 8:   pauza, alege o coloana noua aleatoare                    */
/* ------------------------------------------------------------------ */

/* Notele pentru racheta: E4, A4, D5, G5 (ascendente) */
static const uint16_t rocket_notes[4] = { 330, 440, 587, 784 };

/* Coloana curenta a rachetei - aleasa aleator la fiecare ciclu */
static uint8_t rocket_col = 0;

/* Patternul exploziei - generat aleator cand racheta ajunge sus */
static uint16_t explosion_pattern[CUBE_LAYERS];

#define FIREWORK_STEPS  10

/* Genereaza un pattern aleator de explozie pe toate straturile */
static void firework_generate_explosion(void)
{
    for (uint8_t z = 0; z < CUBE_LAYERS; z++) explosion_pattern[z] = 0;

    /* Aprind 12 LED-uri aleatoare ca sa para o explozie */
    for (uint8_t i = 0; i < 12; i++) {
        uint16_t r = (uint16_t)rand();
        uint8_t  z = r & 0x03;
        uint8_t  c = (r >> 2) & 0x0F;
        explosion_pattern[z] |= (1u << c);
    }
}

/* Aprinde num_leds LED-uri aleatoare in frame - efect sparkle */
static void firework_sparkle(uint16_t frame[CUBE_LAYERS], uint8_t num_leds)
{
    for (uint8_t i = 0; i < num_leds; i++) {
        uint16_t r = (uint16_t)rand();
        uint8_t  z = r & 0x03;
        uint8_t  c = (r >> 2) & 0x0F;
        frame[z] |= (1u << c);
    }
}

static void firework_update(uint16_t frame[CUBE_LAYERS])
{
    uint8_t step = anim_step % FIREWORK_STEPS;

    switch (step) {
        case 0: case 1: case 2: case 3:
            /* Racheta urca layer cu layer, cu note ascendente */
            frame[step] |= (1u << rocket_col);
            buzzer_start(rocket_notes[step]);
            break;

        case 4:
            /* Explozie - pattern aleator generat acum + sunet puternic */
            firework_generate_explosion();
            for (uint8_t z = 0; z < CUBE_LAYERS; z++) {
                frame[z] = explosion_pattern[z];
            }
            buzzer_start(1200);
            break;

        case 5:
            /* Scantei multe - explozia se raspandeste */
            firework_sparkle(frame, 10);
            buzzer_start(880);
            break;

        case 6:
            /* Scantei medii */
            firework_sparkle(frame, 7);
            buzzer_start(660);
            break;

        case 7:
            /* Scantei putine */
            firework_sparkle(frame, 4);
            buzzer_start(440);
            break;

        case 8:
            /* Ultimele scantei - aproape stins */
            firework_sparkle(frame, 2);
            buzzer_start(300);
            break;

        case 9:
            /* Pauza - opresc sunetul si aleg o coloana noua */
            buzzer_stop();
            rocket_col = (uint8_t)(rand() % CUBE_COLUMNS);
            break;
    }

    anim_step = (anim_step + 1) % FIREWORK_STEPS;
}


/* ------------------------------------------------------------------ */
/* Animatia Sparkle                                                     */
/* La fiecare frame aprind NUM_SPARKS LED-uri aleatoare pe tot cubul.  */
/* Pozitiile se schimba complet la fiecare pas - efect de sclipire.    */
/* ------------------------------------------------------------------ */

#define NUM_SPARKS  8

static void sparkle_update(uint16_t frame[CUBE_LAYERS])
{
    for (uint8_t i = 0; i < NUM_SPARKS; i++) {
        uint16_t r = (uint16_t)rand();
        uint8_t  z = r & 0x03;          /* strat aleator 0-3 */
        uint8_t  c = (r >> 2) & 0x0F;  /* coloana aleatoare 0-15 */
        frame[z] |= (1u << c);
    }
}

/* ------------------------------------------------------------------ */
/* API public                                                           */
/* ------------------------------------------------------------------ */

static const char * const anim_names[ANIM_COUNT] = {
    "Rain",
    "Plane Scan",
    "Expand",
    "Spiral",
    "Firework",
    "Sparkle",
};

void animations_init(uint16_t seed)
{
    /* Seed-uiesc rand() cu valoarea potentiometrului la pornire
     * Pozitia potentiometrului variaza => secventa Rain e diferita la fiecare pornire */
    srand(seed);

    current_anim = ANIM_RAIN;
    anim_step    = 0;
    last_update  = 0;
    rain_init();
}

void animations_set(animation_id_t anim)
{
    if (anim >= ANIM_COUNT) return;

    /* Opresc buzzerul la parasirea animatiei Firework */
    if (current_anim == ANIM_FIREWORK) {
        buzzer_stop();
    }

    current_anim = anim;
    anim_step    = 0;
    if (anim == ANIM_RAIN)     rain_init();
    if (anim == ANIM_FIREWORK) rocket_col = (uint8_t)(rand() % CUBE_COLUMNS);
}

animation_id_t animations_get_current(void)
{
    return current_anim;
}

void animations_next(void)
{
    animations_set((animation_id_t)((current_anim + 1) % ANIM_COUNT));
}

const char *animations_get_name(void)
{
    return anim_names[current_anim];
}

void animations_clear(uint16_t frame[CUBE_LAYERS])
{
    for (uint8_t i = 0; i < CUBE_LAYERS; i++) {
        frame[i] = 0x0000;
    }
}

void animations_update(uint32_t now_ms, uint16_t period_ms,
                       uint16_t frame[CUBE_LAYERS])
{
    /* Pattern non-blocant din Lab 2 - returnez imediat daca nu a trecut
     * destul timp, exact ca (now - last_msg) >= 1000 din ex1.c */
    if ((uint32_t)(now_ms - last_update) < period_ms) return;
    last_update = now_ms;

    animations_clear(frame);

    switch (current_anim) {
        case ANIM_RAIN:       rain_update(frame);         break;
        case ANIM_PLANE_SCAN: plane_scan_update(frame);   break;
        case ANIM_EXPAND:     expand_update(frame);       break;
        case ANIM_SPIRAL:     spiral_update(frame);       break;
        case ANIM_FIREWORK:   firework_update(frame);     break;
        case ANIM_SPARKLE:    sparkle_update(frame);      break;
        default:                                          break;
    }
}
