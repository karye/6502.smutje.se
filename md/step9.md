# Riktigt RAM (62256 SRAM)

Att koppla in det första riktiga minneschippet känns som att ge datorn ett eget rum. Från och med nu är det äkta kisel som svarar när processorn knackar på.
## Mål

Hittills har Arduino emulerat allt minne — varje gång CPU:n läser eller skriver en byte är det Arduino som svarar. Det fungerar utmärkt för inlärning och felsökning, men en riktig dator har riktigt minne. Nu tar jag steget och kopplar in ett 62256 SRAM-chip — 32 kilobyte statiskt RAM i en DIP-28-kapsel.

SRAM-chippet tar över adresserna `$0000`–`$3FFF` (16 KB). Det täcker zero page, stacken och gott om utrymme för variabler. Arduino fortsätter att leverera programkoden från `$8000` och uppåt, samt vektorerna på `$FFFA`–`$FFFF`. VIA:n ligger kvar på `$4000`–`$400F`.

Den stora förändringen i koden: Arduino går *tri-state* (hög impedans) för alla adresser under `$4000`. När CPU:n läser från `$0200` är det SRAM-chippet som lägger ut data på bussen — inte Arduino. Jag har nu *tre enheter* som delar på databussen: SRAM, VIA och Arduino.

## Nya komponenter

Ett 62256 SRAM-chip på 32 KB — datorns första riktiga arbetsminne — plus en avkopplingskondensator och kopplingstrådar. Zero page, stack och variabler lämnar nu Arduinons mjukvaruemulering och hamnar i äkta kisel.

| Antal | Komponent |
|---|---|
| 1 | 62256 SRAM (32KB, DIP-28) |
| 1 | 100 nF keramisk kondensator (avkoppling SRAM) |
| – | Kopplingstråd (adress, data, kontroll) |
## 62256 SRAM — pinout

DIP-28-kapsel. 15 adresslinjer, 8 datalinjer, 3 kontrollsignaler. Här ser jag exakt hur kretsen ska vändas och vad varje pinne gör.
> [!NOTE] 🧩 62256 SRAM · se step9.html

■ Adressbuss ■ Databuss ■ Kontroll ■ Ström. Notch/pin 1-markering: uppåt. `/OE` = `GND` (alltid läs ut). `/CE` = U4D pin 11 (aktiv vid `$0000`–`$3FFF`).

## 74HC00 — pinout

DIP-14-kapsel. Alla fyra grindar används nu: U4A+U4B för VIA (steg 7), U4C+U4D för SRAM (steg 9).
> [!NOTE] 🧩 74HC00 · se step9.html

■ U4A (NOT `A15` → SRAM `/CE`) ■ U4B (`A15` NAND `A14` → VIA `/CS2`) ■ U4D (NAND) ■ U4C (NAND).

## Kopplingsschema

SRAM-chippet delar adress- och databuss med CPU, VIA och Arduino. Tre kontrollsignaler avgör vem som pratar.
![Steg 9 — SRAM 62256](schematics/steg-9.png)

## Kopplingar

Tabellerna täcker nu fyra kretsar plus LCD:n, uppdelade efter funktion: ström och kontroll, adressbuss, databuss och övrigt. SRAM:ns avkodning presenteras för sig, eftersom U4C + U4D i **74HC00**:an är det enda nya i detta steg.
### 62256 SRAM
| Pin | Signal | Kopplas till | Varför |
|---|---|---|---|
| 28 | `VDD` | +5V | Strömmatning — glöm inte 100nF avkoppling till GND |
| 14 | `VSS` | GND | Systemjord |
| 10–3, 25, 24, 21, 23, 2, 26, 1 | `A0–A14` | A0–A14 | 15 adresslinjer — SRAM behöver veta vilken byte CPU:n vill åt |
| 11–13, 15–19 | `D0–D7` | D0–D7 (databuss) | 8 datalinjer — SRAM läser/skriver data här |
| 20 | `/CE` | U4D pin 11 | Chip Enable — LÅG = SRAM aktivt. 74HC00 drar LÅG vid $0000–$3FFF |
| 22 | `/OE` | GND | Output Enable — alltid LÅG (SRAM kör alltid ut data vid läsning) |
| 27 | `/WE` | R/W (pin 34) | Write Enable — LÅG = skriv. CPU:ns R/W är LÅG vid skrivning |

