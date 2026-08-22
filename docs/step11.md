# Städad adressrymd

En dator som fungerar men har en stökig adressrymd stör mig. Nu ska hela 64 KB falla på plats — RAM nedtill, I/O i mitten, ROM upptill.
## Mål

I steg 10 fick jag en fristående dator — men adressrymden var inte vacker. VIA:ns 16 bytes på `$4000` klippte av SRAM-minnet mitt i, och den övre halvan av 62256-chippet (`$4000`–`$7FFF`) låg helt oanvänd. Det kändes slarvigt.

Nu städar jag upp. SRAM utökas till hela 32 KB (`$0000`–`$7FFF`) genom att ansluta `A14` — hela 62256-chippet används äntligen. VIA:n flyttas till `$8000`–`$BFFF`, ett 16 KB-stort I/O-fönster i mitten av adressrymden, precis som I/O-portar sitter på riktiga 6502-datorer (Apple II hade I/O på `$C000`, C64 på `$D000`).

Resultatet är en adressrymd utan dött utrymme: RAM längst ner, ROM i mitten och I/O i ett fönster högst upp. Det här är den klassiska 6502-layouten — nu har jag byggt den själv. Fyra kablar och en NAND-grind — och hela 64 KB har fått ett jobb.

Samma väg som *Steve Wozniak* använde i Apple II och som *Ben Eater* visade i sin klassiska 6502-serie.
## Nya komponenter

Inga nya kretsar! Det här steget handlar om att *omkoppla det jag redan har*. Jag behöver bara en enda NAND-grind (¼ av en 74HC00) för hela den nya adressavkodningen. Dessutom en extra kopplingstråd för SRAM:ns `A14` och en för EEPROM:ets `A14`.

| Antal | Komponent | Används till |
|---|---|---|
| 1 | 74HC00 (en grind, ¼ av chippet) | EEPROM /CE = NAND(A15, A14) |
| 1 | Kopplingstråd | SRAM A14 (pin 1) → CPU A14 — öppnar den andra halvan av 62256 |
| — | Kopplingstrådar (omkoppling) | Flytta VIA:ns /CS2 och EEPROM:ets /CE till nya avkodningssignaler |
## Kopplingsschema

Schemat visar den städade bussen: SRAM tar nedre halvan, VIA mitten och EEPROM toppen — varje enhet på sin egen tydliga region.

Schema kommer snart — se avkodningstabellerna nedan så länge.
## Adressavkodning

Allt handlar om vilken enhet som får prata på bussen. Tre enheter, tre chip select-signaler — och en enda NAND-grind räcker.
### Gamla kartan (steg 10) — problemen

✗ SRAM bara 16 KB: 62256 är ett 32 KB-chip, men `A14` var inte ansluten. Den övre halvan låg död.

✗ VIA klippte av minnet: 16 bytes på `$4000` mitt i adressrymden delade RAM:et i två — `$0000`–`$3FFF` använt, `$4000`–`$7FFF` dött.

✗ EEPROM-avkodning enkel: bara NOT `A15` — fungerade, men kunde inte samexistera med VIA på samma sida.

### Nya kartan (steg 11)

✓ SRAM 32 KB (`$0000`–`$7FFF`): `A14` ansluts, `/CE` = `A15` direkt (aktivt låg) — hela chippet aktivt när `A15`=0.

✓ VIA på `$8000`–`$BFFF`: `A15` → `CS1` (aktivt hög), `A14` → `/CS2` (aktivt låg) — *inga grindar alls*, kretsens egna chip-selects gör jobbet.

✓ EEPROM på `$C000`–`$FFFF`: `/CE` = NAND(`A15`, `A14`) — en enda grind, och `A14` kopplas även till EEPROM:ets `A14` så chippet läser sin övre halva.

### Sanningstabell — vem svarar på vilken adress?

Varje enhet har en egen region — inga fönster, inga speglingar mellan enheter. Adressavkodningen är bara fyra kablar och en NAND-grind.

| Adressintervall | A15 | A14 | SRAM /CE | VIA CS1·/CS2 | EEPROM /CE | Vem svarar |
|---|---|---|---|---|---|---|
| $0000–$7FFF | 0 | – | LÅG ✓ | – | HÖG | SRAM |
| $8000–$BFFF | 1 | 0 | HÖG | AKTIV ✓ | HÖG | VIA |
| $C000–$FFFF | 1 | 1 | HÖG | – | LÅG ✓ | EEPROM |
| VIA: CS1 = A15 (aktivt hög) och /CS2 = A14 (aktivt låg) — aktiveras när A15=1 och A14=0. A0–A3 går till RS0–RS3 och väljer register; de speglas var 16:e byte i fönstret. EEPROM: A14 kopplas också till chippets A14 så det läser övre halvan. |  |  |  |  |  |  |
## Kopplingar

