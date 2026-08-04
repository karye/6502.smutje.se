#include <Arduino.h>

// -----------------------------------------------------------
// Steg 7 — VIA + LCD (LCD styrs av 6502 via W65C22 VIA)
// -----------------------------------------------------------

#define PHI2     2   // CPU pin 37
#define RESB     4   // CPU pin 40
#define RWB      3   // CPU pin 34
#define SYNC     13  // CPU pin 7
#define BTN_CLK  11  // Knapp: en klockcykel per tryck
#define BTN_INSTR 12 // Knapp: kör till nästa instruktion

uint8_t ram[1024];       // $0000–$03FF (stack + zero page)
uint8_t program[2048];   // $8000–$87FF (6502-program, Arduino svarar)
uint8_t vectors[6];      // $FFFA–$FFFF

#define VIA_BASE 0x4000

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

void pulse();  // Forward declaration

void pulse() {
  DDRA = 0x00;  // Tri-state

  // -- Fas 1: PHI2 låg --
  digitalWrite(PHI2, LOW);
  delay(10);

  uint8_t kl = PINK;   // CPU A0–A7 (via Arduino A8–A15)
  uint8_t kh = PINF;   // CPU A8–A15 (via Arduino A0–A7)
  uint16_t a = (kl << 0) | (kh << 8);
  bool rw = digitalRead(RWB);

  if (rw && !is_via(a)) {
    DDRA = 0xFF;
    PORTA = read_mem(a);
  }

  // -- Fas 2: PHI2 hög --
  digitalWrite(PHI2, HIGH);
  delay(10);

  if (!rw && !is_via(a)) write_mem(a, PINA);

  // Debug: visa allt
  Serial.print(rw ? "R" : "W");
  Serial.print(" $");
  Serial.print(a, HEX);
  Serial.print(" (K=");
  Serial.print(kl, HEX);
  Serial.print(" F=");
  Serial.print(kh, HEX);
  Serial.print(" rw=");
  Serial.print(rw);
  Serial.println(")");
}