### 74HC00 — adressavkodning för SRAM

U4A (NOT `A15`) och U4B (NAND för VIA) är oförändrade från steg 7. U4C blir NOT `A14`. U4D gör (NOT `A14`) NAND (NOT `A15`) → LÅG endast vid `$0000`–`$3FFF`.

U4A inverterar `A15`, U4C inverterar `A14`. U4D gör (NOT `A14`) NAND (NOT `A15`) → LÅG endast när `A14`=0 OCH `A15`=0. SRAM aktiveras alltså vid `$0000`–`$3FFF` och ingen annanstans. VIAn (U4B) aktiveras vid `$4000`–`$7FFF`. 

| Pin | Signal | Kopplas till | Varför |
|---|---|---|---|
| VIA-avkodning (U4A + U4B — oförändrad från steg 7) |  |  |  |
| 1, 2 | `A15` (in) | `A15` | Båda ingångarna till A15 → inverterare: ut = NOT A15 |
| 3 | NOT `A15` (ut) | U4B pin 4 + U4D pin 13 | NOT A15 → till U4B och U4D |
| 4 | NOT `A15` | U4A pin 3 (NOT A15) | NOT A15 |
| 5 | `A14` (in) | `A14` | (NOT A15) NAND A14 → LÅG vid $4000–$7FFF |
| 6 | VIA `/CS2` (ut) | VIA `/CS2` (pin 23) | VIA aktiveras vid $4000–$7FFF |
| SRAM-avkodning (U4C + U4D — nytt i steg 9) |  |  |  |
| 9, 10 | `A14` (in) | `A14` | Båda ingångarna till A14 → inverterare: ut = NOT A14 |
| 8 | NOT `A14` (ut) | U4D pin 12 | NOT A14 → U4D ingång |
| 12 | NOT `A14` | U4C pin 8 (NOT A14) | NOT A14 |
| 13 | NOT `A15` | U4A pin 3 (NOT A15) | (NOT A14) NAND (NOT A15) → LÅG endast vid $0000–$3FFF |
| 11 | SRAM `/CE` (ut) | SRAM `/CE` (pin 20) | SRAM aktiveras ENDAST vid $0000–$3FFF |
| 14 | `VCC` | +5V | Strömmatning |
| 7 | `GND` | GND | Systemjord |

### CPU, VIA och LCD

CPU, VIA och LCD är oförändrade från steg 7. Här är den kompletta kopplingslistan.

| Pin | Signal | Kopplas till | Varför |
|---|---|---|---|
| — ström och kontroll |  |  |  |
| 8 | `VDD` | +5V | Strömmatning |
| 21 | `VSS` | GND | Systemjord |
| 37 | `PHI2` | Arduino D2 | Klocka — 500 Hz fyrkantsvåg |
| 34 | `R/W` | Arduino D3, VIA pin 22, SRAM /WE | Read/Write — alla kretsar behöver veta |
| 40 | `/RESET` | Arduino D4, VIA pin 34 | Reset |
| 2 | `RDY` | +5V via 10kΩ | Ready — HÖG = kör |
| 4 | `/IRQ` | +5V via 10kΩ | Interrupt — avaktiverad |
| 6 | `/NMI` | +5V via 10kΩ | NMI — avaktiverad |
| 36 | `BE` | +5V via 10kΩ | Bus Enable — HÖG = bussar aktiva |
| 38 | `/SO` | +5V via 10kΩ | Set Overflow — avaktiverad |
| — adressbuss (delas med SRAM, VIA, Arduino) |  |  |  |
| 9–16 | `A0–A7` | Arduino A0–A7, SRAM A0–A7, VIA RS0–RS3 (A0–A3) | Låga adressbyte |
| 17–20, 22–25 | `A8–A15` | Arduino A8–A15, SRAM A8–A14, 74HC00 | Höga adressbyte + avkodning |
| — databuss (delas med SRAM, VIA, Arduino) |  |  |  |
| 26–33 | `D0–D7` | Arduino D22–D29, SRAM D0–D7, VIA D0–D7 | 8-bit data — 100Ω seriemotstånd |
| Övrigt |  |  |  |
| 7 | `SYNC` | Arduino D13 | Opcode fetch |
| — | BTN1 | Arduino D11 → GND | Klocksteg |
| — | BTN2 | Arduino D12 → GND | Instruktionssteg |
| VIA → LCD (oförändrat från steg 7) |  |  |  |
| VIA 2 | `PA0` | 4 (RS) | Register Select |
| VIA 4 | `PA2` | 6 (E) | Enable |
| VIA 10–17 | `PB0–PB7` | 7–14 (DB0–DB7) | 8-bit data |
| LCD — ström och kontrast |  |  |  |
| 1 | `VSS` | GND | Jord |
| 2 | `VDD` | +5V | Ström |
| 3 | `VO` | Potentiometer mittben | Kontrast |
| 5 | `R/W` | GND | Write-only |
| 15 | `A` | +5V via 220Ω | Belysning + |
| 16 | `K` | GND | Belysning − |

