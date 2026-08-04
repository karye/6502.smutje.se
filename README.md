# W65C02 8-bitarsdator med Arduino Mega-emulering

En hembyggd 8-bitarsdator med **W65C02S** CPU och **Arduino Mega 2560** som minnesemulator och klockgenerator. Projektet byggs stegvis — från en blinkande LED till en fullt fungerande dator med I/O-expansion (W65C22 VIA) och LCD-display styrd direkt av 6502-processorn.

## Komponenter (alla steg)

| Antal | Komponent |
|-------|-----------|
| 1 | W65C02S (DIP-40) |
| 1 | Arduino Mega 2560 |
| 1 | W65C22 VIA (DIP-40) — steg 7 |
| 1 | 74HC00 (quad NAND) — steg 7 |
| 1 | LCD 16×2 (parallell, t.ex. QC1602A) — steg 6 |
| 1 | Lysdiod (klockindikering) |
| 1 | 220 Ω motstånd (LED-strömbegränsning) |
| 4 | 10 kΩ motstånd (pull-up: RDY, IRQB, NMIB, SOB) |
| 8 | 100 Ω motstånd (databuss-skydd) |
| 1 | 100 nF keramisk kondensator (avkoppling CPU) |
| 1 | 10 µF elektrolytisk kondensator (strömstabilisering) |
| 2 | Tryckknappar (klocksteg, instruktionssteg) |
| — | Kopplingsdäck + kopplingstråd |

---

## W65C02S — pinout

| Pin | Namn | I/O | Beskrivning |
|-----|------|-----|-------------|
| 1 | VPB | Ut | Vector Pull — utgång, ansluts ej |
| 2 | RDY | In | Ready — HÖG = CPU kör, LÅG = paus |
| 3 | PHI1O | Ut | Phase 1 Out — klocka ut, ansluts ej |
| 4 | IRQB | In | Interrupt Request (aktiv LÅG) |
| 5 | MLB | Ut | Memory Lock — utgång, ansluts ej |
| 6 | NMIB | In | Non-Maskable Interrupt (aktiv LÅG) |
| 7 | SYNC | Ut | Opcode fetch — HÖG vid instruktionshämtning |
| 8 | VDD | — | Strömmatning +5V |
| 9 | A0 | Ut | Adressbuss bit 0 |
| 10 | A1 | Ut | Adressbuss bit 1 |
| 11 | A2 | Ut | Adressbuss bit 2 |
| 12 | A3 | Ut | Adressbuss bit 3 |
| 13 | A4 | Ut | Adressbuss bit 4 |
| 14 | A5 | Ut | Adressbuss bit 5 |
| 15 | A6 | Ut | Adressbuss bit 6 |
| 16 | A7 | Ut | Adressbuss bit 7 |
| 17 | A8 | Ut | Adressbuss bit 8 |
| 18 | A9 | Ut | Adressbuss bit 9 |
| 19 | A10 | Ut | Adressbuss bit 10 |
| 20 | A11 | Ut | Adressbuss bit 11 |
| 21 | VSS | — | Systemjord GND |
| 22 | A12 | Ut | Adressbuss bit 12 |
| 23 | A13 | Ut | Adressbuss bit 13 |
| 24 | A14 | Ut | Adressbuss bit 14 |
| 25 | A15 | Ut | Adressbuss bit 15 |
| 26 | D7 | I/O | Databuss bit 7 (MSB) |
| 27 | D6 | I/O | Databuss bit 6 |
| 28 | D5 | I/O | Databuss bit 5 |
| 29 | D4 | I/O | Databuss bit 4 |
| 30 | D3 | I/O | Databuss bit 3 |
| 31 | D2 | I/O | Databuss bit 2 |
| 32 | D1 | I/O | Databuss bit 1 |
| 33 | D0 | I/O | Databuss bit 0 (LSB) |
| 34 | RWB | Ut | Read/Write — HÖG = läs, LÅG = skriv |
| 35 | NC | — | Ej ansluten |
| 36 | BE | In | Bus Enable — HÖG = bussar aktiva |
| 37 | PHI2 | In | Phase 2 In — klockingång |
| 38 | SOB | In | Set Overflow (aktiv LÅG) |
| 39 | PHI2O | Ut | Phase 2 Out — klocka ut, ansluts ej |
| 40 | RESB | In | Reset (aktiv LÅG) |

