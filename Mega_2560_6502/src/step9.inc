// Steg 9 — Riktigt RAM (62256 32KB SRAM)
// Baserat på steg 7: först visa 4-raders text via VIA/LCD,
// sedan vänta på knapptryck, kör RAM-test.
// SRAM på $0000–$3FFF, VIA på $4000, program på $8000+ (Arduino).

#include <Arduino.h>
#include <LiquidCrystal.h>
#include "program_fib.h"   // Fibonacci-program, byggt med ca65

#define PHI2     2
#define RESB     4
#define RWB      3
#define SYNC     13
#define BTN_CLK  11
#define BTN_INSTR 12

LiquidCrystal lcd(5, 6, 10, 9, 8, 7);

uint8_t program[2048];      // $8000–$87FF (6502-program)
uint8_t vectors[6];         // $FFFA–$FFFF

bool debug_pulse = false;   // Styr loggutskrift i pulse()

#define VIA_BASE     0x4000
#define SRAM_END     0x3FFF

// --- is_sram() — hanteras av fysisk 62256 ---
bool is_sram(uint16_t addr) { return addr <= SRAM_END; }

// --- is_via() — hanteras av fysisk W65C22 ---
bool is_via(uint16_t addr) {
  return (addr >= VIA_BASE && addr < VIA_BASE + 16);
}

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

// --- write_mem() — spara, men bara utanför SRAM/VIA ---
void write_mem(uint16_t addr, uint8_t val) {
  if (is_sram(addr)) return;            // SRAM tar emot
  if (is_via(addr)) return;             // VIA tar emot
  if (addr >= 0xFFFA && addr <= 0xFFFF)
    { vectors[addr - 0xFFFA] = val; return; }
  if (addr >= 0x8000 && addr < 0x8000 + sizeof(program))
    { program[addr - 0x8000] = val; return; }
}

// --- pulse() — en klockcykel, Arduino tri-state för SRAM och VIA ---
void pulse() {
  digitalWrite(PHI2, LOW);
  DDRA = 0x00;               // Tri-state direkt
  delay(10);

  uint8_t kl = PINF;
  uint8_t kh = PINK;
  uint16_t a = (kl << 0) | (kh << 8);
  bool rw = digitalRead(RWB);

  // Arduino svarar ENDAST om adressen inte är SRAM eller VIA
  if (rw && !is_sram(a) && !is_via(a)) {
    DDRA = 0xFF;
    PORTA = read_mem(a);
  }

  digitalWrite(PHI2, HIGH);
  delay(10);

  if (!rw && !is_sram(a) && !is_via(a)) write_mem(a, PINA);

  // Diagnostik (tyst under bulk-körning)
  static int count = 0;
  if (debug_pulse) {
    count++;
    if (count % 20 == 0 || is_via(a) || is_sram(a) || a == 0xFFFC || a == 0xFFFD) {
      Serial.print(rw ? "R" : "W");
      Serial.print(" $");
      Serial.print(a, HEX);
      if (is_sram(a))            Serial.print("  <- SRAM");
      else if (is_via(a))         Serial.print("  <- VIA");
      else if (a >= 0x8000 && a < 0xFFFA) {}
      else if (a == 0xFFFC)      Serial.print("  <- RESET-VEKTOR LAG");
      else if (a == 0xFFFD)      Serial.print("  <- RESET-VEKTOR HOG");
      Serial.println();
    }
  }
}

