# Pedagogisk analys av 6502-webbplatsen

Datum: 2026-08-19
Fokus: inlärningsupplevelsen i index.html + step1–11.html.
Viktning (per projektägaren): **jämn svårighetskurva utan plötsliga hopp** och **konkret byggbarhet** — man ska kunna bygga och testa fysiskt. Design/visualisering utvärderas som pedagogiskt verktyg, inte estetiskt.

## 1. Sammanfattning

Sajten är pedagogiskt stark: varje steg har Mål → Kopplingar (med *varför*) → Exempel på körning (med förväntad utdata) → Om det inte fungerar (med mätvärden), en genomgående berättelse och bra analogier (klocka = hjärtslag, R/W = trafikljus, buss = motorväg, Arduino = startmotor). Den stora styrkan är **konkretionen** — allt går att verifiera fysiskt.

De två svagaste punkterna, sett ur din viktning:

1. **Två plötsliga svårighetshopp**: steg 6→7 (VIA + 74HC00 + LCD-direkt + ett ~10× längre maskinkodsprogram på en gång) och steg 8→9 (från hello-program till Fibonacci med carry-aritmetik, stack och decimalomvandling i ett enda steg).
2. **Tre konkreta hål** där bygginstruktionen är ofullständig eller felaktig: step11 saknar kopplingsschema (placeholder), step8.html säger `pio run -t upload` (bygger nu default steg 9, inte steg 8), och step11.html pekar på `src/program.cfg` (filen ligger i `asm/`).

Därutöver: få aktiva lärande-markörer, ingen ordlista, ingen print-CSS, den mänskliga berättarrösten finns bara på index (se avsnitt 10) — och utseendet är inte helt enhetligt (se avsnitt 11).

## 2. Metod och kriterier

Analysen utgår från kriterierna i planen, med dina två viktningar överst:

1. **Jämn progression** — ett nytt konceptkluster per steg; inga hopp.
2. **Konkret byggbarhet** — varje steg ska gå att bygga och verifiera fysiskt.
3. Överblick och navigering.
4. Begrepp och förklaringar (definitioner, analogier, varför).
5. Aktivt lärande (övningar, experiment, kontrollfrågor).
6. Två spår: begrepp (hur en dator fungerar) och färdighet (bygga/felsöka).
7. Visualisering som pedagogiskt stöd.
8. Tillgänglighet/UX (mobil, kontrast, alt-texter, print).
9. **Röd tråd och berättarröst** (tillagt av projektägaren) — varje sida läses som en berättelse, inte en manual: mänsklig introduktion, konsekvent röst som bär genom hela sidan, framåtblickande avslut.
10. **Enhetligt utseende** (tillagt av projektägaren) — sidorna ska se ut som en serie: samma byggstenar (tabeller, kodblock, serielloggar, callouts, minneskartor), samma färger och formatering. Igenkänning hjälper läsaren.

## 3. Svårighetskurvan — kärnan

| Steg | Nya konceptkluster | Bedömning |
|------|--------------------|-----------|
| 1 | Ström/5V, klocka (statisk CMOS), LED+motstånd, pull-ups, reset låg | Lugn start ✓ |
| 2 | Adressbuss, hex, reset-vektor $FFFC/$FFFD, portregister, seriemonitor | Mjuk ✓ |
| 3 | Databuss (dubbelriktad), R/W, tri-state/DDRA, minnesemulator, NOP-fallback | Största kodsteget hittills, väl förklarat ("kodens fyra lager") ✓ |
| 4 | SYNC, klocksteg vs instruktionssteg, debounce | Lätt ✓ |
| 5 | LCD 4-bit, LiquidCrystal, kontrastpot | Mjuk ✓ |
| 6 | Maskinkod (opcode/operand), little-endian, X-register, 4 opcodes | Mjuk — bra tabell per instruktion ✓ |
| 7 | **VIA + register + DDR + minnesmappad I/O + 74HC00-avkodning + chip select + LCD direkt från 6502 + tri-state-pass-through + program ~10× längre** | **HOPP ⚠** |
| 8 | Assembler (labels, JSR/RTS, segment, .byte/.word), ca65/ld65, program.cfg | Brant men utan ny hårdvara — acceptabelt |
| 9 | SRAM (CE/OE/WE), alla 4 NAND-grindar, zero page, **carry-aritmetik, stack, branchar, decimalomvandling** | **HOPP ⚠ (programmeringsfärdighet)** |
| 10 | EEPROM, bränning, andra Arduino + Python, sidprotokoll | Nytt arbetsflöde men begreppsmässigt lätt ✓ |
| 11 | Avkodningsdesign (chip select, spegling, minimal NAND), ny minneskarta | Djup men fokuserad, bra avslut ✓ |