---

## W65C22 VIA — pinout

| Pin | Namn | I/O | Beskrivning |
|-----|------|-----|-------------|
| 1 | VSS | — | Systemjord GND |
| 2 | PA0 | I/O | Port A bit 0 |
| 3 | PA1 | I/O | Port A bit 1 |
| 4 | PA2 | I/O | Port A bit 2 |
| 5 | PA3 | I/O | Port A bit 3 |
| 6 | PA4 | I/O | Port A bit 4 |
| 7 | PA5 | I/O | Port A bit 5 |
| 8 | PA6 | I/O | Port A bit 6 |
| 9 | PA7 | I/O | Port A bit 7 |
| 10 | PB0 | I/O | Port B bit 0 |
| 11 | PB1 | I/O | Port B bit 1 |
| 12 | PB2 | I/O | Port B bit 2 |
| 13 | PB3 | I/O | Port B bit 3 |
| 14 | PB4 | I/O | Port B bit 4 |
| 15 | PB5 | I/O | Port B bit 5 |
| 16 | PB6 | I/O | Port B bit 6 |
| 17 | PB7 | I/O | Port B bit 7 |
| 18 | CB1 | In | Port B kontroll 1 |
| 19 | CB2 | I/O | Port B kontroll 2 |
| 20 | VDD | — | Strömmatning +5V |
| 21 | IRQB | Ut | Interrupt Request (aktiv LÅG, open drain) |
| 22 | RWB | In | Read/Write — HÖG = läs, LÅG = skriv |
| 23 | CS2B | In | Chip Select 2 (aktiv LÅG) |
| 24 | CS1 | In | Chip Select 1 (aktiv HÖG) |
| 25 | PHI2 | In | Klockingång |
| 26 | D7 | I/O | Databuss bit 7 (MSB) |
| 27 | D6 | I/O | Databuss bit 6 |
| 28 | D5 | I/O | Databuss bit 5 |
| 29 | D4 | I/O | Databuss bit 4 |
| 30 | D3 | I/O | Databuss bit 3 |
| 31 | D2 | I/O | Databuss bit 2 |
| 32 | D1 | I/O | Databuss bit 1 |
| 33 | D0 | I/O | Databuss bit 0 (LSB) |
| 34 | RESB | In | Reset (aktiv LÅG) |
| 35 | RS3 | In | Register Select bit 3 |
| 36 | RS2 | In | Register Select bit 2 |
| 37 | RS1 | In | Register Select bit 1 |
| 38 | RS0 | In | Register Select bit 0 |
| 39 | CA2 | I/O | Port A kontroll 2 |
| 40 | CA1 | In | Port A kontroll 1 |

---

## CPU-kopplingar (gäller alla steg)

| CPU-pin | Signal | Arduino | Not |
|---------|--------|---------|-----|
| 8 | VDD | 5V | Strömmatning |
| 21 | VSS | GND | Systemjord |
| 37 | PHI2 | D2 | Klocka (20 Hz) |
| 34 | R/W | D3 | Read/Write |
| 40 | /RESET | D4 | Reset |
| 2 | RDY | 5V via 10kΩ | Pull-up |
| 4 | /IRQ | 5V via 10kΩ | Pull-up |
| 6 | /NMI | 5V via 10kΩ | Pull-up |
| 36 | BE | 5V via 10kΩ | Aktiv hög |
| 38 | /SO | 5V via 10kΩ | Pull-up |

### Adressbuss (1:1)
| CPU | Arduino | Register |
|-----|---------|----------|
| A0–A7 (pin 9–16) | A0–A7 | `PORTF` |
| A8–A15 (pin 17–20,22–25) | A8–A15 | `PORTK` |

### Databuss
| CPU | Arduino | Register |
|-----|---------|----------|
| D0–D7 (pin 26–33) | D22–D29 | `PORTA` |

