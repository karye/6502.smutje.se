#include <Arduino.h>

// -----------------------------------------------------------
// Steg 7 — VIA + LCD (LCD styrs av 6502 via W65C22 VIA)
// -----------------------------------------------------------

#define PHI2     2   // CPU pin 37
#define RESB     4   // CPU pin 40
#define RWB      3   // CPU pin 34
#define SYNC     13  // CPU pin 7 → Arduino D13
#define BTN_CLK  11  // Knapp: en klockcykel per tryck
#define BTN_INSTR 12 // Knapp: kör till nästa instruktion

uint8_t ram[1024];     // $0000–$03FF (stack + data + program)
uint8_t vectors[6];     // $FFFA–$FFFF

// VIA-adresser — VIA:n är en fysisk krets, Arduinon ska INTE svara här
#define VIA_BASE 0xC000

bool is_via(uint16_t addr) {
  return (addr >= VIA_BASE && addr < VIA_BASE + 16);
}

uint8_t read_mem(uint16_t addr) {
  if (is_via(addr)) return 0;  // VIA hanterar detta själv
  if (addr >= 0xFFFA && addr <= 0xFFFF)
    return vectors[addr - 0xFFFA];
  if (addr < 1024) return ram[addr];
  return 0xEA;
}

void write_mem(uint16_t addr, uint8_t val) {
  if (is_via(addr)) return;    // VIA hanterar detta själv
  if (addr >= 0xFFFA && addr <= 0xFFFF) {
    vectors[addr - 0xFFFA] = val; return;
  }
  if (addr < 1024) ram[addr] = val;
}

void pulse();  // Forward

void setup() {
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

  // --- Reset-vektor → $8000 ---
  write_mem(0xFFFC, 0x00);
  write_mem(0xFFFD, 0x80);

  // LCD-init via VIA (6502-program på $8000):
  //  $8000: A9 FF     LDA #$FF
  //  $8002: 8D 02 C0  STA $C002   (DDRB = alla utgångar)
  //  $8005: 8D 03 C0  STA $C003   (DDRA = alla utgångar)
  //  $8008: A9 38     LDA #$38    (Function set: 8-bit, 2 rader)
  //  $800A: 8D 00 C0  STA $C000   (PORTB = $38)
  //  $800D: A9 04     LDA #$04    (E=1)
  //  $800F: 8D 01 C0  STA $C001   (PORTA)
  //  $8012: A9 00     LDA #$00    (E=0)
  //  $8014: 8D 01 C0  STA $C001
  //  $8017: A9 0C     LDA #$0C    (Display ON)
  //  $8019: 8D 00 C0  STA $C000
  //  $801C: A9 04     LDA #$04
  //  $801E: 8D 01 C0  STA $C001
  //  $8021: A9 00     LDA #$00
  //  $8023: 8D 01 C0  STA $C001
  //  $8026: 4C 26 80  JMP $8026   (klart — loopa)
  write_mem(0x8000, 0xA9); write_mem(0x8001, 0xFF);
  write_mem(0x8002, 0x8D); write_mem(0x8003, 0x02); write_mem(0x8004, 0xC0);
  write_mem(0x8005, 0x8D); write_mem(0x8006, 0x03); write_mem(0x8007, 0xC0);
  write_mem(0x8008, 0xA9); write_mem(0x8009, 0x38);
  write_mem(0x800A, 0x8D); write_mem(0x800B, 0x00); write_mem(0x800C, 0xC0);
  write_mem(0x800D, 0xA9); write_mem(0x800E, 0x04);
  write_mem(0x800F, 0x8D); write_mem(0x8010, 0x01); write_mem(0x8011, 0xC0);
  write_mem(0x8012, 0xA9); write_mem(0x8013, 0x00);
  write_mem(0x8014, 0x8D); write_mem(0x8015, 0x01); write_mem(0x8016, 0xC0);
  write_mem(0x8017, 0xA9); write_mem(0x8018, 0x0C);
  write_mem(0x8019, 0x8D); write_mem(0x801A, 0x00); write_mem(0x801B, 0xC0);
  write_mem(0x801C, 0xA9); write_mem(0x801D, 0x04);
  write_mem(0x801E, 0x8D); write_mem(0x801F, 0x01); write_mem(0x8020, 0xC0);
  write_mem(0x8021, 0xA9); write_mem(0x8022, 0x00);
  write_mem(0x8023, 0x8D); write_mem(0x8024, 0x01); write_mem(0x8025, 0xC0);
  write_mem(0x8026, 0x4C); write_mem(0x8027, 0x26); write_mem(0x8028, 0x80);

  // --- Reset-sekvens ---
  for (int i = 0; i < 5; i++) pulse();
  digitalWrite(RESB, HIGH);
  for (int i = 0; i < 40; i++) pulse();  // Fler cykler för LCD-init
}

void pulse() {
  digitalWrite(PHI2, LOW);
  delay(25);

  uint16_t a = (PINF << 0) | (PINK << 8);
  bool rw = digitalRead(RWB);

  if (rw) {
    if (is_via(a)) {
      DDRA = 0x00;  // VIA driver databussen — Arduino håller sig undan
    } else {
      DDRA = 0xFF;
      PORTA = read_mem(a);
    }
  } else {
    DDRA = 0x00;    // CPU driver databussen vid skrivning
  }

  digitalWrite(PHI2, HIGH);
  delay(25);

  if (!rw && !is_via(a)) write_mem(a, PINA);

  Serial.print(rw ? "R" : "W");
  Serial.print(" $");
  Serial.println(a, HEX);
}

void loop() {
  // --- Knapp 1: En klockcykel ---
  if (!digitalRead(BTN_CLK)) {
    delay(50);
    while (!digitalRead(BTN_CLK));
    delay(50);
    pulse();
  }

  // --- Knapp 2: Kör till nästa instruktion ---
  if (!digitalRead(BTN_INSTR)) {
    delay(50);
    while (!digitalRead(BTN_INSTR));
    delay(50);
    do {
      pulse();
    } while (!digitalRead(SYNC));
  }
}
