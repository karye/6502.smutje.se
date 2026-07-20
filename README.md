# W65C02 8-bitarsdator med Arduino Mega-emulering

Detta projekt är en fristående 8-bitarsdator baserad på mikroprocessorn **W65C02S**. Istället for att använda fysiska EEPROM-, RAM- och logikchip i det första skedet, används en **Arduino Mega 2560** för att simulera datorns minne och klockcykler. 

Eftersom W65C02S har en helt statisk kärna (CMOS) kan klockan stoppas helt utan att processorn tappar bort sitt interna tillstånd. Detta utnyttjas i projektet genom att fysiska knappar låter dig stega igenom instruktionerna manuellt. En lysdiod visar klockans status i realtid, och en tvåraders LCD-display visar aktuell systemstatus direkt på testbänken.

## Projektbeskrivning

Systemet fungerar genom att Arduinon agerar som en "hårdvarusimulator". I varje klockcykel (PHI2) läser Arduinon av processorns adressbuss och R/W-pinne. Om processorn vill läsa kod, hämtar Arduinon rätt instruktion från sitt eget minne (Flash/RAM) och lägger ut det på databussen. Om processorn vill skriva, sparar Arduinon datan.

Den inbyggda LCD-displayen och klock-LED:en ger dig omedelbar visuell feedback utan att du ständigt måste titta på en datorskärm.

## Komponenter som används

*   1 x **W65C02S** (DIP-40 mikroprocessor)
*   1 x **Arduino Mega 2560** (Plus eller standard)
*   1 x **LCD1602** med I2C/IIC-backpack (Tvåraders display)
*   1 x **Lysdiod** (T.ex. röd eller gul för klockindikering)
*   1 x **220 Ω eller 330 Ω motstånd** (Strömbegränsning för lysdioden)
*   5 x **3,3 kΩ eller 10 kΩ motstånd** (Pull-up för CPU-kontrollpinnar)
*   8 x **100 Ω motstånd** (Seriemotstånd på databussen som skydd mot busskollision)
*   8 x **Lysdiod** (Röd, för visuell avläsning av databussen D0–D7)
*   8 x **220 Ω motstånd** (Strömbegränsning för databuss-LED:arna)
*   1 x **104 (100nF) keramisk kondensator** (Avkoppling för CPU)
*   1 x **10 µF eller 47 µF elektrolytisk kondensator** (Strömstabilisering)
*   2 x **Taktila tryckknappar** (För stegning av klocka och instruktioner)
*   Kopplingsdäck (Breadboards) och solid kopplingstråd

---

## Fysiskt kopplingsschema

För att Arduinon ska hinna med att läsa och skriva på bussarna är kablarna grupperade till specifika hårdvaruportar (`PORTA`, `PORTF`, `PORTK`). Detta gör att koden kan läsa 8 eller 16 bitar samtidigt.

### Ström och kontrollpinnar

| W65C02 (Pin) | Signal | Kopplas till | Kommentar |
| :--- | :--- | :--- | :--- |
| **Pin 8** | VDD | Arduino **5V** | Strömmatning |
| **Pin 21** | VSS | Arduino **GND** | Systemlogisk jord (VSS) |
| **Pin 36** | PHI2 | Arduino **D2** | Klockinmatning (Genereras av Arduinon) |
| **Pin 34** | R/W | Arduino **D3** | Read/Write (Hög = Läs, Låg = Skriv) |
| **Pin 40** | /RESET | Arduino **D4** | Styrs av Arduinon för kontrollerad omstart |
| **Pin 7** | SYNC | Arduino **D7** | Opcode fetch-indikering (hög vid instruktionshämtning) |

### Kritiska fasta anslutningar (Pull-up)
Följande pinnar på W65C02 **måste** dras till 5V via ett 3,3 kΩ eller 10 kΩ motstånd för att CPU:n inte ska krascha eller flyta:
*   **Pin 2 (RDY)** -> 5V via motstånd
*   **Pin 4 (/IRQ)** -> 5V via motstånd
*   **Pin 6 (/NMI)** -> 5V via motstånd
*   **Pin 35 (BE)** -> 5V via motstånd *(Extremt viktigt på CMOS-versionen!)*
*   **Pin 38 (/SO)** -> 5V via motstånd

### Filter och avkoppling
*   Placera den **keramiska 104-kondensatorn (100nF)** direkt mellan Pin 8 (VDD) and Pin 21 (VSS) på processorn, så nära chippet som möjligt.
*   Placera en **elektrolytisk kondensator (10µF eller 47µF)** där spänningskablarna från Arduinon går in på kopplingsdäckets strömskenor (strömskena + till -). *Vänd den långa pinnen till + och den markerade minussidan till GND.*

