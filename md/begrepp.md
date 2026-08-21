# Begrepp och förkortningar

En uppslagsplats för alla ord du möter i serien — förklaringar som är korta nog att slå upp mitt i bygget, och tillräckligt djupa för att du ska förstå varför.

## Bussar och signaler

Adressbuss — 16 ledningar (A0–A15) som talar om var processorn vill läsa eller skriva. Värdet på bussen är en adress, t.ex. `$8000`.

Databuss — 8 dubbelriktade ledningar (D0–D7) som bär själva datan mellan kretsarna.

R/W (Read/Write) — signalen som talar om ifall processorn läser (HÖG) eller skriver (LÅG). Pinnen på processorn heter `RWB`.

Klocka (`PHI2`) — pulsen som driver processorn. W65C02S är *statisk CMOS*: klockan kan stoppas hur länge som helst utan att tillståndet försvinner.

Tri-state — ett läge där en krets inte driver en ledning alls (hög impedans), så att någon annan kan göra det i stället.

Pull-up / pull-down — ett motstånd till +5V (pull-up) eller GND (pull-down) som håller en signal på en känd nivå när inget annat driver den.

Fallande flank — ögonblicket då en signal går från HÖG till LÅG. LCD:n läser databussen just då.

Chip select — en ingång som talar om för en krets om just den ska svara på bussen just nu.

## Minne

Adressrymd — alla 65 536 möjliga adresser (`$0000`–`$FFFF`) som processorn kan nå.

Reset-vektor — adresserna `$FFFC`/`$FFFD` där processorn efter reset läser var programmet börjar.

Hex — hexadecimala tal, bas 16, med prefixet `$`. `$FF` = 255, `$8000` = 32 768.

Little-endian — ordningen där en 16-bitars adress lagras med låg byte först (`$8000` skrivs `00 80`).

Zero page — minnesområdet `$0000`–`$00FF`; nås med kortare och snabbare instruktioner.

Stacken — minnesområdet `$0100`–`$01FF` där `JSR`/`PHA`/`RTS` automatiskt sparar och hämtar data.

RAM / SRAM — arbetsminne som försvinner när strömmen bryts. 62256 är 32 KB SRAM.

ROM / EEPROM — beständigt minne. AT28C256 är 32 KB EEPROM som kan raderas och skrivas om elektriskt.

Minnesmappad I/O — när en I/O-krets svarar på adresser som om den vore minne: att skriva till adressen styr en pinne.

Spegling (mirroring) — när en krets svarar på fler adresser än den behöver, eftersom chip select bara tittar på ett fåtal adressbitar.

## Program

Opcode — den första byten i en instruktion; talet som talar om vad processorn ska göra.

Operand — de följande 0–2 byten; data eller adress som instruktionen behöver.

Assembler — mänskligt läsbar 6502-kod (`LDX`, `STA`…) som översätts till maskinkod.

ca65 / ld65 — assembler och länkare från cc65-verktygslådan; bygger `.asm` till `.bin`.

Label — ett namn på en minnesadress i assembler, t.ex. `reset:` eller `fib_loop`.

Subrutin — ett namngivet kodblock som anropas med `JSR` och återvänder med `RTS`.

Carry-flagga — en minnesbit som minns om förra additionen rymdes; grunden för 16-bitars räkning i en 8-bitars CPU.

Branch — ett villkorligt hopp som beror på en flagga (`BEQ`, `BCC`, `BNE`…).

## Kretsar

VIA (W65C22) — I/O-krets med två 8-bitars portar (PA, PB) och kontrollpinnar; sitter på processorns bussar.

DDR — Data Direction Register; bestämmer om varje portpinne är utgång (1) eller ingång (0).

Port — en grupp pinnar som kretsen styr eller avläser som en byte.

74HC00 — krets med fyra NAND-grindar (14 pinnar); används här som adressavkodare.

LCD — flytande kristall-display (HD44780-kontroller) med parallell databuss och RS/E-signaler.

Avkopplingskondensator — en liten kondensator (100 nF) nära varje krets som filtrerar bort brus på strömförsörjningen.

## Bygget

Kopplingsdäck — plåtar med förbundna hål som kopplar ihop komponenter utan lödning.

Kopplingstråd — trådar som förbinder hål på kopplingsdäcket.

Multimeter — instrument för att mäta spänning, ström och resistans; felsökningens viktigaste verktyg.
