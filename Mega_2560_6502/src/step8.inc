// Steg 8 — Assembler-bygge med ca65
// 6502-programmet byggs från src/program.asm via ca65/ld65 → program.h
// Vid bygge anropas scripts/build_asm.py automatiskt

#include <Arduino.h>
#include "program.h"

#define PHI2     2
#define RESB     4
#define RWB      3
#define SYNC     13
#define BTN_CLK  11
#define BTN_INSTR 12

uint8_t ram[1024];
uint8_t program[2048];
uint8_t vectors[6];

#define VIA_BASE 0xC000

bool is_via(uint16_t addr) {
  return (addr >= VIA_BASE && addr < VIA_BASE + 16);
}

uint8_t read_mem(uint16_t addr) {
  if (is_via(addr)) return 0;
  if (addr >= 0xFFFA && addr <= 0xFFFF)
    return vectors[addr - 0xFFFA];
  if (addr >= 0x8000 && addr < 0x8000 + sizeof(program))
    return program[addr - 0x8000];
  if (addr < sizeof(ram)) return ram[addr];
  return 0xEA;
}

void write_mem(uint16_t addr, uint8_t val) {
  if (is_via(addr)) return;
  if (addr >= 0xFFFA && addr <= 0xFFFF)
    { vectors[addr - 0xFFFA] = val; return; }
  if (addr >= 0x8000 && addr < 0x8000 + sizeof(program))
    { program[addr - 0x8000] = val; return; }
  if (addr < sizeof(ram)) ram[addr] = val;
}

void pulse() {
  digitalWrite(PHI2, LOW);
  DDRA = 0x00;
  delay(10);

  uint8_t kl = PINF;
  uint8_t kh = PINK;
  uint16_t a = (kl << 0) | (kh << 8);
  bool rw = digitalRead(RWB);

  if (rw && !is_via(a)) {
    DDRA = 0xFF;
    PORTA = read_mem(a);
  }

  digitalWrite(PHI2, HIGH);
  delay(10);

  if (!rw && !is_via(a)) write_mem(a, PINA);

  Serial.print(rw ? "R" : "W");
  Serial.print(" $");
  Serial.print(a, HEX);
  if (a >= 0x4000 && !rw) { Serial.print("  <- VIA: "); Serial.print(PINA, HEX); }
  else if (a >= 0x8000 && rw)  Serial.print("  <- OPCODE");
  else if (a == 0xFFFC)        Serial.print("  <- RESET-VEKTOR LAG");
  else if (a == 0xFFFD)        Serial.print("  <- RESET-VEKTOR HOG");
  Serial.println();
}

void setup() {
  pinMode(RESB, OUTPUT); digitalWrite(RESB, LOW);
  pinMode(PHI2, OUTPUT); digitalWrite(PHI2, LOW);
  pinMode(SYNC, INPUT);
  pinMode(BTN_CLK,   INPUT_PULLUP);
  pinMode(BTN_INSTR, INPUT_PULLUP);
  pinMode(RWB, INPUT);
  Serial.begin(115200);

  DDRK = 0x00; DDRF = 0x00; DDRA = 0x00;

  // Ladda 6502-programmet från assembler-bygget
  Serial.println("Steg 8 — Assembler-bygge med ca65");
  Serial.print("Programstorlek: ");
  Serial.print(PROGRAM_SIZE);
  Serial.println(" bytes");

  // Ladda programkod ($8000–$87FF): PROGRAM[] minus de sista 6 vektor-byten
  unsigned int code_size = PROGRAM_SIZE - 6;
  for (unsigned int i = 0; i < code_size; i++) {
    write_mem(0x8000 + i, pgm_read_byte(&PROGRAM[i]));
  }

  // Ladda vektorer ($FFFA–$FFFF) från slutet av PROGRAM[]
  for (unsigned int i = 0; i < 6; i++) {
    write_mem(0xFFFA + i, pgm_read_byte(&PROGRAM[code_size + i]));
  }

  Serial.println("Program + vektorer laddat.");

  // Reset-sekvens
  for (int i = 0; i < 5; i++) pulse();
  digitalWrite(RESB, HIGH);

  // Kör ~2000 cykler för hela programmet
  for (int i = 0; i < 2000; i++) pulse();
}

void loop() {
  pulse();
}