### Adressbuss (A0–A15)
Kopplas till de analoga ingångarna på Arduinon för att kunna läsas av direkt via register `PINK` och `PINF`.

| W65C02 (Pin) | Adress | Arduino Mega | Hårdvaruport |
| :--- | :--- | :--- | :--- |
| **Pin 9 till 16** | A0 – A7 | **A8 till A15** | `PORTK` (Låga adressblocket) |
| **Pin 17-20 & 22-25** | A8 – A15 | **A0 till A7** | `PORTF` (Höga adressblocket) |

### Databuss (D0–D7)
Kopplas till digitala pinnar 22–29 via **100 Ω seriemotstånd** på varje ledare.

> **Varning – busskollision!** När CPU:n skriver data (R/W låg) måste Arduinons PORTA-pinnar vara satta som `INPUT` (högimpedans) *innan* PHI2 går hög. Om Arduinon driver bussen samtidigt som CPU:n uppstår en kortslutning som kan förstöra båda kretsarna. Seriemotstånden (100 Ω) begränsar strömmen vid en oavsiktlig kollision och rekommenderas starkt.

*Var noggrann med ordningen eftersom CPU:ns pinnar räknas baklänges på högersidan.*

| W65C02 (Pin) | Data | Arduino Mega | Hårdvaruport |
| :--- | :--- | :--- | :--- |
| **Pin 33** | D0 | **Digital 22** | `PORTA` (Bit 0) |
| **Pin 32** | D1 | **Digital 23** | `PORTA` (Bit 1) |
| **Pin 31** | D2 | **Digital 24** | `PORTA` (Bit 2) |
| **Pin 30** | D3 | **Digital 25** | `PORTA` (Bit 3) |
| **Pin 29** | D4 | **Digital 26** | `PORTA` (Bit 4) |
| **Pin 28** | D5 | **Digital 27** | `PORTA` (Bit 5) |
| **Pin 27** | D6 | **Digital 28** | `PORTA` (Bit 6) |
| **Pin 26** | D7 | **Digital 29** | `PORTA` (Bit 7) |

### Databuss-LED:ar (D0–D7 visuell avläsning)

Åtta lysdioder kopplas mellan varje datalinje och GND (via 220 Ω motstånd) så att du kan läsa av databussens 8-bitarsvärde binärt i realtid. LED:en tänds när motsvarande bit är hög (logisk 1). Koppla LED:arna på **CPU-sidan** av 100 Ω-seriemotstånden.

| Datalinje | LED + motstånd | Anslutning |
| :--- | :--- | :--- |
| D0 | LED0 + 220 Ω | Mellan D0 (CPU pin 33) och GND |
| D1 | LED1 + 220 Ω | Mellan D1 (CPU pin 32) och GND |
| D2 | LED2 + 220 Ω | Mellan D2 (CPU pin 31) och GND |
| D3 | LED3 + 220 Ω | Mellan D3 (CPU pin 30) och GND |
| D4 | LED4 + 220 Ω | Mellan D4 (CPU pin 29) och GND |
| D5 | LED5 + 220 Ω | Mellan D5 (CPU pin 28) och GND |
| D6 | LED6 + 220 Ω | Mellan D6 (CPU pin 27) och GND |
| D7 | LED7 + 220 Ω | Mellan D7 (CPU pin 26) och GND |

### Klockindikering (LED)

För att se klockpulsen visuellt kopplas en lysdiod till klocklinjen. När klockan går hög tänds dioden. Ett motstånd skyddar dioden från att brinna upp.

```text
Klocklinje (Arduino D2 / CPU Pin 36) 
  │
  └───[ 220 Ω eller 330 Ω Motstånd ]───►│ (Lysdiod) ───┐
                                                       │
                                                  Arduino GND

```

### Tvåraders display (LCD1602 I2C)

Displayen har en I2C-backpack på baksidan och behöver bara fyra kablar. Den kopplas direkt till Arduinons dedikerade hårdvaru-I2C-pinnar (20 och 21). Detta gör att Arduinon kan skriva ut information om vad 6502-processorn gör på skärmen (t.ex. "A: $8000 D:$EA").

| LCD1602 I2C Pin | Kopplas till | Kommentar |
| --- | --- | --- |
| **GND** | Arduino **GND** | Jord |
| **VCC** | Arduino **5V** | Strömmatning (+5V) |
| **SDA** | Arduino **Pin 20 (SDA)** | Dataöverföring |
| **SCL** | Arduino **Pin 21 (SCL)** | Klocksynkronisering för I2C |