// ================================================================
// Ladda 6502-program (samma 4-raders LCD-program som steg 7)
// ================================================================
void load_hello_program() {
  uint16_t next = 0x8000;

  // Reset-vektor → $8000
  write_mem(0xFFFC, 0x00);
  write_mem(0xFFFD, 0x80);

  // --- Sätt VIA-portar som utgångar ---
  write_mem(next++, 0xA9); write_mem(next++, 0xFF);  // LDA #$FF
  write_mem(next++, 0x8D); write_mem(next++, 0x02); write_mem(next++, 0x40); // STA $4002 (DDRB)
  write_mem(next++, 0x8D); write_mem(next++, 0x03); write_mem(next++, 0x40); // STA $4003 (DDRA)

  // --- Skicka $30 tre gånger (tvinga 8-bit mode) ---
  for (int i = 0; i < 3; i++) {
    write_mem(next++, 0xA9); write_mem(next++, 0x30);
    write_mem(next++, 0x8D); write_mem(next++, 0x00); write_mem(next++, 0x40);
    write_mem(next++, 0xA9); write_mem(next++, 0x04);
    write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
    write_mem(next++, 0xA9); write_mem(next++, 0x00);
    write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
  }

  // --- Function Set: 8-bit, 2 rader, 5x8 ---
  write_mem(next++, 0xA9); write_mem(next++, 0x38);
  write_mem(next++, 0x8D); write_mem(next++, 0x00); write_mem(next++, 0x40);
  write_mem(next++, 0xA9); write_mem(next++, 0x04);
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
  write_mem(next++, 0xA9); write_mem(next++, 0x00);
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);

  // --- Display ON, cursor OFF ---
  write_mem(next++, 0xA9); write_mem(next++, 0x0C);
  write_mem(next++, 0x8D); write_mem(next++, 0x00); write_mem(next++, 0x40);
  write_mem(next++, 0xA9); write_mem(next++, 0x04);
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
  write_mem(next++, 0xA9); write_mem(next++, 0x00);
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);

  // --- Clear Display ---
  write_mem(next++, 0xA9); write_mem(next++, 0x01);
  write_mem(next++, 0x8D); write_mem(next++, 0x00); write_mem(next++, 0x40);
  write_mem(next++, 0xA9); write_mem(next++, 0x04);
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
  write_mem(next++, 0xA9); write_mem(next++, 0x00);
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);

  // --- Entry Mode: increment, no shift ---
  write_mem(next++, 0xA9); write_mem(next++, 0x06);
  write_mem(next++, 0x8D); write_mem(next++, 0x00); write_mem(next++, 0x40);
  write_mem(next++, 0xA9); write_mem(next++, 0x04);
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
  write_mem(next++, 0xA9); write_mem(next++, 0x00);
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);

  // --- Rad 1: "=== 6502 VIA LCD ===" ---
  write_mem(next++, 0xA9); write_mem(next++, 0x80);
  write_mem(next++, 0x8D); write_mem(next++, 0x00); write_mem(next++, 0x40);
  write_mem(next++, 0xA9); write_mem(next++, 0x04);
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
  write_mem(next++, 0xA9); write_mem(next++, 0x00);
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
  write_mem(next++, 0xA9); write_mem(next++, 0x01);
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);

  const char* line1 = "=== Steg 9 SRAM ===";
  for (const char* p = line1; *p; p++) {
    write_mem(next++, 0xA9); write_mem(next++, *p);
    write_mem(next++, 0x8D); write_mem(next++, 0x00); write_mem(next++, 0x40);
    write_mem(next++, 0xA9); write_mem(next++, 0x05);
    write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
    write_mem(next++, 0xA9); write_mem(next++, 0x01);
    write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
  }

  // --- Rad 2: "Tryck pa knapp..." ---
  write_mem(next++, 0xA9); write_mem(next++, 0xC0);
  write_mem(next++, 0x8D); write_mem(next++, 0x00); write_mem(next++, 0x40);
  write_mem(next++, 0xA9); write_mem(next++, 0x04);
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
  write_mem(next++, 0xA9); write_mem(next++, 0x00);
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
  write_mem(next++, 0xA9); write_mem(next++, 0x01);
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);

  const char* line2 = "Tryck pa knapp...";
  for (const char* p = line2; *p; p++) {
    write_mem(next++, 0xA9); write_mem(next++, *p);
    write_mem(next++, 0x8D); write_mem(next++, 0x00); write_mem(next++, 0x40);
    write_mem(next++, 0xA9); write_mem(next++, 0x05);
    write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
    write_mem(next++, 0xA9); write_mem(next++, 0x01);
    write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
  }

  // --- Halt-loop (vänta på omstart efter knapptryck) ---
  uint16_t halt1 = next;
  write_mem(next++, 0x4C);                           // JMP
  write_mem(next++, halt1 & 0xFF);                   // låg byte
  write_mem(next++, (halt1 >> 8) & 0xFF);            // hög byte
}

// ================================================================
// Ladda Fibonacci-program från assembler-bygge (program_fib.h)
// ================================================================
void load_fibonacci_program() {
  memset(program, 0, sizeof(program));

  // Ladda program ($8000–$87FF) — 2048 bytes
  for (unsigned int i = 0; i < 2048; i++) {
    write_mem(0x8000 + i, pgm_read_byte(&PROGRAM_FIB[i]));
  }

  // Ladda vektorer ($FFFA–$FFFF) — sista 6 byten
  for (unsigned int i = 0; i < 6; i++) {
    write_mem(0xFFFA + i, pgm_read_byte(&PROGRAM_FIB[2048 + i]));
  }
}

// ================================================================
// setup()
// ================================================================
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
  lcd.print("Steg 9 SRAM...");

  Serial.println("Steg 9 — 62256 SRAM");
  Serial.println("Fas 1: Laddar 4-raders VIA/LCD-program...");

  // --- Fas 1: Visa 4-raders text (samma som steg 7) ---
  load_hello_program();

  // Reset och kör
  digitalWrite(RESB, LOW);
  for (int i = 0; i < 5; i++) pulse();
  digitalWrite(RESB, HIGH);

  // Kör ~2500 cykler så LCD-init + text hinner visas
  Serial.println("Kor 1000 cykler (tyst)...");
  for (int i = 0; i < 1000; i++) pulse();
  debug_pulse = true;

  Serial.println("Fas 1 klar — vantar pa knapptryck...");

  // --- Vänta på knapptryck ---
  bool knapp_tryckt = false;
  unsigned long last_msg = 0;
  while (!knapp_tryckt) {
    bool b1 = !digitalRead(BTN_CLK);
    bool b2 = !digitalRead(BTN_INSTR);
    if (b1 || b2) {
      delay(50);
      b1 = !digitalRead(BTN_CLK);
      b2 = !digitalRead(BTN_INSTR);
      if (b1 || b2) {
        knapp_tryckt = true;
        Serial.print(">>> Knapp ");
        Serial.print(b1 ? "1" : "2");
        Serial.println(" tryckt! <<<");
      }
    }
    if (millis() - last_msg > 2000) {
      Serial.println("Vantar pa knapp...");
      last_msg = millis();
    }
  }

  Serial.println("Laddar Fibonacci-program...");

  // --- Fas 2: Kör Fibonacci-program ---
  digitalWrite(RESB, LOW);
  for (int i = 0; i < 10; i++) pulse();

  load_fibonacci_program();

  digitalWrite(RESB, HIGH);

  debug_pulse = false;
  for (int i = 0; i < 8000; i++) pulse();
  debug_pulse = true;

  Serial.println("Fibonacci-program kört.");
}

// ================================================================
// loop()
// ================================================================
void loop() {
  // Efter setup() kan användaren trycka på knapp igen
  // för att stega manuellt
  if (!digitalRead(BTN_CLK)) {
    delay(50); while (!digitalRead(BTN_CLK)); delay(50);
    pulse();
  }
  if (!digitalRead(BTN_INSTR)) {
    delay(50); while (!digitalRead(BTN_INSTR)); delay(50);
    do { pulse(); } while (!digitalRead(SYNC));
  }
}
