// ==============================================================
// Steg 3 — Databuss och minnesemulering
// ==============================================================
// Arduino blir en fullständig minnesemulator. CPU:n får ett
// NOP-program på $8000 och loopar för evigt — varje minnes-
// access loggas i serieövervakaren.

#include <Arduino.h>

// --- Pindefinitioner ---
#define PHI2 2   // Arduino D2 → CPU pin 37 (klocka)
#define RESB 4   // Arduino D4 → CPU pin 40 (reset)
#define RWB  3   // Arduino D3 → CPU pin 34 (Read/Write)

#define CLOCK_HZ 500                         // Klockfrekvens i Hz
#define CLOCK_DELAY (1000 / (CLOCK_HZ * 2))  // ms per klockfas

// --- Minnesarrayer ---
uint8_t ram[1024];       // $0000–$03FF (zero page + stack + ledigt)
uint8_t vectors[6];      // $FFFA–$FFFF (NMI, RESET, IRQ)

// --- read_mem() — slå upp ett minnesvärde ---
// Anropas när CPU:n vill läsa (RWB = HIGH).
// Returnerar rätt byte för varje adressområde.
uint8_t read_mem(uint16_t addr) {
  // Vektorer ($FFFA–$FFFF)
  if (addr >= 0xFFFA && addr <= 0xFFFF) return vectors[addr - 0xFFFA];
  // RAM ($0000–$03FF)
  if (addr < 1024) return ram[addr];
  // Allt annat → NOP ($EA) — ofarlig fallback, CPU:n hoppar bara över
  return 0xEA;
}

// --- write_mem() — spara ett värde i minnet ---
// Anropas när CPU:n vill skriva (RWB = LOW).
void write_mem(uint16_t addr, uint8_t val) {
  if (addr >= 0xFFFA && addr <= 0xFFFF)
    { vectors[addr - 0xFFFA] = val; return; }
  if (addr < 1024) ram[addr] = val;
}

// --- pulse() — en klockcykel med minnesemulering ---
// Detta är hjärtat i hela datorn. Varje cykel:
// 1. Läs adressbuss (PORTF + PORTK)
// 2. Läs R/W-signal
// 3. Om CPU läser: lägg ut rätt byte på databussen (PORTA)
// 4. Om CPU skriver: läs av vad som skrevs och spara
void pulse() {
  // --- Fas 1: PHI2 låg ---
  digitalWrite(PHI2, LOW);
  delay(CLOCK_DELAY);

  uint16_t a = (PINF << 0) | (PINK << 8);  // Läs adressbuss
  bool rw = digitalRead(RWB);               // Läs R/W

  // Styr databussen baserat på R/W
  if (rw) {
    // CPU läser → Arduino driver PORTA med rätt värde
    DDRA = 0xFF;               // PORTA = OUTPUT
    PORTA = read_mem(a);       // Lägg ut byten på databussen
  } else {
    // CPU skriver → Arduino lyssnar (tri-state, hög impedans)
    DDRA = 0x00;               // PORTA = INPUT
  }

  // --- Fas 2: PHI2 hög ---
  digitalWrite(PHI2, HIGH);
  delay(CLOCK_DELAY);

  // Om CPU:n skrev — spara värdet som låg på databussen
  if (!rw) write_mem(a, PINA);

  // Diagnostik: skriv ut vad som hände
  Serial.print(rw ? "R" : "W");
  Serial.print(" $");
  Serial.println(a, HEX);
}

// --- setup() — ladda program och starta CPU ---
void setup() {
  Serial.begin(115200);
  pinMode(PHI2, OUTPUT);
  pinMode(RESB, OUTPUT);
  pinMode(RWB,  INPUT);
  DDRK = 0x00;                 // Adressbuss A8–A15 = INPUT
  DDRF = 0x00;                 // Adressbuss A0–A7  = INPUT
  DDRA = 0x00;                 // Databuss = INPUT (tri-state från start)

  // --- Ladda programmet i minnet ---
  // Reset-vektor: CPU:n startar på $8000
  write_mem(0xFFFC, 0x00);     // Låg byte av $8000
  write_mem(0xFFFD, 0x80);     // Hög byte av $8000

  // Program på $8000: en NOP-instruktion ($EA)
  // NOP gör ingenting — CPU:n går till nästa adress.
  // Nästa adress är också $EA → oändlig loop.
  write_mem(0x8000, 0xEA);

  Serial.println("Steg 3 — Databuss och minnesemulering");

  // Reset-sekvens
  digitalWrite(RESB, LOW);
  for (int i = 0; i < 5; i++) pulse();
  digitalWrite(RESB, HIGH);

  // Kör 10 cykler så vi ser reset-vektorn i serieövervakaren
  for (int i = 0; i < 10; i++) pulse();
}

// --- loop() — fortsätt köra, en klockcykel i taget ---
void loop() {
  pulse();
}
