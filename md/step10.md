# EEPROM som ROM

Det här är steget där datorn slutar vara ett bygge och blir en maskin: slå av strömmen, slå på den — och den startar ändå. Programmet sitter i kisel nu.

## Mål

Hittills har Arduino levererat 6502-programmet — varje gång CPU:n läser från $8000 och uppåt är det Arduinons minnesarray som svarar. Det fungerar utmärkt, men en riktig dator har sitt program i ROM — icke-flyktigt minne som överlever strömavbrott. Nu tar vi det sista stora klivet: vi bränner in programmet på en AT28C256 EEPROM och låter den ersätta Arduino som ROM.

En andra Arduino Mega används som EEPROM-programmerare. Via USB tar den emot en .bin-fil från datorn, bränner den på AT28C256, och verifierar att varje byte sitter rätt. Sedan flyttar du EEPROM-chippet till datorns kopplingsdäck — och datorn startar direkt från äkta ROM, precis som en Commodore 64 eller Apple II.

Arduinon på kopplingsdäcket finns kvar som klocka och diagnostikverktyg — men datorn överlever utan den. Slå på strömmen, och CPU:n läser ditt program direkt ur EEPROM:et. Det här är en fristående dator.

## Nya komponenter

En AT28C256 EEPROM på 32 KB blir datorns ROM, en extra Arduino Mega agerar programmerare som bränner programmet, en extra 74HC00 avkodar $8000–$FFFF för EEPROM:et, och en kondensator avkopplar strömmen. Efter detta steg kan datorn starta helt utan Arduino.
| Antal | Komponent | Används till |
|---|---|---|
| 1 | **AT28C256** (32KB EEPROM, DIP-28) | Programminne — ersätter Arduino för $8000–$FFFF |
| 1 | **Arduino Mega 2560** (extra) | EEPROM-programmerare — används endast vid bränning |
| 1 | 74HC00 (extra) | Adressavkodning för EEPROM — /CE vid $8000–$FFFF |
| 1 | 100 nF keramisk kondensator | Avkoppling vid EEPROM:ets VCC/GND |
| — | Kopplingstråd | Adress, data, kontroll |

## AT28C256 — pinout

DIP-28-kapsel. 15 adresslinjer, 8 datalinjer, 3 kontrollsignaler. Nästan identisk med 62256 SRAM men med /WE som styr bränning och RDY/BUSY som signalerar när bränningen är klar (vi använder en enkel timeout istället).
> [!NOTE] 🧩 AT28C256 · se step10.html

■ Adressbuss ■ Databuss ■ Kontroll ■ Ström. Notch/pin 1-markering: uppåt.

## Kopplingsschema — programmeraren

Den andra Arduino Mega kopplas 1:1 till AT28C256. Här finns inga andra kretsar — bara Arduino, EEPROM, och 100Ω skyddsmotstånd på databussen. När bränningen är klar kopplas allt isär.
![Steg 10 — EEPROM-programmerare](schematics/steg-10-1.png)

## Kopplingsschema — datorn med EEPROM

EEPROM:et sitter nu på datorns adress- och databuss tillsammans med SRAM, VIA och Arduino. En extra 74HC00 avkodar A15 till /CE — EEPROM:et aktiveras vid $8000–$FFFF.
![Steg 10 — datorn med EEPROM](schematics/steg-10-2.png)

## Kopplingar

Först programmeraren — en enkel 1:1-koppling. Sedan datorn — fyra enheter på samma buss.