> [!NOTE] 🗺️ Minnestarta · se step9.html

## Arduino-kod

Det här är slutsteget — och den mest genomgripande förändringen av minnesemulatorn sedan steg 3. Jag kopplar in ett SRAM-chip (32 KB statiskt RAM) som tar över adresserna `$0000–$3FFF`. Arduino måste nu *tri-stata databussen* för alla SRAM- och VIA-adresser — tre enheter delar på samma 8 datalinjer, och bara en får prata i taget.
### Tre enheter, en buss

Databussen (`D0–D7`) är nu ansluten till fyra kretsar samtidigt: CPU, Arduino, SRAM och VIA. Adressavkodningen med 74HC00 avgör vem som svarar:

- `$0000`–`$3FFF` → `is_sram()` returnerar `true`. Arduino rör inte bussen. SRAM-chippet läser och skriver helt på egen hand.
- `$4000`–`$400F` → `is_via()` returnerar `true`. Arduino rör inte bussen. VIA-kretsen hanterar I/O.
- `$8000`–`$FFFF` → Arduino svarar med programkod och vektorer. SRAM och VIA är inaktiva (deras `/CE`- och `/CS2`-signaler är höga).

Det här är en *riktig datorarkitektur*: flera enheter på en delad buss, styrda av adressavkodningslogik. Exakt så här fungerar modernare datorer också — bara med fler kretsar och högre hastigheter.

### `is_sram()` — den nya gränsen

`is_sram()` är lika enkel som `is_via()`: returnerar `true` om adressen är högst `$3FFF`. I `read_mem()` och `write_mem()` kontrolleras SRAM före allt annat — om adressen är under `$4000` gör Arduino ingenting. `pulse()` har nu tre villkor: `!is_sram(a) && !is_via(a)` — Arduino driver bussen endast när varken SRAM eller VIA gör det.

### Två programfaser

Koden är uppdelad i två faser som demonstrerar att SRAM fungerar:

1. Fas 1 — VIA/LCD-text: samma 4-radersprogram som tidigare. Visar att VIA och LCD fungerar med SRAM inkopplat. Programmet väntar sedan på ett knapptryck.
1. Fas 2 — Fibonacci: ett 6502-program som beräknar Fibonacci-sekvensen (0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233) och visar varje tal på LCD:n. Programmet använder zero page (`$00–$02`) för att lagra variabler — minnesadresser som nu ligger i det fysiska SRAM-chippet, inte i Arduinons array.

### Vad som är nytt jämfört med steg 7–8

SRAM-chip, utökad med 74HC00-avkodning (U4C+U4D för SRAM). I koden: `is_sram()`, utökade villkor i `pulse()`/`read_mem()`/`write_mem()`, tvåfasigt programflöde i `setup()`, och Fibonacci-programmet byggt med ca65. `ram[1024]` från tidigare steg är borta — den har ersatts av äkta kisel.
> [!NOTE] 📦 Arduino-kod — step9.inc · 304 rader · se step9.html

## 6502-programmet — Fibonacci i assembler

