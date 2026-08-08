// ==============================================================
// Steg 5 — LCD-display (parallell 4-bit till Arduino)
// ==============================================================
// LCD 16×2 kopplas direkt till Arduino. Adress och data visas
// i realtid — ingen datorskärm behövs för att se vad CPU:n gör.
// Datapinnarna är i omvänd ordning (D10→DB4, D9→DB5...).

#include <Arduino.h>
#include <LiquidCrystal.h>

// --- Pindefinitioner ---
#define PHI2     2    // Arduino D2 → CPU pin 37
#define RESB     4    // Arduino D4 → CPU pin 40
#define RWB      3    // Arduino D3 → CPU pin 34
#define SYNC     13   // Arduino D13 → CPU pin 7
#define BTN_CLK  11   // Knapp 1: en klockcykel (D11 → GND)
#define BTN_INSTR 12  // Knapp 2: en instruktion (D12 → GND)

#define CLOCK_HZ 500                         // Klockfrekvens i Hz
#define CLOCK_DELAY (1000 / (CLOCK_HZ * 2))  // ms per klockfas

// --- LCD-konfiguration ---
// RS=D5, E=D6, DB4=D10, DB5=D9, DB6=D8, DB7=D7
// Notera: datapinnarna är i omvänd ordning!
LiquidCrystal lcd(5, 6, 10, 9, 8, 7);

// --- Minnesarrayer ---
uint8_t ram[1024];       // $0000–$03FF
uint8_t vectors[6];      // $FFFA–$FFFF

// --- read_mem() — slå upp minnesvärde ---
uint8_t read_mem(uint16_t addr) {
  if (addr >= 0xFFFA && addr <= 0xFFFF) return vectors[addr - 0xFFFA];
  if (addr < 1024) return ram[addr];
  return 0xEA;  // Fallback: NOP
}

// --- write_mem() — spara minnesvärde ---
void write_mem(uint16_t addr, uint8_t val) {
  if (addr >= 0xFFFA && addr <= 0xFFFF)
    { vectors[addr - 0xFFFA] = val; return; }
  if (addr < 1024) ram[addr] = val;
}

// --- pulse() — en klockcykel + LCD-uppdatering ---
// Samma minnesemulering som tidigare, men nu uppdateras LCD:n
// efter varje cykel så du ser vad CPU:n gör i realtid.
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

  // --- Uppdatera LCD ---
  // Rad 0: aktuell adress (t.ex. "A:$8000")
  lcd.setCursor(0, 0);
  lcd.print("A:$"); lcd.print(a, HEX);
  if (a < 0x1000) lcd.print(" ");   // Padning för snygg visning
  if (a < 0x100)  lcd.print(" ");
  lcd.print("    ");

  // Rad 1: data på databussen (t.ex. "D:$EA")
  lcd.setCursor(0, 1);
  lcd.print("D:$");
  if (data < 0x10) lcd.print("0");  // Padning för snygg visning
  lcd.print(data, HEX);
  lcd.print("    ");
}

// --- setup() — initiera LCD, ladda program, starta CPU ---
void setup() {
  Serial.begin(115200);
  pinMode(PHI2, OUTPUT);
  pinMode(RESB, OUTPUT);
  pinMode(RWB,  INPUT);
  pinMode(SYNC, INPUT);
  pinMode(BTN_CLK,   INPUT_PULLUP);   // Intern pull-up, knapp till GND
  pinMode(BTN_INSTR, INPUT_PULLUP);
  DDRK = 0x00; DDRF = 0x00; DDRA = 0x00;

  // Initiera LCD — 16 tecken, 2 rader
  lcd.begin(16, 2);
  lcd.print("W65C02S Steg 5");

  // Ladda NOP-program (samma som steg 3–4)
  write_mem(0xFFFC, 0x00);
  write_mem(0xFFFD, 0x80);
  write_mem(0x8000, 0xEA);

  Serial.println("Steg 5 — LCD-display");

  // Reset-sekvens
  digitalWrite(RESB, LOW);
  for (int i = 0; i < 5; i++) pulse();
  digitalWrite(RESB, HIGH);
}

// --- loop() — knappstyrning, samma som steg 4 ---
// Knapp 1: en klockcykel. Knapp 2: kör till nästa instruktion.
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
