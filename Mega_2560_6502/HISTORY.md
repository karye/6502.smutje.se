# Session 2026-07-26 till 2026-07-27 — Steg 3 Debugging

## Översikt
Två dagars intensiv felsökning av W65C02S + Arduino Mega 2560.
Från kompileringsfel till fullt fungerande CPU med NOP-loop på $8000,
LED-klocka på 20 Hz och stabil LCD-display.

---

## 1. Kompileringsfel: `pulse()` ej deklarerad

**Symtom:**
```
src\main.cpp:56:33: error: 'pulse' was not declared in this scope
src\main.cpp:61:34: error: 'pulse' was not declared in this scope
```

**Orsak:** `pulse()` definieras efter `setup()` men anropas i `setup()`.

**Lösning:** Lade till `void pulse();` (forward declaration) före `setup()`.

---

## 2. Skräptecken i serieövervakaren

**Symtom:** `��������������������������������...`

**Orsak:** Baud rate mismatch. `Serial.begin(115200)` i koden, men PlatformIO
använder 9600 som standard för serial monitor.

**Lösning:** Lade till `monitor_speed = 115200` i `platformio.ini`.

---

## 3. `W $1` — CPU:n kör inte

**Symtom:**
```
W $1  [PINK=0x1 PINF=0x0 RWB=L]
W $1  [PINK=0x1 PINF=0x0 RWB=L]
...
```

**Orsak 1:** BE (Bus Enable, CPU pin 36) dragen till +5V via pull-up enligt schemat.
BE är aktiv låg — HIGH = bussarna avstängda (högimpedans).
CPU:n driver varken adressbuss, databuss eller RWB.

**Fix 1:** Kopplade BE (pin 36) till **GND** istället för +5V.

---

## 4. `W $0` — CPU:n fortfarande död

**Symtom:**
```
W $0  [PINK=0x0 PINF=0x0 RWB=L]
W $0  [PINK=0x0 PINF=0x0 RWB=L]
```

### 4a. RESB spänningsfall (LED-problemet)

**Mätning:** RESB = 2.48V (ska vara ~4.7V när HIGH)

**Orsak:** Lysdiod + 220Ω motstånd kopplad till RESB-linjen (från schemat).
LED:n skapar en spänningsdelare som drar ner RESB under CMOS-tröskeln.

**Fix:** Kopplade bort LED + 220Ω från RESB-linjen.

### 4b. För låg matningsspänning

**Mätning:** VDD = 4.644V (för lågt för stabil CPU-drift)

**Orsak:** USB-porten på datorn gav för lite ström.

**Fix 1:** Bytte USB-kabel — VDD gick från 4.644V till 4.777V (fortfarande lågt)

**Fix 2:** Kopplade in **12V DC-adapter** i Arduinons barrel-jack → VDD = 5.088V ✅

**Fix 3:** Senare uppgraderad till **18V DC-adapter** — ännu stabilare.

### 4c. RESB = 3.5V trots D4 = 4.7V

**Mätning:** D4 på Arduinons stift = 4.7V, men RESB vid CPU pin 40 = 3.5V

**Diagnos:** Spänningsfall på ~1.2V över kopplingstråden — onormalt högt.

**Isolering:** Kopplade bort allt utom VDD, VSS, BE, RESB. RESB = 3.55V
fortfarande. D4-tråden fri = 4.7V. CPU pin 40 fri = 3.5V.

**Slutsats:** CPU:n eller kopplingsbrädan kortsluter RESB internt.

**Fix:** **Flyttade CPU:n till en ny plats på breadboard.**
RESB = 4.1V direkt (med bara VDD/VSS inkopplat). ✅

**Lärdom:** Kopplingsbrädor kan ha dolda kortslutningar mellan raderna.
Flytta chipet till en helt annan del av brädan vid misstänkt kortis.

---

## 5. Databussen kopplad till fel Arduino-pinnar

**Symtom:** Databussen kopplad till D14–D21.

**Orsak:** Koden använder PORTA (Arduino D22–D29) för databussen,
inte D14–D21.

**Fix:** Kopplade om till:
```
CPU D0 (pin 33) → Arduino D22  (PORTA bit 0)
CPU D1 (pin 32) → Arduino D23  (PORTA bit 1)
...
CPU D7 (pin 26) → Arduino D29  (PORTA bit 7)
```
Med 100Ω seriemotstånd på varje ledning (skydd mot busskollision).

**Lärdom:** 100Ω motstånden är billig försäkring. Utan dem kan en
felaktig DDRA-inställning orsaka ~5A ström genom CPU+Arduino → brandrisk.

---

## 6. Reset-vektor ej sparad — CPU:n hoppar till $EAEA

**Symtom:** CPU:n hoppar till $EAEA istället för $8000 efter reset.

