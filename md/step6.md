<!-- Handredigerad: jag-röst. Kör ej html2md på denna fil. -->
# Eget 6502-program

Det är en speciell känsla första gången en dator jag byggt själv kör ett program jag skrivit själv. Nio bytes räcker för att få den känslan.

## Mål

Datorn är nu komplett — CPU, minne, LCD, knappar. Men den kör bara `NOP`:ar. Dags att skriva mitt första egna 6502-program!

Jag skriver en enkel räknare: öka X-registret från 0 till 255, spara värdet på adress `$0200`, och loopa. Programmet är bara 9 bytes stort — men det innehåller allt ett riktigt program behöver: initiering, beräkning, minnesskrivning och loop.

6502-program skrivs i *maskinkod* — varje instruktion är 1–3 bytes. `LDX #$00` är två bytes (`$A2 $00`), `INX` är en byte (`$E8`), `STX $0200` är tre bytes (`$8E $00 $02`). Jag skriver in dessa bytes direkt i Arduinons minnesarray, och CPU:n exekverar dem som ett riktigt program.

Ingen ny hårdvara — samma koppling som steg 5. Allt nytt finns i mjukvaran.

## Nya komponenter

Inga nya komponenter — samma koppling som steg 5. Allt nytt i det här steget finns i mjukvaran: det första egna 6502-programmet.

## Kopplingar

Samma fysiska koppling som steg 5 — ingenting har flyttats. Tabellen är med för fullständighet så att jag kan följa varje signal, men det enda som ändras i detta steg är programmet i `setup()`.
| Pin | Signal | Kopplas till | Varför |
|---|---|---|---|
| 8 | `VDD` | +5V | Strömmatning — CPU:ns driftspänning |
| 21 | `VSS` | GND | Systemjord — sluten krets |
| 37 | `PHI2` | Arduino D2 | Klockingång — Arduino skickar fyrkantsvåg |
| 2 | `RDY` | +5V via 10kΩ | Ready — HÖG = CPU får köra |
| 4 | `/IRQ` | +5V via 10kΩ | Interrupt request — HÖG = inget avbrott |
| 6 | `/NMI` | +5V via 10kΩ | Non-maskable interrupt — måste vara HÖG |
| 36 | `BE` | +5V via 10kΩ | Bus Enable — HÖG = bussarna aktiva |
| 38 | `/SO` | +5V via 10kΩ | Set Overflow — avaktiverad |
| 40 | `/RESET` | Arduino D4 | Kontrollerad reset |
| 9–16 | `A0–A7` | Arduino A0–A7 | Låga adressbyte — läses via PORTF |
| 17–20, 22–25 | `A8–A15` | Arduino A8–A15 | Höga adressbyte — läses via PORTK |
| 34 | `R/W` | Arduino D3 | HÖG = CPU läser, LÅG = CPU skriver |
| 33 | `D0` | Arduino D22 via 100Ω | Databit 0 (LSB) |
| 32 | `D1` | Arduino D23 via 100Ω | Databit 1 |
| 31 | `D2` | Arduino D24 via 100Ω | Databit 2 |
| 30 | `D3` | Arduino D25 via 100Ω | Databit 3 |
| 29 | `D4` | Arduino D26 via 100Ω | Databit 4 |
| 28 | `D5` | Arduino D27 via 100Ω | Databit 5 |
| 27 | `D6` | Arduino D28 via 100Ω | Databit 6 |
| 26 | `D7` | Arduino D29 via 100Ω | Databit 7 (MSB) |
| 7 | `SYNC` | Arduino D13 | HÖG = CPU:n hämtar ny opcode |
| LCD 16×2 (parallell 4-bit) |  |  |  |
| 1 | `VSS` | GND | Jord |
| 2 | `VDD` | +5V | Strömmatning |
| 3 | `VO` | Potentiometer mittben | Kontrast — sidoben till +5V och GND |
| 4 | `RS` | Arduino D5 | Register Select — 0 = kommando, 1 = data |
| 5 | `R/W` | GND | Alltid skrivläge |
| 6 | `E` | Arduino D6 | Enable — Arduino pulserar för att skicka data |
| 11 | `DB4` | Arduino D10 | Data bit 4 — omvänd ordning! |
| 12 | `DB5` | Arduino D9 | Data bit 5 |
| 13 | `DB6` | Arduino D8 | Data bit 6 |
| 14 | `DB7` | Arduino D7 | Data bit 7 |
| 15 | `A` | +5V via 220Ω | Bakgrundsbelysning + |
| 16 | `K` | GND | Bakgrundsbelysning − |

## Kopplingsschema

Samma hårdvara som steg 5. LCD:n kopplas direkt till Arduino — `RS`→`D5`, E→`D6`, `DB4–DB7`→`D10`,`D9`,`D8`,`D7` (omvänd ordning).
![Steg 6 — LCD kopplad till Arduino](schematics/steg-6.png)

> [!NOTE] 🗺️ Minnestarta · se step6.html

## Arduino-kod