Det här programmet är intressantare än hello-programmet från steg 8 — det *beräknar* något. Fibonacci-sekvensen är en av matematikens mest berömda talserier: varje tal är summan av de två föregående. 0, 1, 1, 2, 3, 5, 8, 13, 21… och så vidare. Programmet räknar ut nästa tal, visar det på LCD:ns andra rad, pausar en stund, och gör om. Eftersom 6502:an är en 8-bitars processor ryms bara tal upp till 255 — när Fibonacci passerar 233 och försöker beräkna 377 (som är för stort) fångar programmet upp overflödet och börjar om från noll. LCD:n visar "Fib:" på första raden och det aktuella talet — 0, 1, 1, 2, 3… — på andra raden.

Strukturen känns igen från steg 8: 
1. konstanter överst
2. kod från `$8000`
3. subrutiner
4. strängdata och vektorsegment. 

Det nya är att programmet använder *zero page* — adresserna `$00` till `$02` — för att lagra Fibonacci-variablerna. Zero page är 6502:ans snabbaste minnesområde eftersom adresseringsläget bara kräver en byte. Här ligger F(n-2) på `$00`, F(n-1) på `$01`, och det nyberäknade F(n) på `$02`. Det är dessa adresser som i steg 9 ligger i det fysiska SRAM-chippet — inte i Arduinons array. När programmet läser `$00` eller skriver till `$02` är det äkta kisel som svarar.

Själva Fibonacci-beräkningen är förvånansvärt kort — bara en handfull instruktioner. Programmet rensar carry-flaggan med `CLC`, laddar F(n-2) med `LDA $00`, adderar F(n-1) med `ADC $01`, och kontrollerar om additionen spillde över med `BCC no_wrap`. Om carry är satt betyder det att talet blev större än 255 — då nollställs serien och loopen börjar om. Annars sparas resultatet, variablerna skiftas framåt (det gamla F(n-1) blir nya F(n-2), det nya F(n) blir F(n-1)), och talet visas på LCD:n. Varje varv i loopen avslutas med `JMP fib_loop` — tillbaka till början, nästa tal.

Den mest imponerande subrutinen är `show_num`. Den omvandlar ett 8-bitars tal till 1–3 ASCII-siffror genom att räkna hur många hundratal, tiotal och ental som ryms — en algoritm som liknar den jag lärde mig i grundskolan. Först subtraheras 100 upprepade gånger för att få fram hundratalet, sedan 10 för tiotalet, och resten är entalet. Varje siffra konverteras till ASCII genom att addera `'0'` (tecknet noll har ASCII-värdet 48). Efter utskriften skrivs två mellanslag — ett smart knep för att sudda bort gamla siffror när "144" följs av "5". Utan dem skulle displayen visa "544" där fjorton blev fem.
#### Så här visas tex 144

Jag följer `show_num` när A-registret innehåller 144 — talet som visas efter "Fib:":

1. Hundratal: 144 − 100 = 44, och X räknas upp till 1. 44 < 100, så subtraktionen slutar. X (1) + `'0'` = `'1'` → skriv ut.
1. Tiotal: 44 − 10 = 34 (X=1) → 24 (X=2) → 14 (X=3) → 4 (X=4). 4 < 10, klart. X (4) + `'0'` = `'4'` → skriv ut.
1. Ental: resten är 4 → `'4'` → skriv ut.
1. Städning: två mellanslag suddar bort resten av raden — nästa tal (5) skriver över utan att lämna kvar "1445".
### Nya instruktioner i det här programmet

Från steg 8 känner jag redan `LDA`, `STA`, `JSR`, `RTS`, `LDX`, `INX`, `BEQ` och `JMP`. Det här är vad som är nytt:

