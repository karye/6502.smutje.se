# Analys av 6502-projektet

Datum: 2026-08-19
Omfattning: hela repot (HTML, README, Arduino-kod, asm, byggskript, git).
KiCad-scheman är **utanför** denna granskning (hanteras manuellt) — men TODO-posternas förslag om BE/CS1 varnas för i avsnittet *Kritiska rön*, eftersom de är fel och påverkar det manuella arbetet.

## 1. Sammanfattning

Projektet är tekniskt väl genomtänkt och stegvis pedagogiskt uppbyggt. Kärnan — 6502-emuleringen i Arduino (portregister, DDRA-timing, tri-state), ca65/ld65-bygget och adressavkodningen i HTML-dokumentationen — är korrekt. Men det finns **tre kritiska rön**, två av dem i dokumentationen (BE-pinnen) och byggskriptet, och ett som är en kod-/dokument-motsägelse (adressläsningen i steg 2–6). Därutöver en handfull allvarliga luckor (steg 10–11 saknar körbar kod, README är inaktuell) och många mindre synkfel.

## 2. Metod och kriterier

Analysen utgår från följande källhierarki (per projektägarens direktiv):

1. **HTML-sidorna (steg 6–11) = referens** — mest aktuella sanningen om adressrymd, avkodning och kopplingar.
2. **KiCad-scheman** — hanteras manuellt, ej granskade här.
3. **README.md** — behandlad som föråldrad dokumentation; avvikelser mot HTML flaggas som inaktuella.
4. **Arduino-kod + asm + byggskript** — granskade mot HTML där de beskriver samma sak.

Kriterier: teknisk korrekthet mot datablad (w65c02s.pdf, w65c22.pdf), konsistens mellan källorna, byggbarhet/reproducerbarhet, kodkvalitet, kompletthet och git-hygien. Byggkedjan (ca65/ld65) testades körande; databladen verifierades med `pdftotext`.

## 3. Inventering

### HTML (referens)
| Fil | Senast ändrad | Kommentar |
|-----|---------------|-----------|
| step1–5.html | 13 aug | Grundsteg |
| step6–11.html | 19 aug | Senaste arbetsytan (steg 9–11 mest aktuella) |
| index.html | 13 aug | Översikt, alla länkar pekar på befintliga filer ✓ |

### Arduino-kod (Mega_2560_6502)
- `src/main.cpp` — dispatcher med `STEP1`–`STEP9` (inget `STEP10`/`STEP11`).
- `src/step1–9.inc` — steg 1–9. Steg 10 och 11 har **ingen** .inc-fil.
- `asm/` — program_hello.asm, program_fib.asm, program.cfg (+ kommitterade `.bin`/`.o`).
- `scripts/` — build_asm.py (trasig, se K2), bin2h.py (fungerar).
- `platformio.ini` — env:step1–step9; `default_envs = step9`.

### Övrigt
- `schematics/` — PNG för steg 1–7, 9, 10-1, 10-2. Saknas: steg-8 (medvetet, samma hårdvara som steg 7) och steg-11 (step11.html har placeholder ”Schema kommer snart”).
- `docs/` — datablad (w65c02s.pdf, w65c22.pdf) + pinout-PNG:er.
- Git: 339 commits; grenarna main, via-c000 (efter, sannolikt övergiven), responsive (tom mot main).

## 4. Verifierat korrekt

- **ca65/ld65-kedjan fungerar.** `program.cfg` + `program_hello.asm` och `program_fib.asm` bygger till 2054-byte-binärer som är identiska med de kommitterade `.bin`-filerna och `.h`-headerarna. Reproducerbart.
- **Steg 11:s program.cfg (från step11.html) fungerar.** ld65 ger 16 384-byte ROM med vektorer på $FFFA (reset → $C000) — verifierat körande.
- **LCD-kopplingen stämmer** mellan kod, README och HTML: RS=D5, E=D6, DB4=D10, DB5=D9, DB6=D8, DB7=D7.
- **BE-nivån i HTML** (steg 1, 2, 3, 7, 9): BE = +5V via 10 kΩ, HÖG = bussar aktiva — stämmer med databladet.
- **VIA CS1/CS2B per HTML**: CS1 (pin 24) = +5V, /CS2 (pin 23) = avkodning — stämmer med databladet (CS1 måste vara HÖG, /CS2B LÅG).
- **Adressavkodningen i HTML (steg 9/10/11)** är elektriskt korrekt: SRAM $0000–$3FFF (U4C+U4D), VIA $4000–$7FFF (U4A+U4B), EEPROM $8000–$FFFF (steg 10, NOT A15) respektive $C000–$FFFF (steg 11, NAND(A15,A14)).
- **Steg 7–9-koden** har korrekt portregisterhantering, tri-state före PHI2-stigande flank, och minnesrutor utan uppenbara fel (undantag: A4 nedan).
- **index.html** — alla interna länkar pekar på befintliga filer.