### Hopp 1: steg 6 → 7

Steg 7 introducerar fem saker samtidigt: (a) vad en VIA är och dess register, (b) minnesmappad I/O med adressavkodning (74HC00 + chip select), (c) att 6502:an själv styr LCD:n via portpinnar, (d) tri-state-pass-through i Arduino-koden, (e) ett maskinkodsprogram som är en tiopotens längre än steg 6:s. Sektionerna är välstrukturerade (pinouts, registertabell, maskinkodstabell), men den samlade kognitiva belastningen är klart högst i hela serien.

**Förslag för att jämna ut:**
- Lägg till en kort konceptsektion "Så här fungerar en VIA" innan kopplingarna (register = brevlådor, DDR = vägskyltar för riktning, minnesmappad I/O = "chipet svarar på adresser som om det vore minne").
- Lägg en explicit "Vad är nytt i detta steg"-lista överst (Mål gör det delvis — gör checklistan fem punkter, en per kluster).
- Chunk:a maskinkodstabellen: först VIA-init, sedan LCD-init, sedan text — med en rubrik per chunk (tabellen har redan stegnummer — förstärk med delrubriker).

### Hopp 2: steg 8 → 9 (assembler-färdighet)

Steg 8 lär ut assembler med ett enkelt program (labels, JSR/RTS, LDA line1,x, .byte/.word). Steg 9 kräver på en gång: carry-aritmetik (CLC/ADC/SEC/SBC), branch-mönster (BCC/BNE), stack (PHA/PLA), jämförelseloopar (CMP), TXA/DEX/DEY, zero-page-variabler och en avancerad decimalomvandlingssubrutin. Texten förklarar allt väl — men *färdighetssteget* är stort, och till skillnad från steg 6 saknas en referenstabell över de nya instruktionerna.

**Förslag:**
- Lägg till en "Nya instruktioner i detta program"-tabell (som steg 6:s), med exempel på användning.
- Lägg till ett spårat exempel: "så här går det till när 144 ska visas" — följ show_num steg för steg (hundratal, tiotal, ental, mellanslagen).
- Alternativt/komplement: ett mellansteg — ett enkelt "räkna till 255 och visa på LCD" innan Fibonacci.

## 4. Konkret byggbarhet

**Styrkor (byggbarheten är sajtens bästa sida):**
- Varje steg har kopplingstabeller med *varför*-kolumn.
- Exempel på körning visar förväntad utdata (serieloggar, LCD-renderingar).
- Felsökningssektionerna ger mätbara kontroller (t.ex. steg 7: "PHI2 ~2,5 V medel, BE > 4,8 V, /CS2 LÅG vid $4000–$7FFF") och en logikanalysator-tips.
- Komponenttabeller per steg ("Nya komponenter").

**Hål:**
- **step11: inget kopplingsschema** — placeholder "Schema kommer snart". Det sista steget är det minst byggbara tills schemat finns. (Känt — pågår.)
- **step8.html "Så här använder du det": `pio run -t upload -t monitor`** bygger nu *default_envs = step9*, inte steg 8. Instruktionen måste vara `pio run -e step8 -t upload -t monitor`. (Introducerades när default ändrades — analysens tekniska runda ändrade default till steg 9 men step8.html uppdaterades inte.)
- **step11.html: "Öppna `src/program.cfg`"** — filen ligger i `asm/program.cfg`. Sökvägen leder fel.
- **step10.html:s exempel** anropar `python3 upload_eeprom.py …` — filen ligger nu i `scripts/`; exemplet bör visa korrekt sökväg (och `pio run -e step10` fungerar nu när env:step10 finns).
- Steg 6 saknar helt "Nya komponenter"-sektion (alla andra steg har en, även steg 11 med "inga nya kretsar!") — ett litet konsistenshål.

## 5. Överblick och navigation

- index.html är en stark ingång: SVG-blockschema, "Hur fungerar en 6502-processor?" (begreppsgrund), komponenttabell, stegtabell, minneskarta.
- **Saknas på översikten:** (a) en "vad du lär dig per steg"-karta (färdigheter/svårighet — skulle direkt adressera kurvan), (b) en "verktyg och programvara"-sektion (PlatformIO, ca65, Python, multimeter, kopplingsdäck) — i dag nämns verktyg först i steg 8, (c) en tidsuppskattning per steg.
- Sektionsordningen är förutsägbar på alla stegsidor ✓; TOC + prev/next finns ✓.
- Namn-inkonsekvens: steg 1 använder "Komponenter för detta steg", övriga "Nya komponenter"; steg 7 använder "Kopplingar — krets för krets".