void setup() {
  // CPU i reset från start — avgörande!
  pinMode(RESB, OUTPUT); digitalWrite(RESB, LOW);
  pinMode(PHI2, OUTPUT); digitalWrite(PHI2, LOW);

  pinMode(SYNC, INPUT);
  pinMode(BTN_CLK,   INPUT_PULLUP);
  pinMode(BTN_INSTR, INPUT_PULLUP);
  pinMode(RWB, INPUT);
  Serial.begin(115200);

  DDRK = 0x00;  // A8–A15 in
  DDRF = 0x00;  // A0–A7  in
  DDRA = 0x00;

  // Reset-vektor → $8000
  write_mem(0xFFFC, 0x00);
  write_mem(0xFFFD, 0x80);

  // Fullständig LCD-init via VIA (6502-program på $8000):
  // Steg 1: Sätt VIA-portar som utgångar
  //  $8000: A9 FF     LDA #$FF
  //  $8002: 8D 02 C0  STA $C002   (DDRB)   PORTB = output
  //  $8005: 8D 03 C0  STA $C003   (DDRA)   PORTA = output
  write_mem(0x8000, 0xA9); write_mem(0x8001, 0xFF);
  write_mem(0x8002, 0x8D); write_mem(0x8003, 0x02); write_mem(0x8004, 0x40);
  write_mem(0x8005, 0x8D); write_mem(0x8006, 0x03); write_mem(0x8007, 0x40);

  // Steg 2: Vänta >15ms efter power-up (CPU:n har redan väntat tillräckligt)

  // Steg 3: Skicka $30 tre gånger (tvinga 8-bit mode)
  //   $8008: A9 30     LDA #$30
  write_mem(0x8008, 0xA9); write_mem(0x8009, 0x30);
  //   Send 3 times with E-pulse:
  for (int i = 0; i < 3; i++) {
    uint16_t base = 0x800A + i * 9;
    write_mem(base,     0x8D); write_mem(base+1, 0x00); write_mem(base+2, 0x40); // STA $C000
    write_mem(base+3,   0xA9); write_mem(base+4, 0x04);                         // LDA #$04 (E=1)
    write_mem(base+5,   0x8D); write_mem(base+6, 0x01); write_mem(base+7, 0x40); // STA $C001
    write_mem(base+8,   0xA9); write_mem(base+9, 0x00);                         // LDA #$00 (E=0)
  }
  // Fortsättning efter tredje sändningen:
  uint16_t next = 0x800A + 3 * 9;
  write_mem(next,     0x8D); write_mem(next+1, 0x01); write_mem(next+2, 0x40); // STA $C001
  next += 3;

  // Steg 4: Function Set: 8-bit, 2 rader, 5x8 font
  write_mem(next, 0xA9); write_mem(next+1, 0x38); next += 2;                    // LDA #$38
  write_mem(next, 0x8D); write_mem(next+1, 0x00); write_mem(next+2, 0x40); next += 3; // STA $C000
  write_mem(next, 0xA9); write_mem(next+1, 0x04); next += 2;                    // LDA #$04 (E=1)
  write_mem(next, 0x8D); write_mem(next+1, 0x01); write_mem(next+2, 0x40); next += 3; // STA $C001
  write_mem(next, 0xA9); write_mem(next+1, 0x00); next += 2;                    // LDA #$00 (E=0)
  write_mem(next, 0x8D); write_mem(next+1, 0x01); write_mem(next+2, 0x40); next += 3; // STA $C001

  // Steg 5: Display ON, cursor OFF, blink OFF
  write_mem(next, 0xA9); write_mem(next+1, 0x0C); next += 2;                    // LDA #$0C
  write_mem(next, 0x8D); write_mem(next+1, 0x00); write_mem(next+2, 0x40); next += 3; // STA $C000
  write_mem(next, 0xA9); write_mem(next+1, 0x04); next += 2;                    // LDA #$04
  write_mem(next, 0x8D); write_mem(next+1, 0x01); write_mem(next+2, 0x40); next += 3; // STA $C001
  write_mem(next, 0xA9); write_mem(next+1, 0x00); next += 2;                    // LDA #$00
  write_mem(next, 0x8D); write_mem(next+1, 0x01); write_mem(next+2, 0x40); next += 3; // STA $C001

  // Steg 6: Clear Display
  write_mem(next, 0xA9); write_mem(next+1, 0x01); next += 2;                    // LDA #$01
  write_mem(next, 0x8D); write_mem(next+1, 0x00); write_mem(next+2, 0x40); next += 3; // STA $C000
  write_mem(next, 0xA9); write_mem(next+1, 0x04); next += 2;                    // LDA #$04
  write_mem(next, 0x8D); write_mem(next+1, 0x01); write_mem(next+2, 0x40); next += 3; // STA $C001
  write_mem(next, 0xA9); write_mem(next+1, 0x00); next += 2;                    // LDA #$00
  write_mem(next, 0x8D); write_mem(next+1, 0x01); write_mem(next+2, 0x40); next += 3; // STA $C001

  // Steg 7: Entry Mode: increment, no shift
  write_mem(next, 0xA9); write_mem(next+1, 0x06); next += 2;                    // LDA #$06
  write_mem(next, 0x8D); write_mem(next+1, 0x00); write_mem(next+2, 0x40); next += 3; // STA $C000
  write_mem(next, 0xA9); write_mem(next+1, 0x04); next += 2;                    // LDA #$04
  write_mem(next, 0x8D); write_mem(next+1, 0x01); write_mem(next+2, 0x40); next += 3; // STA $C001
  write_mem(next, 0xA9); write_mem(next+1, 0x00); next += 2;                    // LDA #$00
  write_mem(next, 0x8D); write_mem(next+1, 0x01); write_mem(next+2, 0x40); next += 3; // STA $C001

  // Hoppa till Hello-rutinen
  write_mem(next, 0x4C); write_mem(next+1, 0x29); write_mem(next+2, 0x80);     // JMP $8029

  // Hello-rutin — skriv "Hello 6502!" via VIA:
  //  $8029: A9 01     LDA #$01    (RS=1, R/W=0, E=0)
  //  $802B: 8D 01 C0  STA $C001
  //  -- Skriv varje tecken: LDA #char, STA $C000, pulsa E --
  //  'H' = $48
  write_mem(0x8029, 0xA9); write_mem(0x802A, 0x01);
  write_mem(0x802B, 0x8D); write_mem(0x802C, 0x01); write_mem(0x802D, 0x40);
  write_mem(0x802E, 0xA9); write_mem(0x802F, 0x48);  // 'H'
  write_mem(0x8030, 0x8D); write_mem(0x8031, 0x00); write_mem(0x8032, 0x40);
  write_mem(0x8033, 0xA9); write_mem(0x8034, 0x05); write_mem(0x8035, 0x8D); write_mem(0x8036, 0x01); write_mem(0x8037, 0x40);
  write_mem(0x8038, 0xA9); write_mem(0x8039, 0x01); write_mem(0x803A, 0x8D); write_mem(0x803B, 0x01); write_mem(0x803C, 0x40);
  //  'e' = $65
  write_mem(0x803D, 0xA9); write_mem(0x803E, 0x65);
  write_mem(0x803F, 0x8D); write_mem(0x8040, 0x00); write_mem(0x8041, 0x40);
  write_mem(0x8042, 0xA9); write_mem(0x8043, 0x05); write_mem(0x8044, 0x8D); write_mem(0x8045, 0x01); write_mem(0x8046, 0x40);
  write_mem(0x8047, 0xA9); write_mem(0x8048, 0x01); write_mem(0x8049, 0x8D); write_mem(0x804A, 0x01); write_mem(0x804B, 0x40);
  //  'l' = $6C
  write_mem(0x804C, 0xA9); write_mem(0x804D, 0x6C);
  write_mem(0x804E, 0x8D); write_mem(0x804F, 0x00); write_mem(0x8050, 0x40);
  write_mem(0x8051, 0xA9); write_mem(0x8052, 0x05); write_mem(0x8053, 0x8D); write_mem(0x8054, 0x01); write_mem(0x8055, 0x40);
  write_mem(0x8056, 0xA9); write_mem(0x8057, 0x01); write_mem(0x8058, 0x8D); write_mem(0x8059, 0x01); write_mem(0x805A, 0x40);
  //  'l' = $6C
  write_mem(0x805B, 0xA9); write_mem(0x805C, 0x6C);
  write_mem(0x805D, 0x8D); write_mem(0x805E, 0x00); write_mem(0x805F, 0x40);
  write_mem(0x8060, 0xA9); write_mem(0x8061, 0x05); write_mem(0x8062, 0x8D); write_mem(0x8063, 0x01); write_mem(0x8064, 0x40);
  write_mem(0x8065, 0xA9); write_mem(0x8066, 0x01); write_mem(0x8067, 0x8D); write_mem(0x8068, 0x01); write_mem(0x8069, 0x40);
  //  'o' = $6F
  write_mem(0x806A, 0xA9); write_mem(0x806B, 0x6F);
  write_mem(0x806C, 0x8D); write_mem(0x806D, 0x00); write_mem(0x806E, 0x40);
  write_mem(0x806F, 0xA9); write_mem(0x8070, 0x05); write_mem(0x8071, 0x8D); write_mem(0x8072, 0x01); write_mem(0x8073, 0x40);
  write_mem(0x8074, 0xA9); write_mem(0x8075, 0x01); write_mem(0x8076, 0x8D); write_mem(0x8077, 0x01); write_mem(0x8078, 0x40);
  //  ' ' = $20
  write_mem(0x8079, 0xA9); write_mem(0x807A, 0x20);
  write_mem(0x807B, 0x8D); write_mem(0x807C, 0x00); write_mem(0x807D, 0x40);
  write_mem(0x807E, 0xA9); write_mem(0x807F, 0x05); write_mem(0x8080, 0x8D); write_mem(0x8081, 0x01); write_mem(0x8082, 0x40);
  write_mem(0x8083, 0xA9); write_mem(0x8084, 0x01); write_mem(0x8085, 0x8D); write_mem(0x8086, 0x01); write_mem(0x8087, 0x40);
  //  '6' = $36
  write_mem(0x8088, 0xA9); write_mem(0x8089, 0x36);
  write_mem(0x808A, 0x8D); write_mem(0x808B, 0x00); write_mem(0x808C, 0x40);
  write_mem(0x808D, 0xA9); write_mem(0x808E, 0x05); write_mem(0x808F, 0x8D); write_mem(0x8090, 0x01); write_mem(0x8091, 0x40);
  write_mem(0x8092, 0xA9); write_mem(0x8093, 0x01); write_mem(0x8094, 0x8D); write_mem(0x8095, 0x01); write_mem(0x8096, 0x40);
  //  '5' = $35
  write_mem(0x8097, 0xA9); write_mem(0x8098, 0x35);
  write_mem(0x8099, 0x8D); write_mem(0x809A, 0x00); write_mem(0x809B, 0x40);
  write_mem(0x809C, 0xA9); write_mem(0x809D, 0x05); write_mem(0x809E, 0x8D); write_mem(0x809F, 0x01); write_mem(0x80A0, 0x40);
  write_mem(0x80A1, 0xA9); write_mem(0x80A2, 0x01); write_mem(0x80A3, 0x8D); write_mem(0x80A4, 0x01); write_mem(0x80A5, 0x40);
  //  '0' = $30
  write_mem(0x80A6, 0xA9); write_mem(0x80A7, 0x30);
  write_mem(0x80A8, 0x8D); write_mem(0x80A9, 0x00); write_mem(0x80AA, 0x40);
  write_mem(0x80AB, 0xA9); write_mem(0x80AC, 0x05); write_mem(0x80AD, 0x8D); write_mem(0x80AE, 0x01); write_mem(0x80AF, 0x40);
  write_mem(0x80B0, 0xA9); write_mem(0x80B1, 0x01); write_mem(0x80B2, 0x8D); write_mem(0x80B3, 0x01); write_mem(0x80B4, 0x40);
  //  '2' = $32
  write_mem(0x80B5, 0xA9); write_mem(0x80B6, 0x32);
  write_mem(0x80B7, 0x8D); write_mem(0x80B8, 0x00); write_mem(0x80B9, 0x40);
  write_mem(0x80BA, 0xA9); write_mem(0x80BB, 0x05); write_mem(0x80BC, 0x8D); write_mem(0x80BD, 0x01); write_mem(0x80BE, 0x40);
  write_mem(0x80BF, 0xA9); write_mem(0x80C0, 0x01); write_mem(0x80C1, 0x8D); write_mem(0x80C2, 0x01); write_mem(0x80C3, 0x40);
  //  '!' = $21
  write_mem(0x80C4, 0xA9); write_mem(0x80C5, 0x21);
  write_mem(0x80C6, 0x8D); write_mem(0x80C7, 0x00); write_mem(0x80C8, 0x40);
  write_mem(0x80C9, 0xA9); write_mem(0x80CA, 0x05); write_mem(0x80CB, 0x8D); write_mem(0x80CC, 0x01); write_mem(0x80CD, 0x40);
  write_mem(0x80CE, 0xA9); write_mem(0x80CF, 0x01); write_mem(0x80D0, 0x8D); write_mem(0x80D1, 0x01); write_mem(0x80D2, 0x40);
  //  $80D3: 4C D3 80  JMP $80D3   (loop forever)
  write_mem(0x80D3, 0x4C); write_mem(0x80D4, 0xD3); write_mem(0x80D5, 0x80);

  // Reset-sekvens
  for (int i = 0; i < 5; i++) pulse();
  digitalWrite(RESB, HIGH);

  // ~300 cykler för hela LCD-init + Hello
  for (int i = 0; i < 300; i++) pulse();
}

void loop() {
  // Kör kontinuerligt så vi ser JMP-loopen
  pulse();
}