### Programmeraren — Arduino till AT28C256
| Signal | Arduino | AT28C256 | Varför |
|---|---|---|---|
| `VDD`, `VSS` | 5V, GND | `VDD` (28), `VSS` (14) | Strömmatning — glöm inte 100nF avkoppling |
| `A0–A7` | A0–A7 (PORTF) | `A0–A7` | Låga adressbyte — vilken byte som ska brännas |
| `A8–A14` | A8–A14 (PORTK) | `A8–A14` | Höga adressbyte — 15 bitar = 32 768 adresser |
| `D0–D7` | D22–D29 (PORTA) | `D0–D7` via 100Ω | Data — samma portregister som alltid |
| `/WE` | D2 | `/WE` (27) | Write Enable — pulsas LÅG för att bränna |
| `/OE` | D3 | `/OE` (22) | Output Enable — LÅG vid läsning/verifiering |
| `/CE` | GND | `/CE` (20) | Chip Enable — alltid aktiv under programmering |

### Datorn — EEPROM på bussen
| Pin | Signal | Ansluts till | Varför |
|---|---|---|---|
| 28, 14 | `VDD`, `VSS` | +5V, GND | Strömmatning |
| 1–10, 21–26 | `A0–A14` | CPU `A0–A14` | Delad adressbuss med SRAM, VIA, Arduino |
| 11–13, 15–19 | `D0–D7` | CPU `D0–D7` via 100Ω | Delad databuss |
| 20 | `/CE` | **74HC00-utgång** | Aktiveras vid $8000–$FFFF (A15=1) |
| 22 | `/OE` | GND | Alltid läs ut — EEPROM är read-only i datorn |
| 27 | `/WE` | +5V | Aldrig skriva — ROM-läge |

### 74HC00 — EEPROM-avkodning (ny grind)

En extra 74HC00 vid sidan av den från steg 7/9. En enda grind används som inverterare för A15.

A15 NAND A15 = NOT A15. EEPROM:ets /CE är aktiv LÅG. När A15=1 (adress $8000 eller högre) blir grindens utgång LÅG → EEPROM aktiverat. När A15=0 (under $8000) är utgången HÖG → EEPROM är bortkopplat, SRAM eller VIA tar över.
| Pin | Signal | Kopplas till | Varför |
|---|---|---|---|
| 1, 2 | `A15` (in) | **CPU `A15`** | Båda ingångarna till A15 → NAND = NOT A15 |
| 3 | NOT `A15` (ut) | **EEPROM `/CE`** (pin 20) | LÅG när A15=1 → EEPROM aktivt vid $8000–$FFFF |
| 14 | `VCC` | +5V | Strömmatning |
| 7 | `GND` | GND | Systemjord |

> [!NOTE] 🗺️ Minnestarta · se step10.html

## Arduino-kod — programmeraren

Det här programmet laddas upp på den andra Arduino Mega — den som tillfälligt kopplas in som EEPROM-programmerare. Koden är enkel: ta emot 32 768 bytes över serieporten, bränn dem på AT28C256, och verifiera. Programmet körs en gång i setup() och rapporterar resultatet — loop() är tom.

### Hur bränning går till

AT28C256 har inbyggd själv-timing. För att bränna en byte sätter du adress och data, drar /WE låg, väntar minst 10 millisekunder (vi använder 10 för marginal), och drar /WE hög igen. Kretsen sköter resten internt. För att läsa tillbaka drar du /OE låg och läser PINA — precis som med SRAM.

Verifieringen läser tillbaka varje byte och jämför med originaldatan. Om något inte stämmer rapporteras adressen och de två värdena. Annars: "OK — 32768 bytes verifierade".
> [!NOTE] 📦 EEPROM-programmeraren — EEPROM_programmer.ino · 139 rader · se step10.html

## Python-skript — skicka .bin-fil till programmeraren

Ett enkelt Python 3-skript som läser program.bin och skickar det sida för sida över serieporten. Kräver pyserial (pip install pyserial).
> [!NOTE] 📦 Python-skript — upload_eeprom.py · 59 rader · se step10.html

## Arduino-kod — datorn

Koden på datorns Arduino förenklas dramatiskt. program[2048] och de hundratals write_mem()-anropen försvinner. Istället får vi en ny funktion is_eeprom() som returnerar true för adresser mellan $8000 och $FFFF. När CPU:n läser från dessa adresser tri-statar Arduino bussen — EEPROM:et svarar själv.