## 6. Begrepp och förklaringar

- **Styrka:** nya begrepp definieras i regel vid första användning (little-endian, opcode/operand, tri-state, chip select …), och många "varför"-förklaringar är utmärkta (100 Ω-skyddet i steg 3, DDRA-timing, avkodningens logik i steg 11).
- **Ingen ordlista/begreppssida finns.** För en serie med ~30 facktermer vore en appendix-sida (eller en sektion på index) värdefull: opcode, operand, little-endian, hex, pull-up, tri-state, DDR, chip select, zero page, fallande flank, spegling …
- **Terminologi-inkonsekvens:** samma signal skrivs R/W (39 förekomster), RWB (30) och Read/Write (4). Föreslå en standard: R/W för signalen i text, RWB när pinnen (CPU pin 34) avses — och enhetlig genomgång.
- "körs på 6502:an — inte på Arduino" markeras explicit först i steg 7–8; steg 6 borde ha samma tydliga markering (programmet laddas in av Arduino men exekveras av processorn).

## 7. Aktivt lärande

- **Få markörer:** "Lägg märke" (4), "Prova att" (1), "Testa" (1). Inga övningar, inga självtest, inga "fundera på"-frågor.
- Det enda experimentet (ändra CLOCK_HZ i steg 1) är bra — samma mönster saknas i senare steg.
- **Förslag:** avsluta varje steg med 1–2 "Prova själv"-förslag (ändra texten på LCD, räkna baklänges, flytta en koppling och se vad som händer, mät med multimetern…). Kostar lite, höjer lärandet mycket.
- Felsökningssektionerna ger facit per symptom; ett par rader om *metod* (mät → isolera → verifiera) skulle lära ut tänkandet, inte bara svaren.

## 8. Visualisering som pedagogiskt stöd

- **Styrkor:** konsekvent färgkodning (grön = SRAM, gul = VIA, lila = EEPROM, blå = Arduino) genom index, minneskartor och loggar; pinout-SVG:er på varje ny krets; minneskartor som staplar; sanningstabell i steg 11; LCD-renderingar i Exempel-sektionerna.
- Färgbetydelsen finns förklarad i index ("■ Adressbuss…") och i minneskartorna — men inte på varje sida där färgerna används; en återkommande enradslegend vore hjälpsamt.
- Några SVG:er har role="img" + aria-label (index, steg 1/5/7/9/10), andra saknar — enhetlighet saknas.

## 9. Tillgänglighet och UX

- **Mobil:** tabeller rullar (overflow-x-auto), SVG:er skalar, nav wrappar — bra.
- **Print saknas:** ingen `@media print` — utskrift/PDF ger mörk nav och bakgrundsfärger. För en byggserie man gärna skriver ut är detta värt en liten CSS.
- Kontrast och teckenstorlek är i allmänhet bra.
- Minnesskartorna (div-baserade) saknar motsvarighet till alt-text — mindre.

## 10. Röd tråd och berättarröst

Kriterium (tillagt av projektägaren): varje sida ska ha en röd tråd och en tydlig röst som berättar — från början med en mänsklig introduktion. Ingen sida ska kännas rörig.

**Nuläget:**
- **index.html har den mänskliga "jag"-rösten** (”Ända sedan jag först läste om hur enkla 8-bitarsprocessorer fungerar har jag drömt om att bygga en egen dator…”). Stegsidorna har nästan ingen "jag"-röst — de är skrivna i instruktiv "vi"-form.
- **Mål-sektionerna har redan en sorts berättarröst** (”I steg 1 gav vi…”, ”Hittills har… Nu kopplar vi in…”, ”Det kändes slarvigt”) — konsekvent och med bakåt-/framåtreferenser. Det är seriens starkaste tråd.
- **Tråden bryts i sidornas mitt och slut:** referensblock (pinouts, tabeller, minneskartor) staplas utan övergångsmeningar, och ingen sida slutar med en framåtblick — ”Om det inte fungerar” följs direkt av prev/next-länkar. Undantag: step11:s ”Lägg märke till skillnaden mot steg 10…”.

**Per-sida-bedömning:**

