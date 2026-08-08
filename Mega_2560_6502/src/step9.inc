// Steg 9 — Riktigt RAM (62256 32KB SRAM) + VIA + LCD
// Samma knapp-logik som step7. Enda skillnad: is_sram() tri-state för $0000–$3FFF.

#include <Arduino.h>

#define PHI2     2
#define RESB     4
#define RWB      3
#define SYNC     13
#define BTN_CLK  11
#define BTN_INSTR 12

uint8_t program[2048];      // $8000–$87FF
uint8_t vectors[6];         // $FFFA–$FFFF

#define VIA_BASE     0x4000
#define SRAM_END     0x3FFF

bool is_sram(uint16_t addr) { return addr <= SRAM_END; }
bool is_via(uint16_t addr)  { return (addr >= VIA_BASE && addr < VIA_BASE + 16); }

uint8_t read_mem(uint16_t addr) {
  if (is_sram(addr)) return 0;
  if (is_via(addr))  return 0;
  if (addr >= 0xFFFA && addr <= 0xFFFF)
    return vectors[addr - 0xFFFA];
  if (addr >= 0x8000 && addr < 0x8000 + sizeof(program))
    return program[addr - 0x8000];
  return 0xEA;
}

void write_mem(uint16_t addr, uint8_t val) {
  if (is_sram(addr)) return;
  if (is_via(addr))  return;
  if (addr >= 0xFFFA && addr <= 0xFFFF)
    { vectors[addr - 0xFFFA] = val; return; }
  if (addr >= 0x8000 && addr < 0x8000 + sizeof(program))
    { program[addr - 0x8000] = val; return; }
}

void pulse() {
  digitalWrite(PHI2, LOW);
  DDRA = 0x00;
  delay(10);

  uint8_t kl = PINF;
  uint8_t kh = PINK;
  uint16_t a = (kl << 0) | (kh << 8);
  bool rw = digitalRead(RWB);

  if (rw && !is_sram(a) && !is_via(a)) {
    DDRA = 0xFF;
    PORTA = read_mem(a);
  }

  digitalWrite(PHI2, HIGH);
  delay(10);

  if (!rw && !is_sram(a) && !is_via(a)) write_mem(a, PINA);

  Serial.print(rw ? "R" : "W");
  Serial.print(" $"); Serial.print(a, HEX);
  if (is_sram(a))            Serial.print(" SRAM");
  else if (is_via(a))        Serial.print(" VIA");
  else if (a >= 0x8000)      Serial.print(" OPCODE");
  else if (a == 0xFFFC)      Serial.print(" RSTL");
  else if (a == 0xFFFD)      Serial.print(" RSTH");
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

  Serial.println("Steg 9 — SRAM + VIA + LCD (med knappar)");

  // Reset-vektor → $8000
  write_mem(0xFFFC, 0x00);
  write_mem(0xFFFD, 0x80);

  // === EXAKT samma 6502-program som step7 ===
  uint16_t next = 0x8000;

  write_mem(next++, 0xA9); write_mem(next++, 0xFF);  // LDA #$FF
  write_mem(next++, 0x8D); write_mem(next++, 0x02); write_mem(next++, 0x40); // STA $4002 (DDRB)
  write_mem(next++, 0x8D); write_mem(next++, 0x03); write_mem(next++, 0x40); // STA $4003 (DDRA)

  for (int i = 0; i < 3; i++) {
    write_mem(next++, 0xA9); write_mem(next++, 0x30);
    write_mem(next++, 0x8D); write_mem(next++, 0x00); write_mem(next++, 0x40);
    write_mem(next++, 0xA9); write_mem(next++, 0x04);
    write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
    write_mem(next++, 0xA9); write_mem(next++, 0x00);
    write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
  }

  write_mem(next++, 0xA9); write_mem(next++, 0x38);
  write_mem(next++, 0x8D); write_mem(next++, 0x00); write_mem(next++, 0x40);
  write_mem(next++, 0xA9); write_mem(next++, 0x04);
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
  write_mem(next++, 0xA9); write_mem(next++, 0x00);
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);

  write_mem(next++, 0xA9); write_mem(next++, 0x0C);
  write_mem(next++, 0x8D); write_mem(next++, 0x00); write_mem(next++, 0x40);
  write_mem(next++, 0xA9); write_mem(next++, 0x04);
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
  write_mem(next++, 0xA9); write_mem(next++, 0x00);
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);

  write_mem(next++, 0xA9); write_mem(next++, 0x01);
  write_mem(next++, 0x8D); write_mem(next++, 0x00); write_mem(next++, 0x40);
  write_mem(next++, 0xA9); write_mem(next++, 0x04);
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
  write_mem(next++, 0xA9); write_mem(next++, 0x00);
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);

  write_mem(next++, 0xA9); write_mem(next++, 0x06);
  write_mem(next++, 0x8D); write_mem(next++, 0x00); write_mem(next++, 0x40);
  write_mem(next++, 0xA9); write_mem(next++, 0x04);
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
  write_mem(next++, 0xA9); write_mem(next++, 0x00);
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);

  // Skriv "Steg 9 SRAM" på LCD
  const char* msg1 = "Steg 9 SRAM";
  for (int i = 0; msg1[i]; i++) {
    write_mem(next++, 0xA9); write_mem(next++, msg1[i]);
    write_mem(next++, 0x8D); write_mem(next++, 0x00); write_mem(next++, 0x40);
    write_mem(next++, 0xA9); write_mem(next++, 0x05);
    write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
    write_mem(next++, 0xA9); write_mem(next++, 0x01);
    write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
  }

  // Cursor rad 2
  write_mem(next++, 0xA9); write_mem(next++, 0xC0);
  write_mem(next++, 0x8D); write_mem(next++, 0x00); write_mem(next++, 0x40);
  write_mem(next++, 0xA9); write_mem(next++, 0x04);
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
  write_mem(next++, 0xA9); write_mem(next++, 0x00);
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);

  const char* msg2 = "62256 SRAM OK!";
  for (int i = 0; msg2[i]; i++) {
    write_mem(next++, 0xA9); write_mem(next++, msg2[i]);
    write_mem(next++, 0x8D); write_mem(next++, 0x00); write_mem(next++, 0x40);
    write_mem(next++, 0xA9); write_mem(next++, 0x05);
    write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
    write_mem(next++, 0xA9); write_mem(next++, 0x01);
    write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
  }

  // JMP $8000
  write_mem(next++, 0x4C); write_mem(next++, 0x00); write_mem(next++, 0x80);

  Serial.print("Program: "); Serial.print(next - 0x8000); Serial.println(" bytes");

  // Reset-sekvens
  for (int i = 0; i < 5; i++) pulse();
  digitalWrite(RESB, HIGH);
}

void loop() {
  if (!digitalRead(BTN_CLK)) {
    delay(50); while (!digitalRead(BTN_CLK)); delay(50);
    pulse();
  }
  if (!digitalRead(BTN_INSTR)) {
    delay(50); while (!digitalRead(BTN_INSTR)); delay(50);
    do { pulse(); } while (!digitalRead(SYNC));
  }
}