Här är vad som ändras jämfört med steg 10. Allt annat — ström, klocka, reset, adress-/databuss till SRAM och EEPROM — är oförändrat.
### SRAM — nu med `A14`
| Pin | Signal | Kopplas till | Varför |
|---|---|---|---|
| 1 | `A14` | CPU A14 (NY!) | Öppnar den andra halvan av 62256 — SRAM blir 32 KB |
| 20 | `/CE` | CPU `A15` direkt (aktivt låg) | LÅG när A15=0 — hela nedre halvan, ingen grind behövs |
### VIA — flyttad till `$8000`
| Pin | Signal | Kopplas till | Varför |
|---|---|---|---|
| 23 | `/CS2` | CPU `A14` direkt (aktivt låg) | LÅG när A14=0 → VIA vid $8000–$BFFF |
| 24 | `CS1` | CPU `A15` direkt (aktivt hög) | HÖG när A15=1 → övre halvan |
| 38–35 | `RS0–RS3` | CPU A0–A3 (oförändrad) | Väljer register — de speglas var 16:e byte i fönstret |
### EEPROM — övre halvan (`$C000`–`$FFFF`)
| Pin | Signal | Kopplas till | Varför |
|---|---|---|---|
| 20 | `/CE` | NAND-grindens utgång (74HC00 pin 3) | LÅG när A15=1 och A14=1 → $C000–$FFFF |
| 1 | `A14` | CPU `A14` | EEPROM läser sin övre halva — $C000–$FFFF |
### Adressavkodning — en enda NAND-grind

Här är tricket: kretsarnas egna chip-select-pinnar gör avkodningen. SRAM:s `/CE` är aktivt låg, och VIA:n har *två* chip-selects med motsatt polaritet — `CS1` (pin 24, aktivt hög) och `/CS2` (pin 23, aktivt låg). Då matchar adressbitarna kretsarna direkt, och bara EEPROM:s `/CE` behöver en NAND-grind. Aktiveringslogiken finns i sanningstabellen i Adressavkodning-sektionen ovan, och de fyra direktanslutningarna — SRAM `/CE`, VIA `CS1`/`/CS2` och EEPROM `A14` — står i kretstabellerna ovan. Här visas bara NAND-grinden. 

| Pin | Signal | Kopplas till | Varför |
|---|---|---|---|
| 74HC00 — endast en NAND-grind |  |  |  |
| 1, 2 | `A15`, `A14` (in) | CPU `A15`, CPU `A14` | NAND(`A15`, `A14`) — HÖG när någon är 0 |
| 3 | EEPROM `/CE` (ut) | AT28C256 pin 20 | LÅG när A15=1 och A14=1 → $C000–$FFFF |
| 14, 7 | VCC, GND | +5V, GND | Strömmatning |

## Minnestarta

Nu används hela 64 KB, varje enhet på sin egen tydliga region: **SRAM** nedtill, **VIA** i mitten, **EEPROM** upptill.

<div class="memmap">

        <div style="height:100px;display:flex;align-items:stretch;border-bottom:2px solid #d1d5db;border-bottom:2px solid #a855f7">
          <div style="min-height:8px;width:5rem;padding-right:.75rem;display:flex;flex-direction:column;justify-content:space-between;text-align:right">
            <span style="color:#6b7280">$FFFF</span>
            <small>$C000</small>
          </div>
          <div style="flex:1 1 0%;background-color:#f3e8ff;border-left:1px solid #a78bfa;border-right:1px solid #a78bfa;padding-left:.5rem;padding-right:.5rem;display:flex;align-items:center">AT28C256 EEPROM — program + vektorer</div>
          <div style="width:3.5rem;text-align:right;color:#6b7280;padding-right:.5rem">16 KB</div>
        </div>
        <div style="height:100px;display:flex;align-items:stretch;border-bottom:2px solid #d1d5db;border-bottom:2px solid #eab308">
          <div style="min-height:16px;width:5rem;padding-right:.75rem;display:flex;flex-direction:column;justify-content:space-between;text-align:right">
            <span style="color:#6b7280">$BFFF</span>
            <small>$8000</small>
          </div>
          <div style="flex:1 1 0%;background-color:#fef9c3;border-left:1px solid #facc15;border-right:1px solid #facc15;padding-left:.5rem;padding-right:.5rem;display:flex;align-items:center;font-weight:700">W65C22 VIA — I/O-fönster</div>
          <div style="width:3.5rem;text-align:right;color:#6b7280;padding-right:.5rem">16 KB</div>
        </div>
        <div style="height:200px;display:flex;align-items:stretch;border-bottom:2px solid #d1d5db;border-bottom:2px solid #22c55e">
          <div style="min-height:8px;width:5rem;padding-right:.75rem;display:flex;flex-direction:column;justify-content:space-between;text-align:right">
            <span style="color:#6b7280">$7FFF</span>
            <small>$0000</small>
          </div>
          <div style="flex:1 1 0%;background-color:#dcfce7;border-left:1px solid #4ade80;border-right:1px solid #4ade80;padding-left:.5rem;padding-right:.5rem;display:flex;align-items:center;font-weight:700">62256 SRAM — 32 KB (hela chippet)</div>
          <div style="width:3.5rem;text-align:right;color:#6b7280;padding-right:.5rem">32 KB</div>
        </div>
      