| Sida | Öppning/röst | Tråd genom sidan | Rörighet |
|------|--------------|------------------|----------|
| index | Utmärkt — personlig "jag"-röst | Bra | Ren |
| 1–6 | Bra öppningar (”Alla datorer behöver en puls”, ”Dags att skriva vårt första egna 6502-program!”) | Bra | Rena |
| 7 | Bra öppning (”Nu tar vi nästa stora kliv”) | Finns men tunnas ut — mitten är flera stora referensblock i rad | Tätast sidan (inte fel ordning, men rösten bär inte läsaren genom mitten) |
| 8 | Bra öppning | **Trådbrott:** ”Minneskarta” ligger mitt i byggkedje-berättelsen, och ”Så här använder du det” kommer efter kod-detaljerna | Rörig sektionsordning |
| 9 | Bra öppning (”vackrare än hello-programmet”) | Bra | Ren |
| 10 | Bra; världsbytena programmerare/dator är tydligt markerade (”Den andra Arduino…”, ”Först programmeraren… Sedan datorn…”) | Bra | Lång men markerad |
| 11 | Bra öppning (”Det kändes slarvigt”) | Bra | Hål: schema-placeholder mitt i tråden |

**Förslag (innehållsarbete):**
1. Lägg en kort personlig öppningsrad överst på varje stegsida (före eller som första mening i Mål) — samma "jag" som på index (t.ex. steg 6: ”Det är en speciell känsla första gången en dator du byggt själv kör ett program du skrivit själv.”).
2. Lägg en framåtblickande avslutning före prev/next-länkarna på varje sida — ”Vad händer härnäst” (t.ex. steg 1: ”Nu när processorn har puls kan vi börja prata med den — nästa steg kopplar in adressbussen.”).
3. Steg 8: flytta ”Minneskarta” ut ur byggkedje-berättelsen och ”Så här använder du det” närmare ”Byggkedjan”, så ordningen följer berättelsen: Mål → Nya verktyg → Byggkedjan → PlatformIO-byggskript → Så här använder du det → 6502-programmet i assembler → Arduino-kod → Minnestarta → Exempel på körning → Om det inte fungerar.
4. Steg 7: lägg korta övergångsmeningar mellan de stora referensblocken (pinout → kopplingar → minneskarta → program) så rösten bär läsaren.

## 11. Enhetligt utseende

Kriterium (tillagt av projektägaren): sidorna ska se ut som en serie — samma byggstenar, samma färger, samma formatering. Igenkänning hjälper läsaren att hitta rätt typ av innehåll.

**Vad som redan är enhetligt:**
- Tabeller: exakt samma klass överallt (`w-full … bg-white rounded-lg shadow`) ✓
- Kodblock: samma vita stil (`bg-white … border`) på alla sidor ✓
- Minnesskartor: färgade staplar (grön SRAM / gul VIA / lila EEPROM) på steg 6–11 ✓
- Nav, footer, TOC, prev/next: konsekventa ✓

**Inkonsekvenser:**
1. **Seriellog-stilen finns i två varianter.** Standard är `bg-gray-900 text-green-400 p-3 rounded-lg overflow-x-auto` — men step2, step5 och step8 har en äldre variant (`text-gray-100`, `rounded` utan `rounded-lg`, utan `overflow-x-auto`).
2. **Callout-rutor används ojämt.** Färgade rutor (röd = varning, grön = tips/notis, gul = steg 11) finns på step1/3/6/7/8/11 men inte alls på step2/4/5/9/10. Samma typ av notis är ruta på en sida och vanligt stycke på en annan.
3. **step8 använder nästan inga signal-spans** (1 förekomst jämfört med 20–60 på övriga sidor) — signalnamn formateras därmed annorlunda där.
4. **Chip-fetstilen är inte helt genomförd:** steg 11 fetar inte ”Arduino” (0 förekomster, jämfört med 4–14 på andra sidor).

**Förslag:**
1. Uppdatera seriellog-blocken i step2/5/8 till standardstilen.
2. Definiera callout-konventionen (grön = tips/”lägg märke”, röd = varning, gul = notis) och tillämpa den konsekvent — eller konvertera befintliga rutor till vanliga stycken så att byggstenen försvinner helt.
3. Lägg signal-spans i step8 där signalnamn nämns.
4. Städa chip-fetstil i step11 (CPU, Arduino, SRAM, VIA, EEPROM, LCD, 74HC00).

## 12. Per-steg-bedömning