**Orsak:** `ram[512]` täcker bara adresser 0–511 ($0000–$01FF).
Reset-vektorn på $FFFC–$FFFD (65532–65533) är långt utanför.
`write_mem()` ignorerar tyst adresser utanför ram[], och `read_mem()`
returnerar fallback-värdet 0xEA (NOP) för alla adresser ≥512.

Resultat: CPU:n läser 0xEA från $FFFC, 0xEA från $FFFD → reset-vektor = $EAEA.

**Fix:** Skapade separat `uint8_t vectors[6]` för $FFFA–$FFFF:
- `vectors[0..1]` = NMI-vektor ($FFFA–$FFFB)
- `vectors[2..3]` = Reset-vektor ($FFFC–$FFFD)
- `vectors[4..5]` = IRQ/BRK-vektor ($FFFE–$FFFF)

`read_mem()` och `write_mem()` hanterar nu adresser $FFFA–$FFFF via
denna array, medan övriga adresser >511 returnerar 0xEA.

---

## 7. Adress-byteordning (PINK vs PINF)

**Symtom:** CPU:n läste från $FF00 istället för $8000.

**Bakgrund:** Schemat har korsad adressbuss-koppling:
- Arduino A8–A15 → CPU A0–A7 (låg byte)
- Arduino A0–A7 → CPU A8–A15 (hög byte)

Men användaren kopplade **1:1** (rak koppling):
- Arduino A0–A7 → CPU A0–A7 (låg byte)
- Arduino A8–A15 → CPU A8–A15 (hög byte)

Originalkod: `uint16_t a = (PINK << 0) | (PINF << 8);`
→ PINK (A8–A15) som låg byte, PINF (A0–A7) som hög byte → **fel för 1:1**.

**Fix:** Ändrade till `uint16_t a = (PINF << 0) | (PINK << 8);`
→ PINF (A0–A7) som låg byte, PINK (A8–A15) som hög byte → **rätt för 1:1**.

**Lärdom:** Verifiera alltid kopplingen innan du litar på kommentarerna i koden.

---

## 8. CPU:n fast i BRK-loop

**Symtom:**
```
R $FFFE  [PINK=0xFF PINF=0xFE RWB=H]
R $FFFF  [PINK=0xFF PINF=0xFF RWB=H]
R $0     [PINK=0x0 PINF=0x0 RWB=H]
R $1     [PINK=0x0 PINF=0x1 RWB=H]
W $1CD  ...
```
Mönstret repeteras: stack-push → läs IRQ/BRK-vektor → hoppa till $0000 → BRK → loopa.

**Orsak:** RESB (CPU pin 40) var **inte inkopplad alls**.
CPU:n körde fritt från power-up utan reset → slumpmässiga BRK-instruktioner.

**Fix:** Kopplade **Arduino D4 → CPU pin 40 (RESB)**.

---

## 9. CPU:n kör korrekt! 🎉

**Bekräftad utdata:**
```
R $FFFC  [PINK=0xFF PINF=0xFC RWB=H]  ← reset-vektor låg
R $FFFD  [PINK=0xFF PINF=0xFD RWB=H]  ← reset-vektor hög
R $8000  [PINK=0x80 PINF=0x0 RWB=H]   ← NOP-opcode
R $8001  [PINK=0x80 PINF=0x1 RWB=H]   ← NOP-prefetch
R $8001  ...                           ← nästa NOP
```

---

## 10. Klock-LED

**Koppling:** Arduino D2 (PHI2) → 220Ω → LED → GND

**Ursprunglig hastighet:** ~50 Hz (delay(10) per fas = 20ms/cykel)
→ LED lyser fast (för snabbt för ögat)

**Fix:** Sänkte till **20 Hz**:
```cpp
delay(25);  // 25ms per fas, 50ms/cykel = 20 Hz
```
LED blinkar nu synligt.

---

## 11. LCD QC1602A (parallell 4-bitarsläge)

### 11a. Första försöket — förvrängda tecken

**Koppling:** Standard 4-bit: RS→D5, E→D6, D4→D7, D5→D8, D6→D9, D7→D10

**Kod:** `LiquidCrystal lcd(5, 6, 7, 8, 9, 10);`

**Symtom:** Första raden helvit, eller enstaka tecken som "S", "Q".

### 11b. Lösning — omvänd datapinne-ordning

**Fix:** `LiquidCrystal lcd(5, 6, 10, 9, 8, 7);`
Motsvarar fysisk koppling:
```
LCD D4 → Arduino D10
LCD D5 → Arduino D9
LCD D6 → Arduino D8
LCD D7 → Arduino D7
```

**Resultat:** "W65C02S Steg 3" / "CPU redo" visas korrekt. ✅

### 11c. Realtidsuppdatering orsakade skräptecken

**Orsak:** `lcd.print()` i `loop()` anropades för ofta och/eller
skrev inte över hela raden → gamla tecken blev kvar.

