#include <Arduino.h>
#include <LiquidCrystal.h>

// -----------------------------------------------------------
// Steg 4 — Knappstyrning med LCD
// -----------------------------------------------------------

#define PHI2     2   // CPU pin 37
#define RESB     4   // CPU pin 40
#define RWB      3   // CPU pin 34
#define SYNC     13  // CPU pin 7 → Arduino D13
#define BTN_CLK  11  // Knapp: en klockcykel per tryck
#define BTN_INSTR 12 // Knapp: kör till nästa instruktion

uint8_t ram[1024];     // $0000–$03FF (stack + data + program)
uint8_t vectors[6];     // $FFFA–$FFFF

uint8_t read_mem(uint16_t addr) {
  if (addr >= 0xFFFA && addr <= 0xFFFF)
    return vectors[addr - 0xFFFA];
  if (addr < 1024) return ram[addr];
  return 0xEA;
}

void write_mem(uint16_t addr, uint8_t val) {
  if (addr >= 0xFFFA && addr <= 0xFFFF) {
    vectors[addr - 0xFFFA] = val; return;
  }
  if (addr < 1024) ram[addr] = val;
}

// LCD — orörd från steg 3
//        RS  E   D4  D5  D6  D7
LiquidCrystal lcd(5, 6, 10, 9, 8, 7);

void pulse();  // Forward

void setup() {
  // --- LCD ---
  lcd.begin(16, 2);
  lcd.print("W65C02S Steg 4");
  lcd.setCursor(0, 1);
  lcd.print("Tryck BTN1/2");

  // --- Knappar och SYNC ---
  pinMode(SYNC, INPUT);
  pinMode(BTN_CLK,   INPUT_PULLUP);
  pinMode(BTN_INSTR, INPUT_PULLUP);

  // --- Håll CPU i reset från start ---
  pinMode(RESB, OUTPUT); digitalWrite(RESB, LOW);
  pinMode(PHI2, OUTPUT); digitalWrite(PHI2, LOW);
  Serial.begin(115200);
  pinMode(RWB, INPUT);

  DDRK = 0x00;  // A8–A15
  DDRF = 0x00;  // A0–A7
  DDRA = 0x00;

  // --- Reset-vektor → $0200 ---
  write_mem(0xFFFC, 0x00);
  write_mem(0xFFFD, 0x02);

  // Ladda program: räknare som ökar A i loop
  //  $0200: A9 00     LDA #$00   (A = 0)
  //  $0202: 1A        INA        (A = A+1)
  //  $0203: 8D FF 03  STA $03FF  (spara A)
  //  $0206: 4C 02 02  JMP $0202  (loop till INA)
  write_mem(0x0200, 0xA9); write_mem(0x0201, 0x00);
  write_mem(0x0202, 0x1A);
  write_mem(0x0203, 0x8D); write_mem(0x0204, 0xFF); write_mem(0x0205, 0x03);
  write_mem(0x0206, 0x4C); write_mem(0x0207, 0x02); write_mem(0x0208, 0x02);

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

  if (rw) { DDRA = 0xFF; PORTA = read_mem(a); }
  else    { DDRA = 0x00; }

  digitalWrite(PHI2, HIGH);
  delay(25);

  if (!rw) write_mem(a, PINA);

  Serial.print(rw ? "R" : "W");
  Serial.print(" $");
  Serial.println(a, HEX);
}

void loop() {
  // Läs aktuellt A-register (CPU:n sparar till $03FF)
  static uint8_t lastA = 0xFF;
  uint8_t aVal = ram[0x03FF];
  if (aVal != lastA) {
    lastA = aVal;
    lcd.setCursor(0, 0);
    lcd.print("A = $");
    if (aVal < 0x10) lcd.print("0");
    lcd.print(aVal, HEX);
    lcd.print("        ");
    lcd.setCursor(0, 1);
    lcd.print("Tryck BTN1/2");
  }

  // --- Knapp 1: En klockcykel ---
  if (!digitalRead(BTN_CLK)) {
    Serial.println("--- BTN1 (D11) nedtryckt: en klockcykel ---");
    delay(50);
    while (!digitalRead(BTN_CLK));
    delay(50);
    Serial.println("--- BTN1 (D11) släppt ---");
    pulse();
  }

  // --- Knapp 2: Kör till nästa instruktion ---
  if (!digitalRead(BTN_INSTR)) {
    Serial.println("--- BTN2 (D12) nedtryckt: kör till nästa instruktion ---");
    delay(50);
    while (!digitalRead(BTN_INSTR));
    delay(50);
    Serial.println("--- BTN2 (D12) släppt ---");
    do {
      pulse();
    } while (!digitalRead(SYNC));
    Serial.println("--- SYNC hög → instruktion klar ---");
  }
}