// ==============================================================
// Steg 10 — EEPROM som ROM
// ==============================================================
// Arduino är nu ENDAST klocka + reset + diagnostik.
// $8000–$FFFF hanteras av AT28C256 EEPROM.
// SRAM ($0000–$3FFF) och VIA ($4000–$7FFF) är oförändrade.

#include <Arduino.h>

#define PHI2     2
#define RESB     4
#define RWB      3
#define SYNC     13
#define BTN_CLK  11
#define BTN_INSTR 12

#define CLOCK_HZ 500
#define CLOCK_DELAY (1000 / (CLOCK_HZ * 2))

#define VIA_BASE     0x4000
#define SRAM_END     0x3FFF
#define EEPROM_START 0x8000
#define EEPROM_END   0xFFFF

// --- is_sram() — hanteras av fysisk 62256 ---
bool is_sram(uint16_t addr) { return addr <= SRAM_END; }

// --- is_via() — hanteras av fysisk W65C22 ---
// VIA sitter i fönstret $4000–$7FFF (register speglas var 16:e byte).
bool is_via(uint16_t addr) {
  return (addr >= VIA_BASE && addr < 0x8000);
}

// --- is_eeprom() — hanteras av fysisk AT28C256 (NY!) ---
bool is_eeprom(uint16_t addr) {
  return (addr >= EEPROM_START && addr <= EEPROM_END);
}

// --- read_mem() — Arduino svarar ENDAST utanför SRAM/VIA/EEPROM ---
uint8_t read_mem(uint16_t addr) {
  if (is_sram(addr))   return 0;   // SRAM svarar
  if (is_via(addr))    return 0;   // VIA svarar
  if (is_eeprom(addr)) return 0;   // EEPROM svarar (NY!)
  return 0xEA;                      // Fallback: NOP
}

// --- write_mem() — spara, men bara utanför SRAM/VIA/EEPROM ---
void write_mem(uint16_t addr, uint8_t val) {
  if (is_sram(addr))   return;     // SRAM tar emot
  if (is_via(addr))    return;     // VIA tar emot
  if (is_eeprom(addr)) return;     // EEPROM är read-only (NY!)
}

// --- pulse() — en klockcykel, Arduino tri-state för SRAM/VIA/EEPROM ---
void pulse() {
  digitalWrite(PHI2, LOW);
  DDRA = 0x00;
  delay(CLOCK_DELAY);

  uint8_t kl = PINF;
  uint8_t kh = PINK;
  uint16_t a = (kl << 0) | (kh << 8);
  bool rw = digitalRead(RWB);

  // Arduino svarar ENDAST om adressen inte är SRAM, VIA eller EEPROM
  if (rw && !is_sram(a) && !is_via(a) && !is_eeprom(a)) {
    DDRA = 0xFF;
    PORTA = read_mem(a);
  }

  digitalWrite(PHI2, HIGH);
  delay(CLOCK_DELAY);

  if (!rw && !is_sram(a) && !is_via(a) && !is_eeprom(a))
    write_mem(a, PINA);

  // Diagnostik
  Serial.print(rw ? "R" : "W");
  Serial.print(" $");
  Serial.print(a, HEX);
  if (is_sram(a))            Serial.print("  ← SRAM");
  else if (is_via(a))         Serial.print("  ← VIA");
  else if (is_eeprom(a))      Serial.print("  ← EEPROM");
  else if (a == 0xFFFC)      Serial.print("  ← RESET-VEKTOR LÅG (EEPROM)");
  else if (a == 0xFFFD)      Serial.print("  ← RESET-VEKTOR HÖG (EEPROM)");
  Serial.println();
}

// --- setup() — inget program att ladda! ---
void setup() {
  pinMode(RESB, OUTPUT); digitalWrite(RESB, LOW);
  pinMode(PHI2, OUTPUT); digitalWrite(PHI2, LOW);
  pinMode(SYNC, INPUT);
  pinMode(BTN_CLK,   INPUT_PULLUP);
  pinMode(BTN_INSTR, INPUT_PULLUP);
  pinMode(RWB, INPUT);
  Serial.begin(115200);
  DDRK = 0x00; DDRF = 0x00; DDRA = 0x00;

  Serial.println("Steg 10 — EEPROM som ROM");
  Serial.println("Programmet ligger i AT28C256 — inte i Arduino.");

  // Reset-sekvens
  digitalWrite(RESB, LOW);
  for (int i = 0; i < 5; i++) pulse();
  digitalWrite(RESB, HIGH);

  Serial.println("CPU startad. EEPROM levererar programkoden.");
}

// --- loop() — knappstyrning som tidigare ---
void loop() {
  if (!digitalRead(BTN_CLK)) {
    delay(50); while (!digitalRead(BTN_CLK)); delay(50);
    pulse();
  }
  if (!digitalRead(BTN_INSTR)) {
    delay(50); while (!digitalRead(BTN_INSTR)); delay(50);
    do { pulse(); } while (!digitalRead(SYNC));
  }
  // Utan knapptryck: kör fritt
  pulse();
}