**Lösning:** Behåll **bara statiskt startmeddelande** i `setup()`.
Ingen LCD-uppdatering i `loop()`.

---

## 12. `platformio.ini` — slutgiltig konfiguration

```ini
[env:megaatmega2560]
platform = atmelavr
board = megaatmega2560
framework = arduino
monitor_speed = 115200
lib_deps =
    LiquidCrystal
```

---

## Slutgiltig hårdvarukonfiguration

| Signal | CPU-pin | Arduino-pin | Not |
|--------|---------|-------------|-----|
| VDD | 8 | 5V | via 12V/18V-adapter |
| VSS | 21 | GND | |
| PHI2 | 37 | D2 | 20 Hz, LED via 220Ω→GND |
| RESB | 40 | D4 | |
| RWB | 34 | D3 | |
| BE | 36 | GND | AKTIV LÅG! Inte +5V! |
| RDY | 2 | +5V via 10kΩ pull-up | |
| IRQB | 4 | +5V via 10kΩ pull-up | |
| NMIB | 6 | +5V via 10kΩ pull-up | |
| SOB | 38 | +5V via 10kΩ pull-up | |
| A0–A7 | 9–16 | A0–A7 (PORTF) | 1:1-koppling |
| A8–A15 | 17–20,22–25 | A8–A15 (PORTK) | 1:1-koppling |
| D0–D7 | 26–33 | D22–D29 (PORTA) | 100Ω seriemotstånd |
| SYNC | 7 | (ej inkopplad än) | Steg 4 |
| 100nF | mellan pin 8 och 21 | | Avkopplingskondensator |

| LCD-pin | Arduino | Not |
|---------|---------|-----|
| RS | D5 | |
| E | D6 | |
| D4 | D10 | Omvänd ordning! |
| D5 | D9 | |
| D6 | D8 | |
| D7 | D7 | |
| RW | GND | |
| V0 | Potentiometer | Kontrast |

---

## Viktigaste lärdomar

1. **BE (pin 36) måste vara LÅG** — schemat visar pull-up till +5V, men det
   stänger av bussarna helt. Detta är ett fel i schemat för steg 2+.

2. **Kopplingsbrädor kan ha dolda kortslutningar** — flytta chipet till
   ny plats vid misstänkt hårdvarufel.

3. **Mät alltid spänningen VID CPU:n** — inte bara på Arduinons stift.
   Spänningsfall över kablar/bräda avslöjar kortslutningar.

4. **100Ω skyddsmotstånd på databussen** är obligatoriska — utan dem
   kan en busskollision förstöra både CPU och Arduino.

5. **Adressområdet för vektorer** ($FFFA–$FFFF) ligger utanför ett
   512-byte RAM-block — måste hanteras separat.

6. **Verifiera kopplingen mot koden** — kommentarer i koden kan vara
   föråldrade eller skrivna för en annan kopplingsvariant.

---

## ⚠ Avvikelser från schemat

Följande kopplingar i verkligheten skiljer sig från de genererade
schemana (`schematics/step*.svg`). Schemana behöver uppdateras.

### BE (pin 36)
| | Schema | Verklighet |
|---|--------|-----------|
| Koppling | +5V via 3.3kΩ pull-up | **GND** |
| Orsak | Alla steg drar BE till +5V | BE aktiv låg — HIGH stänger av bussarna |

### Adressbuss
| | Schema | Verklighet |
|---|--------|-----------|
| A0–A7 | Arduino A8–A15 → CPU A0–A7 (korsad) | Arduino A0–A7 → CPU A0–A7 (1:1) |
| A8–A15 | Arduino A0–A7 → CPU A8–A15 (korsad) | Arduino A8–A15 → CPU A8–A15 (1:1) |
| Kod | `(PINK \|\| PINF<<8)` enligt orginal | `(PINF \|\| PINK<<8)` anpassad för 1:1 |

### LCD
| | Schema (steg 5) | Verklighet |
|---|-----------------|-----------|
| Typ | I2C (PCF8574) | **Parallell 4-bit** (QC1602A) |
| Pins | SDA, SCL (2 stift) | D5, D6, D7, D8, D9, D10 (6 stift) |
| Kod | `LiquidCrystal_I2C` | `LiquidCrystal(5,6,10,9,8,7)` (omvänd dataordning) |

### LED
| | Schema | Verklighet |
|---|--------|-----------|
| Koppling | RESB-linjen via 220Ω | **D2 (PHI2)** via 220Ω |
| Orsak | LED på RESB drar ner spänningen | Flyttad till PHI2 för att visa klockan |

### Reset-vektor ($FFFC–$FFFD)
| | Schema/kod | Verklighet |
|---|-----------|-----------|
| RAM | 512 bytes (0x000–0x1FF) | Räcker ej — vektorerna på 0xFFFC–0xFFFD ligger utanför |
| Fix | — | Separat `uint8_t vectors[6]` för 0xFFFA–0xFFFF |
