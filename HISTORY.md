# Felsökningshistorik — W65C02S + Arduino Mega 2560

## 2026-07-26 — Steg 3: Första uppladdning och felsökning

### Fel 1: Kompileringsfel — `pulse` undeclared

**Symptom:**
```
src\main.cpp:56:33: error: 'pulse' was not declared in this scope
```

**Orsak:** Funktionen `pulse()` anropas i `setup()` men definieras först efter `setup()`. C++ kräver att funktioner deklareras eller definieras före användning.

**Lösning:** Lade till en forward declaration `void pulse();` före `setup()` i `src/main.cpp`.

---

### Fel 2: Skräptecken i serieövervakaren

**Symptom:**
```
��������������������������������������������...
```

**Orsak:** `Serial.begin(115200)` i koden, men PlatformIO:s serial monitor använder 9600 baud som standard.

**Lösning:** Lade till `monitor_speed = 115200` i `platformio.ini`.

---

### Fel 3: `W $1` — CPU:n driver inte bussarna

**Symptom:**
```
W $1
W $1
W $1
...
```

**Orsak:** BE (Bus Enable, pin 36) var dragen till **+5V** via 3.3kΩ pull-up enligt schemat i `gen_overview.py`. BE är **aktiv låg** — med HIGH är adressbuss, databuss och RWB i högimpedans (tri-state). CPU:n drev aldrig bussarna, så Arduino läste flytande värden.

**Lösning:** Flyttade BE (pin 36) från +5V till **GND**.

---

### Fel 4: `W $0` — CPU:n fortfarande inte aktiv

**Symptom:**
```
W $0  [PINK=0x0 PINF=0x0 RWB=L]
W $0  [PINK=0x0 PINF=0x0 RWB=L]
...
```

**Orsak:** Låga spänningar överallt — CPU:n fick inte tillräcklig ström.

---

### Fel 5: Låga spänningar på CPU:n

**Spänningsmätningar vid CPU:n (alla för låga):**

| Signal | Uppmätt | Förväntat |
|--------|---------|-----------|
| VDD (pin 8) | 4.644V | 4.9–5.1V |
| RESB (pin 40) | 2.48V → 3.8V | ~5V |
| RWB (pin 34) | 2.122V → 3.966V | ~5V |

**Åtgärder som testades:**
- Bortkoppling av LED + 220Ω från RESB-linjen → marginell förbättring
- Byte av USB-kabel → VDD ökade till 4.777V (fortfarande lågt)

---

### Fel 6: Spänningsfall över RESB-kopplingstråden

**Mätning:**
- Arduino D4-stift (direkt på brickan): **4.724V**
- CPU pin 40 (RESB): **3.568V**
- **Spänningsfall: 1.156V** — onormalt högt för en enkel kopplingstråd

**Slutsats:** Något drog betydande ström från RESB-linjen.

---

### Fel 7: CPU pin 40 visar 3.5V helt oinkopplad

**Testuppställning:** Endast VDD och VSS inkopplade på CPU:n. RESB (pin 40) helt fri.

**Mätning:**
- D4-tråd (fri ände): **4.7V** ✅
- CPU pin 40 (inget inkopplat): **3.5V** ❌ — borde vara flytande (~0V eller slumpmässig)

---

### Rotorsak: Defekt plats på kopplingsbrädan

**Avgörande test:** Flyttade CPU:n till en **annan plats på breadboard**. Kopplade endast VDD och VSS.

**Resultat:** RESB = **4.1V** ✅ (normalt, med multimeterns last på intern pull-up)

**Slutsats:** Den ursprungliga platsen på kopplingsbrädan hade en **dold kortslutning/läckström** mellan RESB och GND internt i breadboardet. Detta förklarar alla tidigare symptom:

1. BE → +5V gjorde att bussarna var frånkopplade → `W $1` (flytande pinnar)
2. Efter BE → GND blev bussarna aktiva, men kortslutningen drog ner RESB → `W $0` (CPU i odefinierat läge)
3. Spänningsfallet på 1.156V över kopplingstråden kom från att kortslutningen drog ström
4. CPU pin 40 visade 3.5V oinkopplad på grund av läckström i breadboardet

---

## Lärdomar

1. **BE (pin 36) måste vara LÅG (GND)** för normal CPU-drift. Schemat i `gen_overview.py` har BE dragen till +5V tillsammans med RDY/IRQB/NMIB/SOB — detta är **fel** och bör rättas.

2. **LED + 220Ω bör inte sitta på RESB-linjen.** Den skapar en spänningsdelare som kan störa reset-signalen. Lysdioden bör flyttas till en separat Arduino-pinne eller till PHI2O.

3. **Kopplingsbrädor kan ha interna kortslutningar.** Vid misstänkta spänningsfall — flytta kretsen till en annan plats och testa igen.

4. **Mät spänning direkt på Arduino-stiften först,** sedan vid CPU:n. Detta isolerar snabbt om problemet sitter i Arduino, kablage, eller kopplingsbräda.

5. **`monitor_speed` i `platformio.ini`** måste matcha `Serial.begin()` — annars blir det skräptecken.
