# W65C02 8-bitarsdator

## En hembyggd dator från grunden

 Ända sedan jag först läste om hur enkla 8-bitarsprocessorer fungerar har jag drömt om att bygga en egen dator. Inte en snabb, inte en modern — utan en dator där varje komponent går att förstå. En dator där jag kan följa varje etta och nolla från processorns instruktionsregister hela vägen ut till en lysdiod eller en LCD-display. 

 Eftersom W65C02S är en statisk CMOS-krets kan klockan stoppas helt utan att processorn glömmer sitt tillstånd. Två fysiska knappar låter dig stega igenom programmet — en klockcykel eller en hel instruktion i taget. Du ser allt som händer, i din egen takt. 

## En Arduino som startmotor

 En 6502-dator från 1970-talet behövde ROM och RAM som fysiska chips — och för att testa ett nytt program fick man bränna ett nytt EPROM under UV-ljus, en process som tog 20 minuter per försök. Här använder vi en Arduino Mega 2560 som startmotor: den står för klocka, minne och diagnostik i början, så att vi kan fokusera på processorn och se varje signal på kopplingsdäcket. 

 Steg för steg ersätter vi Arduinons uppgifter med riktiga kretsar. SRAM-chippet tar över minnet, EEPROM:et tar över programkoden, VIA-kretsen tar över I/O till LCD-displayen. Till slut är Arduino reducerad till klocka och seriell diagnostik — och datorn fungerar helt på egen hand. 

 Det här är inte en emulator i mjukvara — processorn kör på riktigt, med riktiga elektriska signaler. Allt du lär dig om 6502:ans instruktionsuppsättning, timing och bussprotokoll gäller på riktigt. 

## Hur det hänger ihop

Så här pratar de olika kretsarna med varandra. CPU:n i mitten, adressbussen åt ena hållet och databussen åt det andra. Arduino är startmotorn — streckad, eftersom den fasas ut steg för steg.
> [!NOTE] 🧩 Översikt över 6502-datorn: Arduino, CPU, VIA med LCD, SRAM och EEPROM i en rad, med adress- och databuss ovanför och nedanför · se index.html

■ Adressbuss ■ Databuss ■ Kontroll ■ Ström/pinnar

## Hur fungerar en 6502-processor?

### Två bussar — adress och data

Processorn har två uppsättningar ledningar ut till världen. Den första är adressbussen — 16 ledningar som tillsammans bildar ett tal mellan 0 och 65 535. Det talet är en minnesadress: "jag vill läsa från adress 32 768", eller i hexadecimalt: $8000. Processorn lägger ut adressen på stiften A0–A15 och alla kretsar på bussen ser den samtidigt.

Den andra uppsättningen är databussen — 8 ledningar. Här kommer svaret tillbaka. När processorn vill läsa från en adress lägger den ut adressen, sätter R/W-signalen till HÖG (Read), och väntar. Den krets som känner igen adressen — i början Arduino, senare SRAM, EEPROM eller VIA — lägger ut rätt byte på databussen. Processorn läser av den och går vidare till nästa instruktion.

### Instruktionscykeln — hämta, avkoda, utför

Varje gång processorn startar en ny instruktion tänder den SYNC-pinnen. Det är processorns sätt att säga "nu börjar jag på något nytt". Sedan följer en förutsägbar dans:

1. Hämta opcode: processorn läser byten på den adress som programräknaren (PC) pekar på. SYNC är hög under just denna cykel.
1. Avkoda: processorn tittar på opcode-byten och förstår vad som ska göras. Är det $A9? Då ska nästa byte laddas in i A-registret (LDA #). Är det $8D? Då ska de två nästa byten tolkas som en adress (STA absolute).
1. Utför: processorn läser eventuella extra bytes (operander), utför operationen, och ökar programräknaren.
1. Nästa: PC pekar nu på nästa opcode. Börja om från steg 1.

### Arduino emulerar ram och rom

 En 6502-dator från 1970-talet hade ROM och RAM som fysiska chips. Varje gång man vill testa ett nytt program får man bränna ett nytt EPROM under UV-ljus — en process som tar 20 minuter per försök. Arduino vänder på det här: man laddar upp ett nytt program på två sekunder över USB. Arduino läser av processorns adressbuss, slår upp rätt byte i sin interna array, och lägger ut den på databussen — allt inom samma klockcykel. 

 Detta är inte en emulator i mjukvara — processorn kör på riktigt, med riktiga elektriska signaler. Arduinon är bara minnet. Det betyder att allt man lär sig om 6502:ans instruktionsuppsättning, timing och bussprotokoll gäller på riktigt. Man kan ta bort Arduino och ersätta den med ett EEPROM och ett SRAM-chip — och datorn fungerar precis likadant. 

### Arduino läser adressbussen

 Arduino Mega har 54 digitala I/O-pinnar — nästan tillräckligt för att täcka 16 adresslinjer + 8 datalinjer + kontrollsignaler. Men den har också något bättre: portregister. Istället för att läsa en pinne i taget med digitalRead() (långsamt!) läser vi hela 8-bitarsportar på en gång: 

- PORTF — 8 bitar, läser CPU:ns A0–A7 i en enda maskininstruktion
- PORTK — 8 bitar, läser CPU:ns A8–A15
- PORTA — 8 bitar, driver databussen D0–D7 (eller går tri-state)

 Tre registerläsningar, en villkorssats, och vi vet exakt vad processorn vill göra — redo att svara inom samma klockcykel. 

## Vad du lär dig per steg
| Steg | Du lär dig | Svårighet |
|---|---|---|
| 1 · Ström och klocka | 5V, GND, klocka, lysdiod | Lugn |
| 2 · Adressbuss | Hex, reset-vektorn, portregister | Lugn |
| 3 · Databuss | R/W, tri-state, minnesemulering | Medel |
| 4 · Knappar | SYNC, enstegning, debounce | Lugn |
| 5 · LCD | Parallell kommunikation, kontrast | Lugn |
| 6 · Eget program | Maskinkod, opcode/operand, little-endian | Medel |
| 7 · VIA + LCD | I/O-krets, minnesmappad I/O, adressavkodning | Brant — seriens största kliv |
| 8 · Assembler | ca65/ld65, labels, subrutiner | Brant |
| 9 · SRAM | Riktigt minne, chip enable, zero page, carry-aritmetik | Medel–Brant |
| 10 · EEPROM | ROM, programmering, bränning | Medel |
| 11 · Adressrymd | Chip select, spegling, minimal avkodning | Brant |

Fastnar du på ett ord? Se [Begrepp och förkortningar](begrepp.html).

## Verktyg och programvara
| Verktyg | Används till | Från steg |
|---|---|---|
| Multimeter | Mäta spänningar och kontrollera signaler — felsökningens viktigaste verktyg | 1 |
| Kopplingsdäck + trådar | Bygga kretsen utan lödning | 1 |
| PlatformIO + VS Code | Bygga och ladda upp Arduino-koden | 1 |
| ca65 + ld65 | Assemblera 6502-program till binärer | 8 |
| Python + pyserial | Skicka .bin-filer till EEPROM-programmeraren | 10 |
| Arduino IDE | Ladda upp EEPROM-programmeraren (andra Arduinon) | 10 |