| Instruktion | Exempel i programmet | Vad CPU:n gör |
|---|---|---|
| CLC / SEC | före ADC / SBC | Rensar/sätter carry-flaggan — förbereder addition respektive subtraktion. |
| ADC $01 | F(n) = F(n-2) + F(n-1) | Adderar ett minnesvärde till A *inklusive* carry — grunden i Fibonacci-loopen. |
| BCC no_wrap | efter ADC | Hoppar om carry = 0, dvs om summan rymdes i 8 bitar. Annars börjar serien om. |
| SBC #100 | hundratal i show_num | Subtraherar 100 från A (med carry) — upprepas tills resten < 100. |
| CMP #100 | "är A >= 100?" | Jämför A med ett värde utan att ändra A — flaggorna avgör sedan om jag hoppar. |
| PHA / PLA | spara resten i show_num | Stacken: sparar A för att komma ihåg den och tar tillbaka den senare. |
| TXA | räknaren → A | Kopierar X-registret till A så att räknaren kan skrivas ut. |
| DEX / DEY | loopräknare | Minskar X/Y med 1 — i delay och i tiotal-uträkningen. |
| BNE | loopar i show_num | Hoppar om resultatet inte är noll — avslutar räknar-looparna. |
| LDA $00 / STA $02 | F-variablerna | Zero page-adressering — variabler i $00–$02 nås med bara en byte. |

> [!NOTE] 📦 6502-programmet — program_fib.asm · 235 rader · se step9.html

Komplett assembler-kod: `Mega_2560_6502/asm/program_fib.asm`. Byggs med `ca65 + ld65 → program_fib.h → inkluderas av step9.inc`.

## Exempel på körning

När jag laddat upp koden körs två faser efter varandra. Först VIA/LCD-programmet, sedan — efter ett knapptryck — Fibonacci-programmet som använder det fysiska SRAM-chippet.

Seriemonitor — fas 1
```
Steg 9 — 62256 SRAM
Fas 1: Laddar 4-raders VIA/LCD-program...
Kor 1000 cykler (tyst)...
Fas 1 klar — vantar pa knapptryck...
```

LCD-displayen — fas 1

Jag trycker på valfri knapp för att gå vidare till Fibonacci:

Seriemonitor — fas 2
```
>>> Knapp 1 tryckt! <<<
Laddar Fibonacci-program...
Fibonacci-program kört.
R $FFFC
R $FFFD
R $8000  ← OPCODE
...
R $0000  ← SRAM (F(n-2))
R $0001  ← SRAM (F(n-1))
W $0002  ← SRAM (F(n))
W $4000  ← VIA: 31  (siffran 1)
W $4000  ← VIA: 34  (siffran 4)
W $4000  ← VIA: 34  (siffran 4)
```

LCD-displayen — mitt i Fibonacci

`144` är det tolfte Fibonacci-talet. Strax därefter kommer `233` — det sista som ryms i 8 bitar — sedan wrappar det.

Varje nytt Fibonacci-tal dyker upp med en kort paus. När talet når 233 (det sista som ryms i en 8-bitars byte) wrappar det tillbaka till 0 och sekvensen börjar om. Under tiden använder 6502-programmet zero page-adresser (`$00`, `$01`, `$02`) för att lagra F(n-2), F(n-1) och F(n) — adresser som nu ligger i det fysiska SRAM-chippet, inte i Arduinons mjukvaruemulerade RAM. I seriemonitor syns dessa som `R $0000` och `W $0002` med `← SRAM`-markering.

Jag har byggt en dator med tre kretsar som delar på samma buss, äkta SRAM för arbetsminne, en VIA för I/O, och en 6502-processor som kör mina egna assembler-program. Från en blinkande lysdiod till Fibonacci på LCD — hela resan på nio steg. Jag satt och tittade på 144 ett bra tag — det var mitt eget minne som räknat.
## Så här felsöker jag

Här är några saker jag kontrollerar:

- Läser CPU:n `$0000` vid reset? Då är SRAM-chipets `/OE` inte `GND`. Utan `/OE`=LÅG driver SRAM inte ut data vid läsning.
- Databuss-krockar? Då får Arduino INTE sätta `DDRA`=`0xFF` vid SRAM-adresser. Jag kontrollerar att `is_sram()` returnerar rätt.
- Skriver SRAM inte? Då måste `/WE` gå till `R/W` (pin 34). Jag kontrollerar att `/CE` är avkodad korrekt via 74HC00.
- Glitchar på bussen? 62256 är snabbare än Arduino. Jag ser till att `/CE` och `/WE` är stabila innan databussen läses.
