// ==============================================================
// Steg 6 — Första egna 6502-programmet (räknare)
// ==============================================================
// CPU:n kör ett riktigt maskinkodsprogram på 9 bytes:
//   LDX #$00 · INX · STX $0200 · JMP $8002
// X-registret räknar 0→255 i oändlig loop. LCD:n visar
// adress och instruktion i realtid.

#include <Arduino.h>
#include <LiquidCrystal.h>

// --- Pindefinitioner ---
#define PHI2     2
#define RESB     4
#define RWB      3
#define SYNC     13
#define BTN_CLK  11
#define BTN_INSTR 12

#define CLOCK_HZ 500                         // Klockfrekvens i Hz
#define CLOCK_DELAY (1000 / (CLOCK_HZ * 2))  // ms per klockfas

// --- LCD-konfiguration ---
LiquidCrystal lcd(5, 6, 10, 9, 8, 7);

// --- Minnesarrayer ---
uint8_t ram[1024];       // $0000–$03FF
uint8_t vectors[6];      // $FFFA–$FFFF

// --- read_mem() — slå upp minnesvärde ---
uint8_t read_mem(uint16_t addr) {
  if (addr >= 0xFFFA && addr <= 0xFFFF) return vectors[addr - 0xFFFA];
  if (addr < 1024) return ram[addr];
  return 0xEA;
}

// --- write_mem() — spara minnesvärde ---
void write_mem(uint16_t addr, uint8_t val) {
  if (addr >= 0xFFFA && addr <= 0xFFFF)
    { vectors[addr - 0xFFFA] = val; return; }
  if (addr < 1024) ram[addr] = val;
}

// --- pulse() — en klockcykel + LCD-uppdatering ---
void pulse() {
  digitalWrite(PHI2, LOW);
  delay(CLOCK_DELAY);

  uint16_t a = (PINK << 0) | (PINF << 8);
  bool rw = digitalRead(RWB);
  uint8_t data = 0;

  if (rw) { DDRA = 0xFF; PORTA = read_mem(a); data = PORTA; }
  else    { DDRA = 0x00; }

  digitalWrite(PHI2, HIGH);
  delay(CLOCK_DELAY);

  if (!rw) { data = PINA; write_mem(a, data); }

  // LCD-uppdatering
  lcd.setCursor(0, 0);
  lcd.print("A:$"); lcd.print(a, HEX);
  if (a < 0x1000) lcd.print(" ");
  if (a < 0x100)  lcd.print(" ");
  lcd.print("    ");

  lcd.setCursor(0, 1);
  lcd.print("D:$");
  if (data < 0x10) lcd.print("0");
  lcd.print(data, HEX);
  lcd.print("    ");
}

// --- setup() — ladda räknarprogrammet ---
void setup() {
  Serial.begin(115200);
  pinMode(PHI2, OUTPUT);
  pinMode(RESB, OUTPUT);
  pinMode(RWB,  INPUT);
  pinMode(SYNC, INPUT);
  pinMode(BTN_CLK,   INPUT_PULLUP);
  pinMode(BTN_INSTR, INPUT_PULLUP);
  DDRK = 0x00; DDRF = 0x00; DDRA = 0x00;

  lcd.begin(16, 2);
  lcd.print("Steg 6 Raknare");

  // --- Ladda räknarprogrammet ---
  // Reset-vektor → $8000
  write_mem(0xFFFC, 0x00);
  write_mem(0xFFFD, 0x80);

  // LDX #$00   — ladda X-registret med 0 (körs bara en gång)
  write_mem(0x8000, 0xA2);
  write_mem(0x8001, 0x00);

  // INX        — öka X med 1 (loop-start, 1 byte)
  write_mem(0x8002, 0xE8);

  // STX $0200  — spara X till minnesadress $0200 (3 bytes)
  write_mem(0x8003, 0x8E);
  write_mem(0x8004, 0x00);  // Låg byte av $0200
  write_mem(0x8005, 0x02);  // Hög byte av $0200

  // JMP $8002  — hoppa tillbaka till INX (3 bytes)
  write_mem(0x8006, 0x4C);
  write_mem(0x8007, 0x02);  // Låg byte av $8002
  write_mem(0x8008, 0x80);  // Hög byte av $8002

  Serial.println("Steg 6 — Räknarprogram");
  Serial.println("Program: LDX #$00 · INX · STX $0200 · JMP $8002");

  // Reset-sekvens
  digitalWrite(RESB, LOW);
  for (int i = 0; i < 5; i++) pulse();
  digitalWrite(RESB, HIGH);
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
}
