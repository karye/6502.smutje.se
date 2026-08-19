// ==============================================================
// Steg 11 — städad adressrymd
// ==============================================================
// SRAM: $0000–$7FFF (32 KB, hela 62256-chippet)
// VIA:  $8000–$BFFF (I/O-fönster, 16 KB)
// EEPROM: $C000–$FFFF
// Arduino: endast klocka + reset + diagnostik.

#include <Arduino.h>

#define PHI2     2
#define RESB     4
#define RWB      3
#define SYNC     13
#define BTN_CLK  11
#define BTN_INSTR 12

#define CLOCK_HZ 500
#define CLOCK_DELAY (1000 / (CLOCK_HZ * 2))

#define SRAM_END     0x7FFF   // NY — 32 KB nu
#define VIA_BASE     0x8000   // NY — flyttad från $4000
#define EEPROM_START 0xC000
#define EEPROM_END   0xFFFF

// --- is_sram() — hela nedre halvan, fysisk 62256 ---
bool is_sram(uint16_t addr) { return addr <= SRAM_END; }

// --- is_via() — I/O-fönster på $8000 (16 KB, register speglade) ---
bool is_via(uint16_t addr) {
  return (addr >= VIA_BASE && addr < 0xC000);
}

// --- is_eeprom() — toppen på $C000 ---
bool is_eeprom(uint16_t addr) {
  return (addr >= EEPROM_START && addr <= EEPROM_END);
}

// --- read_mem() — Arduino svarar ENDAST utanför alla fysiska enheter ---
uint8_t read_mem(uint16_t addr) {
  if (is_sram(addr))   return 0;   // SRAM svarar
  if (is_via(addr))    return 0;   // VIA svarar
  if (is_eeprom(addr)) return 0;   // EEPROM svarar
  return 0xEA;
}

// --- write_mem() — bara utanför fysiska enheter ---
void write_mem(uint16_t addr, uint8_t val) {
  if (is_sram(addr))   return;
  if (is_via(addr))    return;
  if (is_eeprom(addr)) return;
}

// --- pulse() — en klockcykel, tri-state för SRAM/VIA/EEPROM ---
void pulse() {
  digitalWrite(PHI2, LOW);
  DDRA = 0x00;
  delay(CLOCK_DELAY);

  uint8_t kl = PINF;
  uint8_t kh = PINK;
  uint16_t a = (kl << 0) | (kh << 8);
  bool rw = digitalRead(RWB);

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
  else if (a == 0xFFFC)      Serial.print("  ← RESET-VEKTOR LÅG");
  else if (a == 0xFFFD)      Serial.print("  ← RESET-VEKTOR HÖG");
  Serial.println();
}

// --- setup() — inget program att ladda, EEPROM levererar ---
void setup() {
  pinMode(RESB, OUTPUT); digitalWrite(RESB, LOW);
  pinMode(PHI2, OUTPUT); digitalWrite(PHI2, LOW);
  pinMode(SYNC, INPUT);
  pinMode(BTN_CLK,   INPUT_PULLUP);
  pinMode(BTN_INSTR, INPUT_PULLUP);
  pinMode(RWB, INPUT);
  Serial.begin(115200);
  DDRK = 0x00; DDRF = 0x00; DDRA = 0x00;

  Serial.println("Steg 11 — städad adressrymd");
  Serial.println("SRAM 32 KB ($0000-$7FFF), VIA ($8000), EEPROM ($C000+)");

  digitalWrite(RESB, LOW);
  for (int i = 0; i < 5; i++) pulse();
  digitalWrite(RESB, HIGH);

  Serial.println("CPU startad.");
}

// --- loop() — knappstyrning ---
void loop() {
  if (!digitalRead(BTN_CLK)) {
    delay(50); while (!digitalRead(BTN_CLK)); delay(50);
    pulse();
  }
  if (!digitalRead(BTN_INSTR)) {
    delay(50); while (!digitalRead(BTN_INSTR)); delay(50);
    do { pulse(); } while (!digitalRead(SYNC));
  }
  pulse();
}
