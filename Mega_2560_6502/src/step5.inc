// Steg 5 — LCD-display (parallell 4-bit till Arduino)
// LCD:n visar aktuell adress och data i realtid.

#include <Arduino.h>
#include <LiquidCrystal.h>

#define PHI2     2
#define RESB     4
#define CLOCK_DELAY 1   // ms per klockfas (1 = ~500 Hz, 10 = ~50 Hz)
#define RWB      3
#define CLOCK_DELAY 1   // ms per klockfas (1 = ~500 Hz, 10 = ~50 Hz)
#define SYNC     13
#define CLOCK_DELAY 1   // ms per klockfas (1 = ~500 Hz, 10 = ~50 Hz)
#define BTN_CLK  11
#define BTN_INSTR 12
#define CLOCK_DELAY 1   // ms per klockfas (1 = ~500 Hz, 10 = ~50 Hz)

// LCD: RS=D5, E=D6, D4=D10, D5=D9, D6=D8, D7=D7 (omvänd ordning!)
LiquidCrystal lcd(5, 6, 10, 9, 8, 7);

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
  lcd.print("W65C02S Steg 5");

  write_mem(0xFFFC, 0x00);
  write_mem(0xFFFD, 0x80);
  write_mem(0x8000, 0xEA);

  Serial.println("Steg 5 — LCD-display");

  digitalWrite(RESB, LOW);
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
