// Steg 4 — Manuell stegning med knappar
// Knapp 1 (D11): en klockcykel. Knapp 2 (D12): kör till nästa instruktion.

#include <Arduino.h>

#define PHI2     2
#define RESB     4
#define RWB      3
#define SYNC     13
#define BTN_CLK  11
#define BTN_INSTR 12

uint8_t ram[1024];
uint8_t vectors[6];

uint8_t read_mem(uint16_t addr) {
  if (addr >= 0xFFFA && addr <= 0xFFFF) return vectors[addr - 0xFFFA];
  if (addr < 1024) return ram[addr];
  return 0xEA;
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
  pinMode(SYNC, INPUT);
  pinMode(BTN_CLK,   INPUT_PULLUP);
  pinMode(BTN_INSTR, INPUT_PULLUP);
  DDRK = 0x00; DDRF = 0x00; DDRA = 0x00;

  write_mem(0xFFFC, 0x00);
  write_mem(0xFFFD, 0x80);
  write_mem(0x8000, 0xEA);

  Serial.println("Steg 4 — Knappstegning");

  digitalWrite(RESB, LOW);
  for (int i = 0; i < 5; i++) pulse();
  digitalWrite(RESB, HIGH);
}

void loop() {
  // Knapp 1: en klockcykel
  if (!digitalRead(BTN_CLK)) {
    delay(50);
    while (!digitalRead(BTN_CLK));
    delay(50);
    pulse();
  }

  // Knapp 2: kör till nästa instruktion (SYNC hög)
  if (!digitalRead(BTN_INSTR)) {
    delay(50);
    while (!digitalRead(BTN_INSTR));
    delay(50);
    do { pulse(); } while (!digitalRead(SYNC));
  }
}