## 5. Rön

### Kritiska

#### K1 — BE-pinnen: README och TODO föreslår fel nivå (GND), databladet kräver HÖG
Databladet (docs/w65c02s.pdf, avsnitt 3.2): *”When Bus Enable is high, the Address, Data and RWB buffers are active. When BE is low, these buffers are set to the high impedance status.”*

Fel i README.md:
- rad 19: `10 kΩ motstånd (pull-down: BE → GND)` — ska vara pull-**up** till +5V.
- rad 69: `BE | In | Bus Enable — LÅG = bussar aktiva, HÖG = tri-state` — omvänt.
- rad 99: `BE | GND | Bus Enable — LÅG = bussar aktiva` — fel.

Rätt i README: rad 243 (felsökning) och rad 254 (varning). HTML-sidorna har rätt (+5V via 10 kΩ).

**TODO.md-varning:** posten ”BE (CPU pin 36) → GND i KiCad för alla steg” är fel — gör inte detta. Koppla BE till **+5V via 10 kΩ** i stället. (Viktigt inför det manuella KiCad-arbetet.)

Åtgärd: rätta README-rad 19/69/99; ändra TODO-posten till ”BE → +5V via 10kΩ”.

#### K2 — build_asm.py bygger en fil som inte finns
`Mega_2560_6502/scripts/build_asm.py` (rad 14–17) pekar på `asm/program_hello.hello.asm` — filen heter `program_hello.asm`. ca65 misslyckas → `pio run -e step8` / `-e step9` avbryts i pre-build-steget. (De kommitterade `.h`/`.bin`-filerna gör att befintliga byggen ser ut att fungera, men en ren checkout går inte att bygga.)

Åtgärd: byt till `program_hello.asm` (och justera `.o`/`.bin`-namnen i samma skript för konsistens).

#### K3 — Adressläsningen är byte-vänt i steg 2–6
- `src/step2.inc` … `src/step6.inc`: `uint16_t addr = (PINK << 0) | (PINF << 8);`
- `src/step7.inc`–`src/step9.inc`: `uint16_t a = (PINF << 0) | (PINK << 8);` (korrekt)
- HTML-tabellerna (step2–6 ”Kopplingar”): A0–A7 → Arduino A0–A7 (PORTF), A8–A15 → Arduino A8–A15 (PORTK). Låg byte = PINF.
- Exempelutskrifterna i step2/3/6.html visar korrekta adresser ($FFFC, $8000, $0200) — de motsäger formeln i steg 2–6.

Formeln i steg 2–6 introducerades i commit f2d2a41 (”Fix address calculation…”) — samma commit gav step8.inc den korrekta kl/kh-formen. Med nuvarande steg 2–6-kod och dokumenterad koppling skulle reset-vektorn läsas som $FCFF i stället för $FFFC och programmet aldrig starta på $8000.

Slutsats: antingen är koden fel (mest troligt, eftersom steg 7–9 och HTML-exemplen använder motsatt ordning) eller så är kopplingstabellerna fel. Måste verifieras mot hårdvara (se Öppna frågor), men kod och dokumentation måste i vilket fall synkas.

Åtgärd: ändra steg 2–6 till `(PINF << 0) | (PINK << 8)` — eller, om hårdvaran verkligen är omvänd, rätta tabellerna i stället.

### Allvarliga

#### A1 — README:s minneskarta för steg 11 är inaktuell
README rad 126–139: VIA på $A000–$A00F, EEPROM på $A010–$FFFF och $8000–$9FFF.
HTML (step11.html, referens): SRAM $0000–$7FFF (32 KB), VIA $8000–$BFFF (I/O-fönster), EEPROM $C000–$FFFF.
Åtgärd: uppdatera README till $8000/$C000-layouten (eller peka på step11.html).

#### A2 — Steg 10 och 11 har ingen körbar kod i repot
- Ingen `src/step10.inc` / `src/step11.inc`, inga `env:step10`/`env:step11` i platformio.ini, inga `STEP10`/`STEP11` i main.cpp.
- Steg 10:s EEPROM-programmerare (Arduino-skiss) och `upload_eeprom.py` (Python) finns bara som kodblock i step10.html.
- Steg 11:s Arduino-kod och nya program.cfg (ROM $C000) finns bara i step11.html.

Det går alltså inte att bygga steg 10–11 från repot. Åtgärd: lägg till kodfiler + envs (och gärna `upload_eeprom.py` under t.ex. `Mega_2560_6502/scripts/`).

