# Adressbuss och reset

Första gången jag såg en 6502 leta upp sin egen startadress i seriemonitorn var ögonblicket då allt föll på plats. Den här processorn vet exakt var den ska börja — vi ska bara få se det hända.

## Mål

I steg 1 gav vi processorn ström och klocka. Nu ska vi ge den kontroll över adressbussen — 16 ledningar som talar om var i minnet CPU:n vill läsa eller skriva.

Alla 6502-processorer startar på samma sätt: efter reset läser de sk reset-vektorn på adress $FFFC och $FFFD för att få reda på var programmet börjar. Det är inbyggt i kisel — ingen mjukvara, ingen konfiguration. Genom att koppla in adressbussen och en reset-signal kan vi se detta hända i seriemonitor.

Vi kopplar också in /RESET (pin 40) till Arduino D4. När vi håller RESB låg i minst 2 klockcykler nollställs CPU:ns interna register. När vi sedan släpper den (HÖG) börjar CPU:n köra från reset-vektorn.

Adressbussen läser vi via Arduinons analoga portar — PORTF (A0–A7) och PORTK (A8–A15). Det ger oss 16 bitar i två registerläsningar.

## Nya komponenter

Det här steget kräver inga nya elektroniska komponenter — bara kopplingstrådar. Sexton för adresslinjerna A0–A15 och en extra för reset-signalen, eftersom vi nu tar kontroll över CPU:ns uppstart via /RESET.
| Antal | Komponent |
|---|---|
| 17 | Kopplingstrådar (16 adress + 1 RESB) |

## Kopplingsschema

Schemat visar den nya kopplingen: 16 adresslinjer som löper från CPU:ns A0–A15 till Arduinons analoga portar A0–A15, plus reset-signalen från D4 till /RESET. Allt annat är oförändrat från steg 1.
![Steg 2 — adressbuss och reset](schematics/steg-2.png)

## Kopplingar

Tabellen visar alla kopplingar från steg 1 plus de nya: adressbussens 16 linjer och reset-signalen. Adresslinjerna är grupperade i två byte — A0–A7 till PORTF och A8–A15 till PORTK — eftersom Arduino läser dem som två 8-bitars register.
| Pin | Signal | Kopplas till | Varför |
|---|---|---|---|
| 8 | `VDD` | +5V | Strömmatning — CPU:ns driftspänning |
| 21 | `VSS` | GND | Systemjord — sluten krets |
| 37 | `PHI2` | **Arduino D2** | Klockingång — Arduino skickar fyrkantsvåg |
| 2 | `RDY` | +5V via 10kΩ | Ready — HÖG = CPU får köra. Utan denna stannar CPU:n |
| 4 | `/IRQ` | +5V via 10kΩ | Interrupt request — HÖG = inget avbrott |
| 6 | `/NMI` | +5V via 10kΩ | Non-maskable interrupt — måste vara HÖG |
| 36 | `BE` | +5V via 10kΩ | Bus Enable — HÖG = bussarna aktiva. Utan denna är CPU:n bortkopplad! |
| 38 | `/SO` | +5V via 10kΩ | Set Overflow — avaktiverad |
| 40 | `/RESET` | **Arduino D4** | Kontrollerad reset — Arduino håller CPU:n i reset tills vi är redo |
| 9–16 | `A0–A7` | **Arduino A0–A7** | Låga adressbyte — läses via PORTF |
| 17–20, 22–25 | `A8–A15` | **Arduino A8–A15** | Höga adressbyte — läses via PORTK |

## Arduino-kod

I steg 1 gav vi processorn liv. Nu ska vi lyssna på vad den säger. Koden i detta steg gör något fundamentalt: den läser av processorns adressbuss — 16 ledningar som tillsammans talar om var i minnet CPU:n vill läsa — och skriver ut adressen i seriemonitor. Det är som att koppla in en logikanalysator, fast gratis.

### Portregister — att läsa 8 pinnar på en gång