### Knappar & SYNC
| Signal | Arduino |
|--------|---------|
| SYNC (CPU pin 7) | D13 |
| BTN1 (klocksteg) | D11 → GND |
| BTN2 (instr.steg) | D12 → GND |

---

## Adressrymd

```
$FFFF ┌──────────────┐
      │  Arduino ROM  │  A15=1 ($8000–$BFFF)
$C000 ├──────────────┤
      │  W65C22 VIA   │  $C000–$C00F (steg 7)
$8000 ├──────────────┤
      │  Arduino ROM  │  $8000–$BFFF (6502-program)
$0000 └──────────────┘
```

---

## Stegöversikt

| Steg | Innehåll | Ny hårdvara |
|------|----------|-------------|
| 1 | CPU + ström + klocka + LED | CPU, LED, 100nF, 10µF |
| 2 | Adressbuss + reset | 16 kopplingstrådar |
| 3 | Databuss + minnesemulering | 8×100Ω |
| 4 | Knappar + stegning | 2 knappar |
| 5 | LCD via Arduino (parallell 4-bit) | LCD, 10kΩ pot |
| 6 | LCD direkt från Arduino, 6502-program | — (samma hårdvara) |
| 7 | W65C22 VIA + 74HC00, LCD via VIA | VIA, 74HC00 |

---

## Steg 5–6: LCD-koppling (parallell 4-bit)

| LCD-pin | Arduino | Funktion |
|---------|---------|----------|
| VSS (1) | GND | Jord |
| VDD (2) | 5V | Ström |
| VO (3) | Potentiometer | Kontrast |
| RS (4) | D5 | Register Select |
| R/W (5) | GND | Write-only |
| E (6) | D6 | Enable |
| DB4 (11) | D10 | Data (omvänd ordning) |
| DB5 (12) | D9 | |
| DB6 (13) | D8 | |
| DB7 (14) | D7 | |
| A (15) | 5V via 220Ω | Bakgrundsbelysning |
| K (16) | GND | |

Kod: `LiquidCrystal lcd(5, 6, 10, 9, 8, 7)` — notera omvänd dataordning.

---

## Steg 7: VIA + 74HC00

### W65C22 VIA — bussanslutning
| VIA-pin | Signal | Ansluts till |
|---------|--------|-------------|
| 25 | PHI2 | CPU PHI2 (pin 37) |
| 22 | R/W | CPU R/W (pin 34) |
| 34 | /RESET | CPU /RESET (pin 40) |
| 24 | CS1 | +5V (alltid aktiv) |
| 23 | /CS2 | 74HC00 utgång (pin 8) |
| 38–35 | RS0–RS3 | CPU A0–A3 |
| 33–26 | D0–D7 | CPU D0–D7 (databuss) |
| 20 | VDD | +5V |
| 1 | VSS | GND |

### 74HC00 — adressavkodning
| Grind | Pins | Funktion |
|-------|------|----------|
| A | 1,2 → 3 | A14 → båda ingångar = NOT A14 |
| B | 4,5 → 6 | A15 → båda ingångar = NOT A15 |
| C | 9,10 → 8 | Utgång → VIA /CS2 |
| 14 | VCC | +5V |
| 7 | GND | GND |

VIAn aktiveras när A14=A15=1 ($C000–$FFFF).

### VIA → LCD
| VIA-pin | LCD-pin | Funktion |
|---------|---------|----------|
| PA0 (2) | RS (4) | Register Select |
| PA2 (4) | E (6) | Enable |
| PB0–PB7 (10–17) | DB0–DB7 (7–14) | 8-bit data |

---

## Varningar

- **100Ω seriemotstånd på databussen är obligatoriska** — utan dem kan en busskollision förstöra CPU och Arduino.
- **BE (pin 36) måste vara HÖG (+5V)** — LÅG tri-statar bussarna.
- **Använd 12–18V DC-adapter** till Arduino för stabil 5V-matning.
- **Kopplingsbrädor kan ha dolda kortslutningar** — flytta kretsen vid misstänkt fel.
