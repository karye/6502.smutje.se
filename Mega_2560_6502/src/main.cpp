#include <Arduino.h>
#include <LiquidCrystal.h>

// -----------------------------------------------------------
// Steg 3 — Databuss och fullständig minnesemulering
// -----------------------------------------------------------

#define PHI2  2   // CPU pin 37
#define RESB  4   // CPU pin 40
#define RWB   3   // CPU pin 34

uint8_t  ram[512];
uint8_t  vectors[6];  // $FFFA–$FFFF

uint8_t read_mem(uint16_t addr) {
  if (addr >= 0xFFFA && addr <= 0xFFFF)
    return vectors[addr - 0xFFFA];
  if (addr < 512) return ram[addr];
  return 0xEA;
}

void write_mem(uint16_t addr, uint8_t val) {
  if (addr >= 0xFFFA && addr <= 0xFFFF) {
    vectors[addr - 0xFFFA] = val;
    return;
  }
  if (addr < 512) ram[addr] = val;
}

// LCD — 4-bit, flyttad till D8–D13 för att lämna D5–D7 fria
//        RS   E   D4  D5  D6  D7
LiquidCrystal lcd(5, 6, 10, 9, 8, 7);

uint16_t lcdAddr;
bool     lcdRw;

void pulse();  // Forward

void setup() {
  // --- LCD ---
  lcd.begin(16, 2);
  lcd.print("W65C02S Steg 3");
  lcd.setCursor(0, 1);
  lcd.print("CPU redo");

  // --- Håll CPU i reset från start ---
  pinMode(RESB, OUTPUT); digitalWrite(RESB, LOW);
  pinMode(PHI2, OUTPUT); digitalWrite(PHI2, LOW);
  Serial.begin(115200);
  pinMode(RWB, INPUT);

  DDRK = 0x00;  // A8–A15
  DDRF = 0x00;  // A0–A7
  DDRA = 0x00;

  // --- Reset-vektor → $8000 ---
  write_mem(0xFFFC, 0x00);
  write_mem(0xFFFD, 0x80);
  write_mem(0x8000, 0xEA);  // NOP

  // --- Reset-sekvens ---
  for (int i = 0; i < 5; i++) pulse();
  digitalWrite(RESB, HIGH);
  for (int i = 0; i < 15; i++) pulse();
}

void pulse() {
  digitalWrite(PHI2, LOW);
  delay(25);

  uint16_t a = (PINF << 0) | (PINK << 8);
  bool rw = digitalRead(RWB);

  // Spara för LCD
  lcdAddr = a; lcdRw = rw;

  if (rw) { DDRA = 0xFF; PORTA = read_mem(a); }
  else    { DDRA = 0x00; }

  digitalWrite(PHI2, HIGH);
  delay(25);

  if (!rw) write_mem(a, PINA);

  Serial.print(rw ? "R" : "W");
  Serial.print(" $");
  Serial.println(a, HEX);

  // Spara för LCD
  lcdAddr = a; lcdRw = rw;
}

void loop() {
  pulse();  // Kontinuerlig körning
}