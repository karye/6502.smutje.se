# Fristående klocka

Det sista som påminner om Arduinon är klockan. Nu byter jag ut den mot en riktig kristall — när datorn sedan startar med strömmen behöver den ingen hjälp alls.
## Mål

Sedan steg 10 levererar EEPROM:et programmet och SRAM:et minnet — men Arduino har fortfarande skött *klockan*. Nu kopplar jag in en riktig klockgenerator: en 16 MHz-oscillator vars frekvens jag delar ner med en **74HC393** till 1 MHz. En enkel RC-krets sköter reset.

När det här steget är klart kan jag koppla bort Arduinon helt. Slå på strömmen och datorn startar på egen hand — programmet ur EEPROM:en, minnet i SRAM:en, texten på LCD:n. En riktig, fristående 6502-dator.

Hastigheten förändrar en sak: vid 1 MHz tar varje instruktion bara några mikrosekunder, och LCD:n hinner inte med om programmet inte själv väntar. Därför lär jag mig också *tidshantering* — delay-subrutiner som ger displayen de pauser den kräver.

1 MHz är samma fart som original-Apple II:n — snabbt nog att kännas som en riktig dator, långsamt nog att följa med i vad som händer.

## Nya komponenter
| Antal | Komponent | Används till |
|---|---|---|
| 1 | 16 MHz-oscillator (DIL-14, 5V) | Klockans källa — ren 16 MHz fyrkantsvåg direkt på utgången |
| 1 | 74HC393 (dubbel 4-bitars ripple-räknare, DIP-14) | Delar 16 MHz → 1 MHz (÷16) för `PHI2` |
| 1 | Tryckknapp | Manuell reset |
| 1 | 10 µF elektrolytisk kondensator | Reset-RC — håller `/RESET` låg en stund vid start |
| 1 | 10 kΩ motstånd | Pull-up till `/RESET` |
| 1 | 100 nF keramisk kondensator | Avkoppling vid 74HC393:ns VCC/GND |
## Kopplingsschema

Schemat visar klockkedjan: oscillatorn längst till vänster, 74HC393:an i mitten som delare, och `PHI2` ut till CPU:n. Resten av datorn (SRAM, VIA, EEPROM) är oförändrad från steg 11.

Schema kommer snart — se klockkedjan och kopplingstabellerna nedan så länge.

## Klockkedjan — från 16 MHz till 1 MHz

En oscillator är en komplett klockkrets i en kapsel: kristallen och förstärkaren sitter inbyggda, och utgången ger en ren fyrkantsvåg direkt — inget att bygga runt. 16 MHz är för snabbt för W65C02S (max 14 MHz), så jag delar ner frekvensen med en 74HC393.

74HC393:an innehåller *två* oberoende 4-bitars räknare. Varje steg halverar frekvensen, och alla utgångar har 50% arbetscykel — perfekt för `PHI2`:

| Utgång | Delning | Frekvens | Användning |
|---|---|---|---|
| 1Q0 (pin 3) | ÷2 | 8 MHz | Snabbast som är säker — men LCD-timing kräver många delay-loopar |
| 1Q1 (pin 4) | ÷4 | 4 MHz | Mellanfart |
| 1Q2 (pin 5) | ÷8 | 2 MHz | Mellanfart |
| 1Q3 (pin 6) | ÷16 | 1 MHz | Jag använder denna — klassisk 6502-fart, snäll mot LCD:n |
## 74HC393 — pinout

DIP-14-kapsel. Två oberoende 4-bitars ripple-räknare (1 och 2). `1Q3` är den utgång jag använder — 16 MHz ÷16 = 1 MHz till `PHI2`.
> [!NOTE] 🧩 74HC393 · se step12.html

## Kopplingar

Tre små kretsar att koppla: oscillatorn, delaren och reset-kretsen. Allt annat — ström, bussar, SRAM, VIA, EEPROM — är oförändrat från steg 11.
### Oscillator (DIL-14, 5V, 16 MHz)
| Pin | Signal | Kopplas till | Varför |
|---|---|---|---|
| 14 | `VCC` | +5V | Strömmatning — glöm inte 100 nF avkoppling |
| 7 | `GND` | GND | Systemjord |
| 8 | `OUT` | 74HC393 pin 1 (`1CLK`) | 16 MHz fyrkantsvåg till räknaren |
| 1 | `ENA` | — (lämnas oansluten) | Enable — de flesta moduler kör alltid |
### 74HC393 — frekvensdelare
| Pin | Signal | Kopplas till | Varför |
|---|---|---|---|
| 1 | `1CLK` | Oscillatorns ut (pin 8) | 16 MHz in — räknaren tickar på varje fallande flank |
| 2 | `1MR` | GND | Master reset — LÅG = räknaren räknar |
| 6 | `1Q3` | CPU `PHI2` (pin 37) | 16 MHz ÷16 = 1 MHz — datorns nya hjärtslag |
| 14 | `VCC` | +5V | Strömmatning — 100 nF avkoppling till GND |
| 7 | `GND` | GND | Systemjord |
### Reset-RC — start utan Arduino
| Komponent | Kopplas till | Varför |
|---|---|---|
| 10 kΩ | +5V → CPU `/RESET` (pin 40) | Pull-up — håller `/RESET` HÖG i drift |
| 10 µF | CPU `/RESET` → GND | Laddas långsamt vid start → `/RESET` hålls LÅG några ms, sedan HÖG |
| Knapp | CPU `/RESET` → GND | Manuell reset — tryck så startar datorn om |

