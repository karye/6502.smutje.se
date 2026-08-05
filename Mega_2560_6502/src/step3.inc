// Steg 3 — Databuss och minnesemulering
// Arduino emulerar minne. CPU:n kör NOP i loop från $8000.

#include <Arduino.h>

#define PHI2 2
#define RESB 4
#define RWB  3

uint8_t ram[1024];
uint8_t vectors[6];

uint8_t read_mem(uint16_t addr) {
  if (addr >= 0xFFFA && addr <= 0xFFFF) return vectors[addr - 0xFFFA];
  if (addr < 1024) return ram[addr];
  return 0xEA;  // NOP — ofarlig fallback
}

void write_mem(uint16_t addr, uint8_t val) {
  if (addr >= 0xFFFA && addr <= 0xFFFF) { vectors[addr - 0xFFFA] = val; return; }
  if (addr < 1024) ram[addr] = val;
}

void pulse() {
  digitalWrite(PHI2, LOW);
  delay(10);

  uint16_t a = (PINK << 0) | (PINF << 8);
  bool rw = digitalRead(RWB);

  if (rw) { DDRA = 0xFF; PORTA = read_mem(a); }
  else    { DDRA = 0x00; }

  digitalWrite(PHI2, HIGH);
  delay(10);

  if (!rw) write_mem(a, PINA);

  Serial.print(rw ? "R" : "W");
  Serial.print(" $");
  Serial.println(a, HEX);
}

void setup() {
  Serial.begin(115200);
  pinMode(PHI2, OUTPUT);
  pinMode(RESB, OUTPUT);
  pinMode(RWB,  INPUT);
  DDRK = 0x00; DDRF = 0x00; DDRA = 0x00;

  write_mem(0xFFFC, 0x00);  // Reset-vektor → $8000
  write_mem(0xFFFD, 0x80);
  write_mem(0x8000, 0xEA);  // NOP

  Serial.println("Steg 3 — Databuss och minnesemulering");

  digitalWrite(RESB, LOW);
  for (int i = 0; i < 5; i++) pulse();
  digitalWrite(RESB, HIGH);
  for (int i = 0; i < 10; i++) pulse();
}

void loop() { pulse(); }