De flesta LCD1602 I2C-backpackar använder adress **`0x27`** eller **`0x3F`**. Kör en I2C-skanner på Arduinon för att verifiera adressen före programmering.

### Knappar för manuell stegning

För att du ska kunna kontrollera exekveringen kopplas två knappar direkt till Arduinon. Vi använder Arduinons interna pull-up-motstånd, vilket innebär att knapparna kopplas direkt mellan pinne och jord.

```text
  Arduino D5 (Klocksteg) -------[ Knapp ]------- Arduino GND
  Arduino D6 (Instruktionssteg) -[ Knapp ]------- Arduino GND

```

> **Notera – avstudsning (debounce):** Mekaniska knappar studsar vid tryck och kan generera flera oönskade pulsar. Avstudsning måste hanteras i mjukvaran (t.ex. en 50 ms spärrtid efter varje registrerat tryck).

* **Knapp 1 (Pin D5 - Clock Step):** Varje tryckning matar exakt en klockcykel (PHI2 Hög -> Låg). Lysdioden tänds och släcks i takt med dina tryck. Perfekt för att se mikrostegen inuti en instruktion.
* **Knapp 2 (Pin D6 - Instruction Step):** Håller klockan rullande tills processorns `SYNC`-pinne (Pin 7) slår om, vilket indikerar att en helt ny instruktion har laddats.

---

## Stegvis bygg- och testplan

För att undvika fel är det bäst att bygga datorn i faser. Testa varje fas innan du går vidare till nästa.

### Steg 1: Strömmatning och grundläggande klocka

1. Placera W65C02 på kopplingsdäcket.
2. Koppla 5V och GND från Arduinon till kopplingsdäckets strömskenor. Montera den elektrolytiska kondensatorn över skenorna.
3. Koppla Pin 8 (VDD) och Pin 21 (VSS) till strömskenorna. Sätt den keramiska 104-kondensatorn direkt över chippets strömpinnar.
4. Koppla alla 5 pull-up-motstånd (pinnar 2, 4, 6, 35, 38) till 5V.
5. Koppla klockan från Arduino D2 till CPU Pin 36 samt din LED med tillhörande motstånd.
6. **Test:** Ladda upp ett enkelt blink-skript till Arduinon som slår på/av Pin D2 med 1 Hz frekvens. Kontrollera att din LED blinkar stadigt.

### Steg 2: Adressbuss och reset-sekvens

1. Koppla bort strömmen. Koppla in alla 16 adresslinjer från CPU:n till Arduinons analoga pinnar (A0–A15) enligt tabellen.
2. Koppla Arduino Pin D4 till CPU Pin 40 (/RESET).
3. Skriv ett Arduino-skript som drar /RESET låg i 5 klockcykler vid uppstart och därefter håller den hög. Låt skriptet skriva ut värdet av adressbussen till den seriella monitorn vid varje klockcykel.
4. **Test:** Starta systemet. Kontrollera i serieövervakaren att processorn, efter att reset släpps, utför sin startsekvens och försöker läsa från adresserna `$FFFC` och `$FFFD` (Reset-vektorn).

### Steg 3: Databuss och minnesemulering

1. Koppla in de 8 datalinjerna (D0–D7) mellan CPU:n och Arduino Pin D22–D29 via 100 Ω seriemotstånd.
2. Koppla R/W (CPU Pin 34) till Arduino Pin D3.
3. Uppdatera Arduino-koden: Skapa en minnes-array i Arduinon. Lägg startadressen (t.ex. `$8000`) på adress `$FFFC/$FFFD`. Lägg en `NOP`-instruktion (`$EA`) eller en loop på adress `$8000`.
4. Konfigurera koden så att den sätter `PORTA` till utgångar och lägger ut rätt data när R/W är hög (läs), och sätter dem till ingångar när R/W är låg (skriv).
5. **Test:** Starta datorn. Verifiera i serieövervakaren att processorn läser startvektorn, hoppar till `$8000` och börjar exekvera dina instruktioner cykel för cykel.

### Steg 4: Interaktiv stegning och LCD-display

1. Koppla in de två knapparna till Arduino Pin D5 och D6 samt CPU:ns SYNC-pinne till Arduino D7.
2. Koppla in LCD1602 via I2C till Arduino Pin 20 och 21.
3. Uppdatera koden så att klockan inte längre körs automatiskt, utan väntar på knapptryckningar (avläsning av D5 eller D6).
4. Lägg till kod i Arduinon för att formatera adress- och databussens värden till hexadecimal text och skicka till LCD-displayen.
5. **Test:** Du kan nu trycka på klockknappen för att se processorn stega manuellt. Skärmen ska visa exakt vilken adress och instruktion som körs i realtid på ditt skrivbord.