</div>

## Arduino-kod

Ändringarna i Arduinon är minimala — bara tre konstanter och en villkorsändring i `is_eeprom()`. Koden vet nu att SRAM täcker `$0000`–`$7FFF`, VIA bor på `$8000` och EEPROM på `$C000`. Allt annat är tri-state för fysiska kretsar.
???+ note "📦 Arduino-kod"
    ```cpp
    --8<-- "Mega_2560_6502/src/step1.inc"
    ```

## Länkskript — program.cfg

I steg 8 skapade jag `program.cfg` — länkaren ld65:s karta över minnet. Den talar om var i adressrymden koden hamnar. Nu uppdateras den: ROM:en ligger nu rent på `$C000`–`$FFFF` (16 KB) — inga fönster eller gap längre.

### Så här gör jag

1. Jag öppnar `asm/program.cfg` i mitt PlatformIO-projekt — samma fil som skapades i steg 8.
1. Jag ersätter innehållet med koden nedan och sparar.
1. Jag bygger projektet — `build_asm.py` skickar filen till ld65 automatiskt (`-C program.cfg`). Inget att köra manuellt.
1. Assembler-filen behöver inga ändringar — segmenten nedan matchar den.
???+ note "📦 Länkskript"
    ```cfg
    --8<-- "Mega_2560_6502/asm/program.cfg"
    ```

Koden läggs i `$C000`–`$FFFF` (ROM, 16 KB). Reset-vektorn ligger som alltid på `$FFFA`–`$FFFF`.

## Exempel på körning

När jag laddat upp koden och startat datorn ser jag i seriemonitorn att enheterna svarar på sina nya adresser:

<div class="monlcd">
<div>
<p class="xlabel"><strong>Seriemonitor (115200 baud)</strong></p>

```text
Steg 11 — städad adressrymd
SRAM 32 KB ($0000-$7FFF), VIA ($8000), EEPROM ($C000+)
CPU startad.

R $FFFC  ← RESET-VEKTOR LÅG (EEPROM)
R $FFFD  ← RESET-VEKTOR HÖG (EEPROM)
R $C000  ← EEPROM
R $C001  ← EEPROM
W $0200  ← SRAM            (variabel skrivning)
W $8000  ← VIA: 01         (LCD-kommando via VIA)
W $8001  ← VIA: 05         (registerval)
...
W $8000  ← VIA: 3D         (tecknet "=")
...
```

</div>
<div>
<p class="xlabel"><strong>LCD-displayen (16×2)</strong></p>

<div class="lcd"><div class="lcd-badge">LCD 16×2</div><div class="lcd-screen"><div>=== 6502 VIA LCD ===</div><div>Hello from W65C02!</div></div></div>

</div>
</div>

LCD-displayen (16×2)

Samma program som steg 8–10, men VIA:n adresseras nu på `$8000` istället för `$4000`.

Lägg märke till skillnaden mot steg 10: SRAM-skrivningar kan nu ske var som helst i `$0000`–`$7FFF`, VIA:n svarar på `$8000` och EEPROM på `$C000`. Bussloggen visar tydligt vilken enhet som pratar.
## Så här felsöker jag

Här är några saker jag kontrollerar:

- Läser CPU:n `$FFFC`/`$FFFD` men hoppar till fel adress? Då är EEPROM:ets `/CE` förmodligen inte rätt avkodat. Jag kontrollerar NAND-grinden (74HC00 pin 3) — den måste ge LÅG när `A15`=1 och `A14`=1.
- Svarar VIA:n inte på `$8000`? Då kontrollerar jag `CS1` (pin 24) = `A15` (hög) och `/CS2` (pin 23) = `A14` (låg) — VIA:n kräver båda samtidigt. Jag dubbelkollar att `RS0–RS3` fortfarande går till `A0–A3`.
- Är SRAM fortfarande bara 16 KB? Då är `A14` (SRAM pin 1) inte ansluten. Utan den kan processorn inte nå `$4000`–`$7FFF`.
- Busskrockar? Då kan två enheter svara samtidigt om avkodningen är fel. Jag mäter `/CE` och `/CS2` medan CPU:n läser — exakt en ska vara LÅG.
- Kraschar programmet vid `$8000`? Då placerade länkskriptet kod i VIA-fönstret. Jag kontrollerar `program.cfg` — ROM ska börja på `$C000`, inte tidigare.