#### A3 — README:s komponentlista saknar steg 9–11-hårdvara
62256 SRAM, AT28C256 EEPROM, andra Arduinon (programmerare, steg 10) och andra 74HC00 (steg 10) saknas i ”Komponenter (alla steg)”. BOM-raden för BE är dessutom fel (K1).

#### A4 — Latent busskollision på $4010–$7FFF i steg 7–10
I steg 7–10 är `is_via()` = $4000–$400F, men den fysiska VIA:n (CS1=+5V, /CS2 låg vid $4000–$7FFF) svarar på hela fönstret. Arduino:n driver då 0xEA på läsningar i $4010–$7FFF — båda driver databussen samtidigt. Demoprogrammen når aldrig dit, men en avläsning av området (t.ex. vid felsökning) ger en krock. I steg 11 är `is_via()` = $8000–$BFFF — korrekt hanterat där.
Åtgärd: utöka `is_via()` till hela VIA-fönstret även i steg 7–10.

### Mindre

- **M1 — Default-steg motsäger varandra:** main.cpp-kommentar ”default = steg 7”, fallback inkluderar step8.inc, README ”default = steg 8”, platformio.ini `default_envs = step9`.
- **M2 — Klockfrekvens-dokumentationen:** README rad 93 ”Klocka (20 Hz)” och stegtabeller i HTML (steg 2/3/7) säger 20 Hz; koden kör 500 Hz (steg 2+) / 1 Hz (steg 1). program_fib.asm-kommentaren ”≈ 1 sekund vid 50 Hz” är också inaktuell (nu ~0,1 s vid 500 Hz).
- **M3 — Inaktuella README-texter:** rad 180 ”Alla sju steg” (det finns nio .inc); rad 183 ”env:step1 … env:step8” (nio envs); rad 138 ”Steg 7–10 använde … SRAM på $0000–$3FFF” (steg 7–8 hade inget fysiskt SRAM); rad 157 steg 11 ”(andra 74HC00)” (HTML säger ”inga nya kretsar” — den andra 74HC00 kom i steg 10).
- **M4 — TODO.md: ”Via CS1 (pin 4) kopplas till GND — steg 7” (bockad):** fel pinnummer — CS1 är pin **24** (pin 4 är PA2, LCD E) — och fel nivå: CS1 ska vara **HÖG** för att VIA:n ska svara. Kontrollera om denna ändring redan gjordes i KiCad; i så fall fungerar inte VIA:n. (Viktigt inför det manuella KiCad-arbetet.)
- **M5 — Steg 1, RESB:** HTML säger ”/RESET lämnas flytande — CPU:n startar själv”, medan step1.inc håller RESB låg via D4 (CPU i reset). Dokumentation och kod säger emot varandra.
- **M6 — program_fib.h byggs inte automatiskt:** build_asm.py genererar bara program_hello.h. Vid ändring av program_fib.asm måste headern genereras manuellt.
- **M7 — Byggartefakter i git:** `asm/*.bin` och `asm/*.o` är kommitterade; Mega_2560_6502/.gitignore har inaktuella sökvägar (`src/program.o`, `src/program.bin`) och ignorerar inte `asm/*.o`, `asm/*.bin`.
- **M8 — Grenen `via-c000`** ligger efter main och innehåller en äldre VIA-$C000-variant — sannolikt övergiven. Rensa eller arkivera.
- **M9 — README ”6502-programflöde (steg 7)”** beskriver 4-radersprogrammet ($94/$D4), medan step7.inc skriver 2 rader (”W65C02 VIA LCD” + ”smutje.se W65C02”).

### Kosmetiska

- **Kos1 — step1.html:** step11-länken i nav har annorlunda indragning; step1 saknar ”→ Steg 2”-länk (övriga sidor har prev/next).
- **Kos2 — step9/step10-minneskartor** visar $4010–$7FFF som ”oanvänt” trots att VIA-register speglas där (presentationsval — förtydliga eller justera).

## 6. Status per steg

| Steg | Status |
|------|--------|
| 1 | Kod ok; RESB-diskrepans (M5); klocka 1 Hz i kod vs ”20 Hz”-dok. |
| 2–6 | **Byte-vänt adressläsning (K3)**; LCD-koppling (steg 5–6) korrekt. |
| 7 | Kod korrekt med rik diagnostik; VIA-avkodning per HTML korrekt; latent krock (A4); README-flöde beskriver 4 rader (M9). |
| 8 | Byggkedjan dokumenterad men build_asm.py trasig (K2); program_hello.asm kompilerar; default-otydlighet (M1). |
| 9 | Kod ok (förutom A4); SRAM-avkodning i HTML korrekt; program_fib.h byggs ej automatiskt (M6). |
| 10 | Kod bara i HTML (A2); minneskarta ok i HTML; BOM saknar 2:a Arduino + 74HC00 + AT28C256 (A3). |
| 11 | Referenssida; minneskarta/avkodning korrekt och verifierad mot ld65; kod + program.cfg bara i HTML (A2); README inaktuell (A1); schema-placeholder ”kommer snart”. |