| Steg | Pedagogik | Byggbarhet | Kommentar |
|------|-----------|-----------|-----------|
| 1 | Bra | Bra | Experimentet med CLOCK_HZ är föredömligt |
| 2 | Bra | Bra | Reset-vektorn förklaras väl |
| 3 | Utmärkt | Bra | "Kodens fyra lager" + trafikljus-analogin |
| 4 | Bra | Bra | SYNC-konceptet tydligt |
| 5 | Bra | Bra | LiquidCrystal förklaras på rätt nivå |
| 6 | Utmärkt | Bra | Instruktionstabellen är mönsterexempel; saknar "körs på 6502"-markering och "Nya komponenter"-sektion |
| 7 | Strukturerad men överlastad | Bra | Det stora hoppet; felsökningen är den bästa i serien |
| 8 | Bra | **Hål** | Fel env-kommando i "Så här använder du"; annars bra byggkedjeförklaring |
| 9 | Bra förklaring, stort färdighetssteg | Bra | Saknar instruktionsreferens och spårat exempel |
| 10 | Bra | Bra | Tvåfas-flödet (bränn → flytta) är konkret; sökvägar till filer bör stämmas |
| 11 | Utmärkt konceptuellt | **Hål** | Saknar schema + fel program.cfg-sökväg |

## 13. Prioriterad åtgärdslista

**Snabba fixar (dokument, liten insats):**
1. ✅ step8.html: `pio run -t upload` → `pio run -e step8 -t upload -t monitor` (alla tre ställen).
2. ✅ step11.html: `src/program.cfg` → `asm/program.cfg`.
3. ✅ step10.html: `python3 scripts/upload_eeprom.py …` i exemplet + println i kodblocket (och EEPROM_programmer.ino synkat).
4. ✅ Steg 6: ny sektion ”Nya komponenter — inga, samma hårdvara” + markeringen ”Lägg märke till: programmet körs på 6502-processorn”.
5. ✅ Terminologi: granskad — RWB förekommer bara i kod (makronamn, korrekt) och R/W i löptext (korrekt). Ingen ändring behövdes.
6. ✅ Print-CSS tillagd i style.css (@media print). SVG:erna hade redan role="img" — ingen ändring behövdes där.

**Innehållsarbete (större insats):**
7. ✅ Steg 7: ny sektion ”Så här fungerar en VIA” (register = brevlådor, DDR, minnesmappad I/O, chip select, fallande flank) + ”Vad är nytt i det här steget?”-checklista. Maskinkodstabellen var redan chunkad med delrubriker.
8. ✅ Steg 9: tabell över nya instruktioner (CLC/ADC/BCC/SBC/CMP/PHA/PLA/TXA/DEX/DEY/BNE/zero page) + spårat exempel ”Så här visas 144”.
9. ✅ ”Prova själv”-förslag i slutet av varje steg (h3 i Exempel på körning) — 11 sidor.
10. ✅ Ny begrepp.html (5 grupper, ~30 begrepp) + ”Vad du lär dig per steg”-karta och ”Verktyg och programvara” på index; begreppslänk i navet på alla sidor.
11. ✅ Enhetlig färglegend (blå/grön/gul/lila/grå) i minneskartorna på steg 6–11.
12. ✅ Personlig öppningsrad överst på varje stegsida — samma ”jag”-röst som på index.
13. ✅ Framåtblickande avslut (”Vad händer härnäst?”) före prev/next-länkarna på varje sida.
14. ✅ Steg 8: sektionsordning omstrukturerad så byggkedjan följer berättelsen (”Så här använder du det” fick också sin saknade section-wrapper). Steg 7: förbättrat med två nya sektioner; övergångsmeningar mellan varje enskilt referensblock återstår som kosmetisk finputs.
15. ✅ Seriellog-blocken i step2/5/8 → standardstilen (text-green-400, rounded-lg, overflow-x-auto).
16. ✅ ”Om det inte fungerar”-boxen borttagen på step10/11 (nu som övriga sidor, med inledningen ”Här är några saker att kontrollera:”). Konvention: standardsektioner utan box; färgade rutor reserveras för ⚠-varningar (steg 3), byggkedjekort (steg 8) och gammal/ny-jämförelser (steg 11).
17. ✅ step8: RS/E signal-spans i löptexten. step11:s ”Arduinon” verifierad — bestämd form fetas inte någonstans (samma praxis som steg 3), ingen ändring behövdes.
Bonus: step2–6.html:s visade kodblock synkade med den rättade adressformeln (K3-komplettering) — referenssidorna matchar nu repo-koden.

**Pågår hos projektägaren:** steg 11-schemat (bygger för närvarande på att det blir klart).

Inga ändringar görs utan godkännande — säg till vilka punkter du vill att jag utför (jag kan börja med de snabba fixarna 1–6).
