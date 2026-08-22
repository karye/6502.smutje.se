<!-- Handredigerad: jag-röst. Kör ej html2md på denna fil. -->
# Assembler-bygge med ca65

Att skriva hex-bytes fungerar, men det är som att stava ett ord i stället för att säga det. Med assembler får koden ett språk — labels, subrutiner och kommentarer.

## Mål

Hittills har jag skrivit 6502-program som hårdkodade hex-bytes direkt i Arduinons `setup()`. Det fungerar — men det är inte hållbart i längden. Nu tar jag steget till *riktig assembler* med ca65, en professionell 6502-assembler från cc65-verktygslådan.

Ingen ny hårdvara. Samma kopplingar som steg 7. Allt nytt finns i byggkedjan.

Arbetsflödet blir:

1. Skriv 6502-kod i `asm/program_hello.asm` — med labels, subrutiner och kommentarer
1. PlatformIO kör `ca65` + `ld65` före C++-kompileringen
1. Resultatet (`program_hello.bin`) konverteras till en C-header (`program.h`)
1. `setup()` läser `PROGRAM[]` och laddar in i minnesemulatorn

Jag redigerar en *läsbar assembler-fil* — inte en lång rad `write_mem()`-anrop.

## Nya verktyg

Ingen ny hårdvara — bara mjukvara. Dessa två program installerar jag en gång, sedan bara rullar det på.
| Verktyg | Installation |
|---|---|
| ca65 / ld65 | `sudo apt install cc65` (Linux) eller ladda ner från [cc65.github.io (Windows) |
| Python 3 | Redan installerat med PlatformIO |

## Byggkedjan

Så här går min assembler-kod från en textfil till körbar kod i processorn. Tre steg, helt automatiskt — PlatformIO sköter allt.

## PlatformIO-byggskript

`scripts/build_asm.py` är en *pre-build hook* som PlatformIO kör före C++-kompileringen. Den anropar ca65, ld65 och bin2h.py i sekvens, konfigurerad i `platformio.ini`:
```
extra_scripts = pre:scripts/build_asm.py
```
> [!NOTE] 📦 Byggskriptet — build_asm.py · 52 rader · se step8.html

### bin2h.py — binär till C-header

Konverterar `program_hello.bin` till `PROGRAM[]` (en `PROGMEM`-array) som `main.cpp` inkluderar.
> [!NOTE] 📦 Byggskriptet — bin2h.py · 39 rader · se step8.html

## Så här använder jag det

1. Redigera assembler-koden:
```
nano asm/program_hello.asm
```

2. Bygg och ladda upp (Windows-maskinen):
```
pio run -e step8 -t upload -t monitor
```

Viktigt: `platformio.ini` måste ha `build_src_filter = +<*> -<program.asm>` för att förhindra att AVR-assemblern försöker kompilera 6502-koden.

Vid varje bygge körs `ca65` → `ld65` → `bin2h.py` automatiskt. Ändrar jag `program_hello.asm` byggs allt om.

## 6502-programmet i assembler

Det här programmet körs på 6502-processorn — inte på Arduino. Det är skrivet i ren assembler för W65C02 och pratar direkt med VIA-kretsen som i sin tur styr LCD-displayen. När processorn startar efter reset hoppar den till `reset`-labeln, initierar VIA:ns portar som utgångar, skickar en standardsekvens av kommandon för att väcka LCD:n i 8-bitarsläge, och går sedan in i huvudloopen `hello`. Där skrivs två rader text ut — "=== 6502 VIA LCD ===" och "Hello from W65C02!" — displayen rensas, och allt börjar om. En oändlig loop av text, rensa, text, rensa.

Programmet är uppbyggt i lager. Överst definieras fyra konstanter — `VIA_ORB`, `VIA_ORA`, `VIA_DDRB` och `VIA_DDRA` — som ger mänskliga namn åt VIA-kretsens registeradresser. Sedan följer själva koden i segmentet `CODE` som placeras från adress `$8000`. Där finns `reset` (startpunkten), `hello` (huvudloopen), och två subrutiner: `lcd_command` och `lcd_data`. Efter koden ligger strängdatan — två rader text, var och en avslutad med en nolla som signalerar att strängen är slut. Sist kommer vektorsegmentet där `.word reset` på adress `$FFFC` talar om för processorn var programmet börjar.

De två subrutinerna är programmets verkliga arbetshästar. `lcd_command` skickar ett kommando till LCD:n: den lägger kommandobyten på PORTB, sätter `RS`=0 och `E`=1, och drar sedan `E` till 0 — den fallande flanken får LCD:n att läsa. `lcd_data` gör exakt samma sak, men med `RS`=1 för att signalera att byten är ett tecken, inte ett kommando. Båda anropas med `JSR` (Jump to Subroutine) och återvänder med `RTS` — 6502:ans motsvarighet till funktionsanrop. Strängarna skrivs ut tecken för tecken i en loop: X-registret räknar genom strängen, `LDA line1,x` hämtar nästa tecken, och när en nolla dyker upp vet programmet att strängen är slut.

Det fina med att ha programmet i en separat `.asm`-fil är att jag kan ändra och experimentera utan att röra Arduino-koden. Jag byter ut texten i `line1` mot mitt eget namn, kör `pio run -e step8 -t upload`, och inom sekunder visar LCD:n min nya text. Assemblerkoden är kommenterad på svenska så att varje instruktion förklarar sig själv — jag läser den som en receptbok, rad för rad, så förstår jag exakt vad processorn gör i varje ögonblick.
> [!NOTE] 📦 6502-programmet — program_hello.asm · 151 rader · se step8.html

Komplett assembler-kod: `Mega_2560_6502/asm/program_hello.asm`.

## Arduino-kod

Steg 7:s största svaghet var att 6502-programmet byggdes som hundratals `write_mem()`-anrop i C++-koden. Det fungerar — men det är plågsamt att redigera, svårt att läsa, och omöjligt att felsöka. Steg 8 löser detta med en *professionell assembler-kedja*: jag skriver läsbar 6502-kod i en `.asm`-fil, och PlatformIO bygger om den automatiskt vid varje uppladdning.

### Från assembler till header — byggkedjan i detalj

När jag kör `pio run` händer tre saker innan C++-koden kompileras:

1. ca65 — assemblerar `program_hello.asm` till en objektfil (`.o`). Översätter mnemonics som `LDA #$FF` till maskinkod (`A9 FF`) och löser lokala labels.
1. ld65 — länkaren placerar koden på rätt adresser enligt `program.cfg`. Koden läggs på `$8000`, vektorerna på `$FFFA`. Resultatet är en binärfil på exakt 2054 bytes (2048 kod + 6 vektorer).
1. bin2h.py — konverterar binärfilen till en C-header med en `PROGMEM`-array. Arduinons flash-minne är begränsat — `PROGMEM` gör att programmet ligger kvar i flash och inte kopieras till RAM.

Hela kedjan styrs av `scripts/build_asm.py`, en pre-build hook som PlatformIO anropar automatiskt. Ändrar jag `.asm`-filen byggs allt om.

### Arduino-koden — bara `setup()` ändras

`pulse()`, `loop()` och hela minnesemulatorn är oförändrade från steg 7. Skillnaden sitter i `setup()`: istället för hundratals `write_mem()` loopar jag nu igenom `PROGRAM[]` och kopierar:

- Kod: `PROGRAM_SIZE - 6` bytes kopieras till `$8000` och uppåt. `pgm_read_byte()` läser från PROGMEM (flash).
- Vektorer: de sista 6 byten kopieras till `$FFFA`–`$FFFF`. Där finns reset-vektorn som talar om att programmet börjar på `$8000`.

### 6502-programmet — läsbar assembler

`program_hello.asm` är samma 2-radersprogram som steg 7, men nu med labels (`reset:`, `hello:`), subrutiner (`lcd_command:`, `lcd_data:`), och nollterminerade strängar (`.byte "...", 0`). `JSR` (Jump to Subroutine) och `RTS` (Return from Subroutine) använder stacken för att hålla reda på återhoppsadressen — precis som funktionsanrop i högnivåspråk. Att läsa assemblerkoden är som att läsa en receptbok: varje rad är ett steg, och tillsammans berättar de exakt vad processorn ska göra.

### Vad som är nytt jämfört med steg 7

Ingen ny hårdvara. I koden: `#include "program_hello.h"`, `PROGRAM[]`, `PROGRAM_SIZE`, och `pgm_read_byte()`. I byggkedjan: `ca65`, `ld65`, `bin2h.py`, `build_asm.py`. Det här är samma verktyg som användes för att bygga spel till NES och program till Apple II — professionella verktyg för en hobbyprocessor.
> [!NOTE] 📦 Arduino-kod — step8.inc · 125 rader · se step8.html

> [!NOTE] 🗺️ Minnestarta · se step8.html

## Exempel på körning

När jag bygger och laddar upp ser jag byggkedjan arbeta i terminalen:
```
pio run -e step8 -t upload -t monitor
```
```
[build_asm] Assemblerar .../asm/program_hello.asm …
[build_asm] OK — 2054 bytes → .../program_hello.h
Processing step8 (platform: ...)
...
Steg 8 — assembler-bygge med ca65
Programstorlek: 2054 bytes
Program + vektorer laddat.
```

Seriemonitor — VIA-aktivitet
```
W $4002  ← VIA: FF  (DDRB)
W $4003  ← VIA: FF  (DDRA)
...
W $4000  ← VIA: 3D  (=)
W $4000  ← VIA: 3D  (=)
...
```

LCD-displayen — två rader text

Seriemonitor visar samma VIA- och LCD-aktivitet som i steg 7 — men med en avgörande skillnad: jag har inte skrivit en enda `write_mem()`. Programmet kommer från `program_hello.asm`, en ren assembler-fil med labels, subrutiner och kommentarer.

Jag gör ett test: jag ändrar texten i `line1:` från "=== 6502 VIA LCD ===" till "Hej varlden!" och kör `pio run -e step8 -t upload -t monitor` igen. Hela kedjan körs om — ca65, ld65, bin2h, kompilering, uppladdning — och LCD:n visar min nya text. Iterationstiden är sekunder, inte minuter. Första gången kedjan rullade ihop på egen hand kändes det som att få en assistent.

### Så här provar jag

- Jag ändrar texten i `line4` (rad 4) till mitt eget namn och kör `pio run -e step8 -t upload` — jag ser hela kedjan kompilera om.
- Jag lägger till ett utropstecken i slutet av `line2` och tittar på vilka bytes som ändras i `.bin`-filen.

## Så här felsöker jag

Här är några saker jag kontrollerar:

```
sudo apt install cc65
```
- ca65: command not found? Då installerar jag cc65:
```
pio run -e step8
```
- "No such file: program.h"? Då måste första bygget generera den. Jag kör två gånger om det krävs.
- Gör CPU:n inget? Då kontrollerar jag att `write_mem()`-loopen faktiskt kopierar `PROGRAM[]` till `program[]`. Jag loggar med `Serial.print()`.

## Vad händer härnäst?

Nu bygger jag program som ett proffs — ca65 och ld65 gör det tunga jobbet. I nästa steg får datorn riktigt minne: ett SRAM-chip ersätter Arduinons arrayer.