Arduinon är nu reducerad till klockgenerator, reset-kontroll och seriell diagnostik. Alla minnesarrayer utom vectors[6] är borta. Vektorn på $FFFA–$FFFF ligger sist i EEPROM-bilden — .segment "VECTORS" i assembler-koden placerar dem där.

Koden nedan är baserad på steg 9 men med is_eeprom() istället för program[]. read_mem() returnerar 0 för EEPROM-adresser — Arduino rör dem inte.
> [!NOTE] 📦 Datorns Arduino — step10.inc · 124 rader · se step10.html

## Exempel på körning

### Fas 1 — bränna EEPROM

Bygg programmet med ca65 som vanligt, kör Python-skriptet för att bränna:
```
$ pio run -e step10 -t upload    # Ladda upp dator-Arduinon
$ python3 scripts/upload_eeprom.py asm/program_hello.bin /dev/ttyACM1

AT28C256 EEPROM-programmerare
Redo att ta emot .bin-fil...
Sida 1/8: Bränner sida 1 (adress $0000)... OK
Sida 2/8: Bränner sida 2 (adress $0100)... OK
...
Sida 8/8: Bränner sida 8 (adress $0700)... OK
Alla sidor mottagna.
Verifierar...
KLAR
Totalt brända bytes: 2054
```

### Fas 2 — flytta EEPROM till datorn

Koppla bort programmerings-Arduinon. Flytta AT28C256-chippet till datorns kopplingsdäck och anslut enligt kopplingstabellen ovan. Slå på strömmen.

Seriemonitor — datorn startar
```
Steg 10 — EEPROM som ROM
Programmet ligger i AT28C256 — inte i Arduino.
CPU startad. EEPROM levererar programkoden.

R $FFFC  ← RESET-VEKTOR LÅG (EEPROM)
R $FFFD  ← RESET-VEKTOR HÖG (EEPROM)
R $8000  ← EEPROM
R $8001  ← EEPROM
W $4002  ← VIA: FF
W $4003  ← VIA: FF
W $4000  ← VIA: 30  (LCD-init)
...
W $4000  ← VIA: 3D  (=)
...
```

LCD-displayen (16×2)

Varje gång du ser ← EEPROM i loggen är det AT28C256 som svarar — inte Arduino. Reset-vektorn på $FFFC/$FFFD ligger i EEPROM:ets sista bytes, inskrivna av assembler-kedjan. CPU:n läser dem, hoppar till $8000, och kör programmet direkt från äkta ROM.

Koppla bort Arduinon helt (ersätt klockan med en 555-timer eller kristalloscillator) så har du en helt fristående 6502-dator. Programmet överlever strömavbrott — slå på strömmen imorgon och det finns kvar, inbränt i kisel.

### Prova själv

- Bränn asm/program_fib.bin i stället för hello-programmet och se datorn starta direkt i Fibonacci — utan att röra datorns Arduino.

## Om det inte fungerar

Här är några saker att kontrollera:

- CPU:n läser $FFFC/$FFFD men hoppar till fel adress? EEPROM:ets /CE är förmodligen inte rätt avkodat. Kontrollera att 74HC00-grinden får A15 på båda ingångarna och att utgången går till EEPROM pin 20.
- Programmet körs inte alls? Kontrollera att /OE är kopplat till GND och /WE till +5V. Utan /OE låg kör EEPROM:et inte ut data.
- Bränningen misslyckas? Mät spänningen vid EEPROM:ets VDD — den måste vara minst 4.8V. Använd 12V DC-adapter till programmerings-Arduinon.
- Busskrockar? 100Ω motstånd på databussen är nu ännu viktigare — fyra enheter delar på samma 8 ledningar.
- Glöm inte avkopplingskondensatorn vid EEPROM:ets VDD/GND.

## Vad händer härnäst?

Din dator är nu fristående — nästan. I sista steget städar vi upp adressrymden, så att varje minnesbyte får ett tydligt jobb.