Datorn fungerar — men den har hittills bara kört `NOP`:ar. Nu skriver jag mitt första egna 6502-program. Det är bara 9 bytes stort, men det innehåller allt ett riktigt program behöver: initiering, beräkning, minnesskrivning och en loop. CPU:n kommer att räkna från 0 till 255 i en oändlig loop — och jag kommer att kunna följa varje steg.

Lägg märke till: programmet körs på 6502-processorn — Arduino laddar bara in byten i sin minnesarray och genererar klockan. Precis som tidigare är Arduino minnesemulator, inte exekverare.

### 6502-maskinkod — så här ser ett program ut

En 6502-instruktion är 1–3 bytes lång. Första byten är alltid *opcode* — en siffra som talar om för CPU:n vad den ska göra. Resten är *operander* — data eller adresser som instruktionen behöver. Vårt program har fyra instruktioner:
| Instruktion | Bytes | Vad CPU:n gör |
|---|---|---|
| LDX #$00 | A2 00 | Ladda X-registret med 0. X är min räknare. `#` betyder "omedelbart värde" — använd siffran 0, inte en minnesadress. Den här instruktionen körs bara en gång. |
| INX | E8 | Öka X med 1. En enda byte, ingen operand. Det här är loopens startpunkt — hit hoppar jag tillbaka. |
| STX $0200 | 8E 00 02 | Spara X-registrets värde till minnesadress $0200. Notera: låg byte ($00) kommer före hög byte ($02). 6502 är *little-endian*. |
| JMP $8002 | 4C 02 80 | Hoppa tillbaka till INX. Återigen: låg byte först ($02), hög byte sen ($80). Utan denna instruktion skulle CPU:n bara fortsätta rakt fram genom minnet. |

### Arduino-koden — bara `setup()` ändras

`pulse()`, `loop()` och LCD-koden är helt oförändrade från steg 5. Det enda som ändras är programladdningen i `setup()`. Istället för en enda `write_mem(0x8000, 0xEA)` skriver jag nu 9 bytes — en i taget, på stigande adresser från `$8000`. Varje `write_mem()` placerar en byte i Arduinons `program[]`-array, och CPU:n kommer att läsa dem som instruktioner.

### Vad jag ser på LCD:n

När programmet körs kan jag med Knapp 2 (instruktionssteg) följa programflödet: `$8000` (`LDX`) → `$8002` (`INX`) → `$8003` (`STX`) → `$8006` (`JMP`) → `$8002` (`INX` igen). Varje varv genom loopen ökar värdet på adress `$0200` — och jag kan se det på LCD:ns rad 1 som `D:$01`, `D:$02`, `D:$03`… hela vägen upp till `D:$FF` (255). Sedan wrappar X-registret till 0 och loopen börjar om.
> [!NOTE] 📦 Arduino-kod — step6.inc · 131 rader · se step6.html

## Exempel på körning

När jag laddat upp koden och trycker på Knapp 2 (instruktionssteg) ser jag programmet rulla genom adresserna — både i seriemonitor och på LCD-displayen:

Seriemonitor — efter ett varv i loopen
```
Steg 6 — Räknarprogram
Program: LDX #$00 · INX · STX $0200 · JMP $8002

R $FFFC
R $FFFD
R $8000
R $8001
R $8002
R $8003
R $8004
R $8005
W $0200
R $8006
R $8007
R $8008
R $8002  ← tillbaka till INX
...
```

LCD-displayen — mitt i loopen

Adress `$0200` håller räknarens värde. Efter 5 varv genom loopen visar `D:$05`.

För första gången ser jag ett W i loggen! CPU:n *skriver* till adress `$0200` — det är `STX $0200` som sparar X-registrets värde. På LCD:ns rad 1 ser jag `D:$01`, `D:$02`, `D:$03`… räknaren ökar för varje varv. När den når `D:$FF` (255) wrappar den till `D:$00` — X-registret är bara 8 bitar brett.

Programmet är ynka 9 bytes, men det innehåller redan alla mönster jag behöver för att skriva större program: initiering (`LDX`), beräkning (`INX`), lagring (`STX`) och kontrollflöde (`JMP`). Jag har just skrivit mitt första 6502-program! Nio bytes — och jag kunde inte sluta titta på loggen.

### Så här provar jag

- Jag låter räknaren börja på 5: jag ändrar `LDX #$00` till `LDX #$05` i `step6.inc` och laddar om.
- Jag byter `INX` mot `DEX` och ser räknaren gå baklänges — den wrappar till 255 efter noll.

## Så här felsöker jag

Här är några saker jag kontrollerar:

- Kör CPU:n `NOP` istället? Då har jag glömt att ladda det nya programmet. Jag kontrollerar `write_mem()`-anropen i `setup()`.
- Hoppar CPU:n till fel adress? Då är `JMP`-adressen fel. `$8002` är `$02 $80` (låg byte först!).
- Stannar programmet? Då kanske jag har en `BRK`-instruktion (`0x00`) någonstans. Jag dubbelkollar att alla bytes är korrekta.

## Vad händer härnäst?

Nu har jag skrivit maskinkod som datorn verkligen kör. I nästa steg tar processorn över LCD:n helt själv — jag kopplar in en VIA.