## Programmet — nu med tidshantering

Programmet är samma LCD-hello som tidigare, men med en viktig skillnad: vid 1 MHz tar en instruktion bara ~2–3 µs, och HD44780-displayen kräver pauser mellan kommandon (klartext-rensning tar t.ex. 1,64 ms). Förut, vid 500 Hz, hanns allt med av sig själv — nu måste 6502-programmet *självt* vänta.

Lösningen är en `delay_ms`-subrutin med nästlade loopar. Varje varv i den inre loopen tar ~4 cykler ≈ 4 µs; 250 varv ≈ 1 ms. `lcd_cmd` väntar ~2 ms efter varje kommando, `lcd_data` ~1 ms efter varje tecken. Displayen får alltid den paus den behöver.
> [!NOTE] 📦 Programmet — program_standalone.asm · 144 rader · se step12.html

Programmet byggs med ca65 + ld65 och bränns om i EEPROM:en med steg 10-programmeraren:
```
cd Mega_2560_6502
ca65 -o asm/standalone/program_standalone.o asm/standalone/program_standalone.asm
ld65 -C asm/standalone/program_standalone.cfg -o asm/standalone/program_standalone.bin asm/standalone/program_standalone.o
python3 scripts/upload_eeprom.py asm/standalone/program_standalone.bin /dev/ttyACM1
```

Binären är 16 KB och ligger på EEPROM-adress 0 (= CPU `$C000`) med vektorer på `$3FFA` (= `$FFFA`) — samma minneskarta som steg 11. Länkskriptet `program_standalone.cfg` finns i `asm/standalone/`.

## Ingen Arduino-kod

Det här steget har ingen Arduino-kod — Arduino finns inte med på kopplingsdäcket längre. Allt som återstår av den gamla startmotorn är minnet av hur den hjälpte mig: klockan sköter kristallen nu, reset sköter RC-kretsen, och programmet ligger i EEPROM:en.
## Exempel på körning

Slå på strömmen. Inget att ladda upp, inget att ansluta — datorn bara startar. Efter en kort stund (LCD-initieringen) visas:

LCD 16×2 — efter start

- Texten skrivs om i en loop: visa en stund, rensa, börja om.
- Stäng av strömmen och slå på igen — datorn startar direkt ur EEPROM:en.
- Tryck på reset-knappen — datorn startar om utan att strömmen bryts.
- Koppla bort Arduino helt om den fortfarande sitter i — den behövs inte.

Ingen seriemonitor längre: LCD:n är datorns enda ansikte utåt. Jag vred på strömmen och datorn bara vaknade — precis som jag tänkt mig.
### Så här provar jag

- Jag kopplar en lysdiod (med 220 Ω) från `PHI2` (pin 37) till `GND` — den lyser svagt, eftersom den är på halva tiden (50% arbetscykel vid 1 MHz).
- Jag mäter `1Q0`–`1Q3` med multimetern — medelspänningen halveras för varje delningssteg (≈2,5 V, ≈1,25 V, ≈0,6 V, ≈0,3 V).
- Jag flyttar klockan från `1Q3` till `1Q0` (8 MHz) — jag ser vad som händer med LCD:n: programmets delay-loopar är räknade för 1 MHz, så displayen hinner inte med.
- Jag mäter `/RESET` (pin 40) med multimetern strax efter start — den ska ligga nära 0 V direkt, sedan stiga mot 5 V.

## Så här felsöker jag

Här är några saker jag kontrollerar:

- Ingen klocka? Då mäter jag oscillatorns ut (pin 8) — en multimeter visar ~2,5 V medel om 16 MHz-pulsen finns. Jag kontrollerar VCC (pin 14) och GND (pin 7).
- Startar CPU:n inte? Då mäter jag `PHI2` (pin 37) — ska vara 1 MHz, och `/RESET` (pin 40) — ska vara HÖG efter start. Jag kontrollerar reset-RC: 10 kΩ till +5V, 10 µF till GND.
- Är LCD:n blank? Då måste programmet i EEPROM:en vara rätt version — med delay-loopar. Jag bränner om med `program_standalone.bin` (inte hello-programmet från steg 10).
- Skakig eller felaktig text? Då kontrollerar jag avkopplingen (100 nF) vid 74HC393:ns VCC/GND och att `1MR` (pin 2) är GND.
- Busskrockar? Samma som tidigare: exakt en enhet ska vara aktiv per klockcykel — jag mäter chip-selects medan jag stegar.