## 7. Prioriterad åtgärdslista

1. **K1** – Rätta BE i README (rad 19/69/99) och TODO (koppla BE till +5V via 10 kΩ, inte GND).
2. **M4** – Kontrollera CS1 i KiCad (pin 24 ska vara HÖG) innan schemana ritas klart.
3. **K2** – Fixa filnamnet i build_asm.py.
4. **K3** – Verifiera adressbussens koppling på hårdvara och synka steg 2–6-koden med steg 7–9 och HTML.
5. **A1** – Uppdatera README-minneskartan för steg 11.
6. **A2** – Lägg till körbar kod för steg 10–11 (step10/11.inc, env:step10/11, programmeraren + upload_eeprom.py).
7. **A3** – Komplettera README-komponentlistan.
8. **A4** – Utöka is_via() till hela VIA-fönstret i steg 7–10.
9. **M1–M3, M5–M9** – Dokumentations- och hygiensynk.
10. **Kos1–Kos2** – Kosmetik.

## 8. Öppna frågor (kräver hårdvara eller projektägaren)

- Är adressbussen i steg 2–6 verkligen kopplad A0–A7 → PORTF enligt tabellerna (då var koden fel), eller omvänt? Koden är nu synkad med tabellerna (K3-åtgärden); verifiera på hårdvara att steg 2–6 visar korrekta adresser i loggen.
- Har CS1-ändringen (”pin 4 → GND” i gamla TODO) redan gjorts i KiCad? (M4)
- Är det korrekt att steg 11 inte kräver någon ny krets (HTML), eller krävs ”andra 74HC00” (gamla README)? (M3)

## 9. Åtgärdsstatus (uppdaterad efter åtgärderna)

| Punkt | Åtgärd | Status |
|-------|--------|--------|
| K1 | BE → +5V via 10 kΩ i README (BOM, pinout, CPU-tabell) + TODO.md rensad | ✅ |
| K2 | build_asm.py bygger nu rätt filnamn (alla asm/*.asm) | ✅ |
| K3 | steg 2–6-adressläsningen synkad med steg 7–9 (`PINF \| PINK<<8`) | ✅ (verifieras på hårdvara) |
| M4 | TODO.md: CS1 (pin 24) ska vara +5V — korrekt vägledning inför KiCad-arbetet | ✅ (dok) |
| A1 | README-minneskartan för steg 11 uppdaterad till $8000/$C000 | ✅ |
| A2 | step10.inc + step11.inc + env:step10/11 + EEPROM_programmer.ino + upload_eeprom.py | ✅ |
| A3 | README-BOM kompletterad (62256, AT28C256, 2:a Arduino, 2:a 74HC00) | ✅ |
| A4 | is_via() utökad till hela VIA-fönstret i steg 7–10 (+ step8/9/10.html-kodblocken) | ✅ |
| M1 | Default-steg synkat: main.cpp + platformio.ini + README → steg 9 | ✅ |
| M2 | Klockfrekvens-dok: 500 Hz i README + step7/9.html + fib-kommentar | ✅ |
| M3 | README-texter: ”elva steg”, env:step1…11, steg 9–10-SRAM, steg 11 utan ny krets | ✅ |
| M5 | step1.html: /RESET hålls låg av Arduino (i stället för ”flytande”) | ✅ |
| M6 | build_asm.py bygger både program_hello och program_fib (headers verifierade identiska) | ✅ |
| M7 | asm/*.o + asm/*.bin borttagna från git-spårning; .gitignore uppdaterad | ✅ |
| M8 | Grenen via-c000 lämnas — radera/arkivera vid tillfälle | ⏳ |
| M9 | README:s steg-7-programflöde korrigerat till 2 rader ($80, $C0) | ✅ |
| Kos1 | step1.html: nav-indragning rättad (nästa-länk fanns redan) | ✅ |
| Kos2 | Minnesskartornas presentationsval ($4010–$7FFF som ”oanvänt”) lämnas | ⏳ |

Ej gjorda (kräver projektägaren): M8 (grenrensning), Kos2 (presentation), KiCad-nätlister/PNG-export, samt hårdvaruverifiering av K3.
