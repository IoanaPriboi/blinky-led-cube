# Blinky LED Cube

Cub LED 4x4x4 controlat de un Arduino Nano V3 (ATmega328P), programat in C pur, fara framework Arduino.

## Functionalitati

**Modul Animatie** - 6 animatii selectabile cu butoanele, viteza controlata din potentiometru:
- Rain - picaturi care cad aleator de sus in jos
- Plane Scan - un strat complet se aprinde si se deplaseaza de jos in sus
- Expand - LED-urile se extind din centrul cubului spre exterior si se contracta
- Spiral - coloanele se aprind in spirala din interior spre exterior
- Firework - racheta urca, explodeaza si lasa scantei, sincronizat cu sunete din buzzer
- Sparkle - LED-uri aleatoare clipesc haotic pe tot cubul

**Modul Timer** - countdown configurabil (10s - 99min 50s), cu afisare vizuala pe cub si alerta buzzer la expirare.

## Hardware

- Arduino Nano V3 / ATmega328P
- Cub LED 4x4x4 (64 LED-uri)
- 2x 74HC595 (shift registers pentru coloane)
- 4x tranzistor NPN 2N2222 (pentru straturi)
- LCD 16x2 cu adaptor I2C (PCF8574, adresa 0x27)
- Buzzer pasiv
- Potentiometru 10k
- 4 butoane

## Software

Proiect PlatformIO, compilat cu avr-gcc. Toate perifericele sunt controlate direct prin registre AVR (fara biblioteci Arduino).

```
src/        - fisiere sursa .c
include/    - headere .h
platformio.ini
```

## Proiect

Realizat in cadrul cursului Proiectarea cu Microprocesoare, UPB.
