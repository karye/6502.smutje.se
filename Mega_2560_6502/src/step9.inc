// Steg 9 — Riktigt RAM (62256 32KB SRAM)
// SRAM på $0000–$3FFF (16 KB). Arduino tri-state för SRAM-adresser.
// VIA på $4000–$400F, program på $8000+ (Arduino).

#include <Arduino.h>
#include <LiquidCrystal.h>

#define PHI2     2
#define RESB     4
#define RWB      3
#define SYNC     13
#define BTN_CLK  11
#define BTN_INSTR 12

LiquidCrystal lcd(5, 6, 10, 9, 8, 7);

uint8_t ram[1024];          // Används ej längre för $0000–$03FF — SRAM hanterar detta
uint8_t program[2048];      // $8000–$87FF (6502-program)
uint8_t vectors[6];         // $FFFA–$FFFF

#define VIA_BASE     0xC000
#define SRAM_END     0x7FFF

// --- is_sram() — hanteras av fysisk 62256 ---
bool is_sram(uint16_t addr) { return addr <= SRAM_END; }

// --- is_via() — hanteras av fysisk W65C22 ---
bool is_via(uint16_t addr) { return (addr >= VIA_BASE && addr < VIA_BASE + 16); }

// --- read_mem() — Arduino svarar bara utanför SRAM/VIA ---
uint8_t read_mem(uint16_t addr) {
  if (is_sram(addr)) return 0;          // SRAM svarar
  if (is_via(addr)) return 0;           // VIA svarar
  if (addr >= 0xFFFA && addr <= 0xFFFF)
    return vectors[addr - 0xFFFA];
  if (addr >= 0x8000 && addr < 0x8000 + sizeof(program))
    return program[addr - 0x8000];
  return 0xEA;
}

void write_mem(uint16_t addr, uint8_t val) {
  if (is_sram(addr)) return;            // SRAM tar emot
  if (is_via(addr)) return;             // VIA tar emot
  if (addr >= 0xFFFA && addr <= 0xFFFF)
    { vectors[addr - 0xFFFA] = val; return; }
  if (addr >= 0x8000 && addr < 0x8000 + sizeof(program))
    { program[addr - 0x8000] = val; return; }
}

void pulse() {
  digitalWrite(PHI2, LOW);
  DDRA = 0x00;               // Tri-state direkt — SRAM/VIA tar över om de ska
  delay(10);

  uint8_t kl = PINF;
  uint8_t kh = PINK;
  uint16_t a = (kl << 0) | (kh << 8);
  bool rw = digitalRead(RWB);

  // Arduino svarar endast på adresser utanför SRAM och VIA
  if (rw && !is_sram(a) && !is_via(a)) {
    DDRA = 0xFF;
    PORTA = read_mem(a);
  }

  digitalWrite(PHI2, HIGH);
  delay(10);

  if (!rw && !is_sram(a) && !is_via(a)) write_mem(a, PINA);

  // Diagnostik
  Serial.print(rw ? "R" : "W");
  Serial.print(" $");
  Serial.print(a, HEX);
  if (is_sram(a))            Serial.print("  <- SRAM");
  else if (is_via(a))         Serial.print("  <- VIA");
  else if (a >= 0x8000 && rw) Serial.print("  <- OPCODE");
  else if (a == 0xFFFC)      Serial.print("  <- RESET-VEKTOR LAG");
  else if (a == 0xFFFD)      Serial.print("  <- RESET-VEKTOR HOG");
  Serial.println();

  // LCD-uppdatering
  lcd.setCursor(0, 0);
  lcd.print("A:$"); lcd.print(a, HEX);
  if (a < 0x1000) lcd.print(" ");
  if (a < 0x100)  lcd.print(" ");
  lcd.print("    ");
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

  lcd.begin(16, 2);
  lcd.print("Steg 9 SRAM 62256");

  // Ladda 6502-program (samma som steg 8 — från program.h)
  // För steg 9 laddar vi programmet på $8000 via Arduino
  write_mem(0xFFFC, 0x00);
  write_mem(0xFFFD, 0x80);

  // Räknarprogram som använder SRAM på $0200
  write_mem(0x8000, 0xA2);  // LDX #$00
  write_mem(0x8001, 0x00);
  write_mem(0x8002, 0xE8);  // INX
  write_mem(0x8003, 0x8E);  // STX $0200  ← skriver till SRAM!
  write_mem(0x8004, 0x00);
  write_mem(0x8005, 0x02);
  write_mem(0x8006, 0x4C);  // JMP $8002
  write_mem(0x8007, 0x02);
  write_mem(0x8008, 0x80);

  Serial.println("Steg 9 — 62256 SRAM på $0000–$3FFF");
  Serial.println("SRAM hanterar zero page, stack och data");
  Serial.println("Arduino = program ($8000+) + vektorer");
  Serial.println("VIA = I/O ($4000–$400F)");

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