Arduino Mega 2560 har 54 digitala pinnar, men att läsa dem en och en med digitalRead() är alldeles för långsamt. Varje anrop tar flera mikrosekunder — och med 16 adresslinjer skulle vi tappa synkroniseringen med processorn direkt. Lösningen är portregister — specialregister som läser eller skriver 8 pinnar samtidigt i en enda maskininstruktion:

- DDRF = 0x00 — sätt alla 8 pinnar på PORTF som ingångar. Motsvarar 8 × pinMode() men går på några nanosekunder.
- PINF — läs alla 8 pinnar på en gång. Varje bit i byten motsvarar en pinne. Här är det CPU:ns A0–A7.
- PINK — läser CPU:ns A8–A15 på samma sätt.

För att få ihop en 16-bitars adress skiftar vi ihop de två byten:
```
uint16_t addr = (PINF << 0) | (PINK << 8);
```

Resultatet är ett tal mellan 0 och 65 535 — processor ns fullständiga adressrymd.

### Reset-sekvensen — vad som händer när CPU:n vaknar

Koden gör tre saker i setup():

1. Håller CPU:n i reset — RESB = LOW i minst 2 klockcykler (vi kör 5 för säkerhets skull). Under denna tid är CPU:ns interna register nollställda och bussarna är tri-state.
1. Släpper reset — RESB = HIGH. Detta är ögonblicket CPU:n vaknar. Den gör inget val, fattar inget beslut — den läser helt enkelt reset-vektorn på adress $FFFC och $FFFD.
1. Läser och loggar — i loop() läser vi adressbussen i varje klockcykel och skriver ut den. Första två raderna kommer alltid att vara $FFFC och $FFFD — reset-vektorn. Det är processorns sätt att fråga "var ska jag börja?".

### Vad som är nytt jämfört med steg 1

Klockan har höjts från 1 Hz till 500 Hz — för snabbt för att se med ögat, men lagom för att läsa adressbussen i seriemonitor. pulse() har förenklats (ingen lysdiod denna gång) och loop() gör nu det tunga jobbet: läsa portregister, skifta ihop adressen, skriva ut. setup() har fått en reset-sekvens — den viktigaste nyheten i detta steg.
> [!NOTE] 📦 Arduino-kod — step2.inc · 68 rader · se step2.html

## Exempel på körning

När du öppnar seriemonitor (115200 baud) efter uppladdning ser du ungefär detta:

Seriemonitor (115200 baud)
```
Steg 2 — adressbuss och reset
R $FFFC
R $FFFD
R $8000
R $8001
R $8002
R $8003
Klar — nu loopar vi:
R $8004
R $8005
R $8006
...
```

Varje rad börjar med R (Read — CPU:n läser) följt av $ och en hexadecimal adress. De två första raderna är alltid $FFFC och $FFFD — reset-vektorn. Processorn frågar "var ska jag börja?" och svaret (som just nu är skräp eftersom databussen inte är inkopplad) leder den till någon adress — ofta $8000 eller $0000 beroende på vad som ligger på de flytande datalinjerna.

Lägg märke till att $FFFC och $FFFD bara dyker upp en gång — efter reset. Sedan räknar adresserna uppåt när CPU:n hämtar instruktioner. Utan databuss (steg 3) kan CPU:n inte hämta riktiga opcodes, men adressbussen fungerar redan — och det är precis vad vi ville bevisa.

### Prova själv

- Koppla en lysdiod med 220 Ω från A0 (pin 9) till GND — A0 växlar på varannan adress, så dioden blinkar i takt med att adresserna räknas.

## Om det inte fungerar

Här är några saker att kontrollera:

- Ser du bara 0 eller 1? CPU:n är fortfarande i reset eller BE är LÅG. Kontrollera att RESB går HÖG efter reset-sekvensen, och att BE har +5V.
- Ser du FFFC men inte FFFD? Någon adresslinje är felkopplad. Dubbelkolla A0–A15 en och en.
- Skräptecken i seriemonitor? Baud rate måste vara 115200. Kontrollera platformio.ini om du använder PlatformIO. 

## Vad händer härnäst?

Processorn kan nu berätta var den vill läsa. I nästa steg får den också svar — vi kopplar in databussen.
