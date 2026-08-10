#include <Arduino.h>

// ==============================================================
// Steg 7 — VIA + LCD (LCD styrs av 6502 via W65C22 VIA)
// ==============================================================
// 6502-programmet initierar VIA + LCD och skriver 4 rader text.
// Arduino är endast minnesemulator + klocka — all I/O via VIA.

#define PHI2     2   // CPU pin 37
#define RESB     4   // CPU pin 40
#define CLOCK_HZ 500                         // Klockfrekvens i Hz
#define CLOCK_DELAY (1000 / (CLOCK_HZ * 2))  // ms per klockfas
#define RWB      3   // CPU pin 34
#define SYNC     13  // CPU pin 7
#define BTN_CLK  11  // Knapp: en klockcykel per tryck
#define BTN_INSTR 12 // Knapp: kör till nästa instruktion

uint8_t ram[1024];       // $0000–$03FF (stack + zero page)
uint8_t program[2048];   // $8000–$87FF (6502-program, Arduino svarar)
uint8_t vectors[6];      // $FFFA–$FFFF

#define VIA_BASE 0x4000

// --- is_via() — är detta en VIA-adress? ---
// VIA sitter på $4000–$400F. Arduino ska INTE svara här —
// den fysiska W65C22-kretsen hanterar dessa adresser själv.
bool is_via(uint16_t addr) {
  return (addr >= VIA_BASE && addr < VIA_BASE + 16);
}

// --- read_mem() — slå upp ett minnesvärde ---
// Returnerar 0 för VIA-adresser (Arduino tri-state, VIA svarar).
// Returnerar NOP ($EA) som fallback för oanvända adresser.
uint8_t read_mem(uint16_t addr) {
  if (is_via(addr)) return 0;
  if (addr >= 0xFFFA && addr <= 0xFFFF)
    return vectors[addr - 0xFFFA];
  if (addr >= 0x8000 && addr < 0x8000 + sizeof(program))
    return program[addr - 0x8000];
  if (addr < sizeof(ram)) return ram[addr];
  return 0xEA;
}

// --- write_mem() — spara ett värde i minnet ---
// Ignorera VIA-adresser — den fysiska kretsen tar emot skrivningen.
void write_mem(uint16_t addr, uint8_t val) {
  if (is_via(addr)) return;
  if (addr >= 0xFFFA && addr <= 0xFFFF)
    { vectors[addr - 0xFFFA] = val; return; }
  if (addr >= 0x8000 && addr < 0x8000 + sizeof(program))
    { program[addr - 0x8000] = val; return; }
  if (addr < sizeof(ram)) ram[addr] = val;
}

void pulse();  // Forward declaration

// --- pulse() — en klockcykel med minnesemulering + VIA-pass-through ---
// Kritiskt: DDRA = 0x00 direkt när PHI2 går låg, INNAN delay().
// Arduino svarar ENDAST på adresser utanför VIA-området.
void pulse() {
  digitalWrite(PHI2, LOW);

  // Släpp databussen EFTER klockfall — håll kvar data under CPU:ns hold time (~10ns)
  DDRA = 0x00;
  delay(CLOCK_DELAY);

  uint8_t kl = PINF;   // CPU A0–A7 (via Arduino A0–A7)
  uint8_t kh = PINK;   // CPU A8–A15 (via Arduino A8–A15)
  uint16_t a = (kl << 0) | (kh << 8);
  bool rw = digitalRead(RWB);

  if (rw && !is_via(a)) {
    DDRA = 0xFF;
    PORTA = read_mem(a);
  }

  // -- Fas 2: PHI2 hög --
  digitalWrite(PHI2, HIGH);

  if (!rw && !is_via(a)) write_mem(a, PINA);

  // === Debug: råa portvärden + fasdetektion ===
  static uint8_t phase = 0;
  static uint16_t last_a = 0xFFFF;
  uint8_t new_phase = phase;
  
  // Fasövergångar
  if      (a == 0xFFFC && last_a != 0xFFFC)           new_phase = 1; // Reset-vektor
  else if (a >= 0x8000 && a <= 0x800F && rw && phase <= 1) new_phase = 2; // Programstart
  else if (a == 0x4002 && !rw && phase <= 2)           new_phase = 3; // VIA init
  else if (a == 0x4000 && !rw && phase == 3)           new_phase = 4; // LCD init
  else if (rw && a >= 0x8000 && phase == 5)            new_phase = 6; // Hello JMP loop
  
  if (new_phase != phase) {
    phase = new_phase;
    Serial.println();
    switch (phase) {
      case 1: Serial.println("══════ RESET-SEKVENS — CPU lämnar reset ══════"); break;
      case 2: Serial.println("══════ PROGRAMSTART $8000 — LDA #$FF ══════");    break;
      case 3: Serial.println("══════ VIA INIT — Sätter portar som utgångar ══════"); break;
      case 4: Serial.println("══════ LCD INIT — Skickar init-kommandon ══════");    break;
      case 5: Serial.println("══════ HELLO UTSKRIFT — Skriver 'Hello 6502!' ══════"); break;
      case 6: Serial.println("══════ KLAR — JMP loop (skriver om) ══════");        break;
    }
  }
  last_a = a;
  
  Serial.print(rw ? "R" : "W");
  Serial.print(" $");
  Serial.print(a, HEX);
  Serial.print(" [PINK=0x");  Serial.print(PINK, HEX);
  Serial.print(" PINF=0x");   Serial.print(PINF, HEX);
  Serial.print(" RWB=");      Serial.print(rw ? "H" : "L");
  
  if      (a >= 0x4000 && !rw) { Serial.print("  ← VIA: "); Serial.print(PINA, HEX); }
  else if (a >= 0x8000 && rw)   Serial.print("  ← OPCODE");
  else if (a == 0xFFFC)         Serial.print("  ← RESET-VEKTOR LÅG");
  else if (a == 0xFFFD)         Serial.print("  ← RESET-VEKTOR HÖG");
  else if (a < 0x200)          Serial.print("  ← STACK/ZP");
  
  Serial.println();
}

// --- setup() — ladda 6502-program och starta CPU ---
void setup() {
  // CPU i reset från start — avgörande!
  pinMode(RESB, OUTPUT); digitalWrite(RESB, LOW);
  pinMode(PHI2, OUTPUT); digitalWrite(PHI2, LOW);

  pinMode(SYNC, INPUT);
  pinMode(BTN_CLK,   INPUT_PULLUP);
  pinMode(BTN_INSTR, INPUT_PULLUP);
  pinMode(RWB, INPUT);
  pinMode(8, INPUT);  // Övervaka E  (VIA PA2 → LCD pin 6)
  pinMode(9, INPUT);  // Övervaka RS (VIA PA0 → LCD pin 4)
  Serial.begin(115200);

  DDRK = 0x00;  // A8–A15 in
  DDRF = 0x00;  // A0–A7  in
  DDRA = 0x00;

  // Reset-vektor → $8000
  write_mem(0xFFFC, 0x00);
  write_mem(0xFFFD, 0x80);

  // Sekventiell programladdning — next räknas upp automatiskt
  uint16_t next = 0x8000;

  // === Steg 1: Sätt VIA-portar som utgångar ===
  // LDA #$FF
  write_mem(next++, 0xA9); write_mem(next++, 0xFF);
  // STA $4002  (DDRB)
  write_mem(next++, 0x8D); write_mem(next++, 0x02); write_mem(next++, 0x40);
  // STA $4003  (DDRA)
  write_mem(next++, 0x8D); write_mem(next++, 0x03); write_mem(next++, 0x40);

  // === Steg 3: Skicka $30 tre gånger (tvinga 8-bit mode) ===
  for (int i = 0; i < 3; i++) {
    write_mem(next++, 0xA9); write_mem(next++, 0x30);  // LDA #$30
    write_mem(next++, 0x8D); write_mem(next++, 0x00); write_mem(next++, 0x40); // STA $4000
    write_mem(next++, 0xA9); write_mem(next++, 0x04);  // LDA #$04 (RS=0, E=1)
    write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40); // STA $4001
    write_mem(next++, 0xA9); write_mem(next++, 0x00);  // LDA #$00 (RS=0, E=0)
    write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40); // STA $4001
  }

  // === Steg 4: Function Set: 8-bit, 2 rader, 5x8 font ===
  write_mem(next++, 0xA9); write_mem(next++, 0x38);  // LDA #$38
  write_mem(next++, 0x8D); write_mem(next++, 0x00); write_mem(next++, 0x40); // STA $4000
  write_mem(next++, 0xA9); write_mem(next++, 0x04);  // LDA #$04 (RS=0, E=1)
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40); // STA $4001
  write_mem(next++, 0xA9); write_mem(next++, 0x00);  // LDA #$00 (RS=0, E=0)
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40); // STA $4001

  // === Steg 5: Display ON, cursor OFF, blink OFF ===
  write_mem(next++, 0xA9); write_mem(next++, 0x0C);  // LDA #$0C
  write_mem(next++, 0x8D); write_mem(next++, 0x00); write_mem(next++, 0x40); // STA $4000
  write_mem(next++, 0xA9); write_mem(next++, 0x04);  // LDA #$04
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40); // STA $4001
  write_mem(next++, 0xA9); write_mem(next++, 0x00);  // LDA #$00
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40); // STA $4001

  // === Steg 6: Clear Display ===
  write_mem(next++, 0xA9); write_mem(next++, 0x01);  // LDA #$01
  write_mem(next++, 0x8D); write_mem(next++, 0x00); write_mem(next++, 0x40); // STA $4000
  write_mem(next++, 0xA9); write_mem(next++, 0x04);  // LDA #$04
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40); // STA $4001
  write_mem(next++, 0xA9); write_mem(next++, 0x00);  // LDA #$00
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40); // STA $4001

  // === Steg 7: Entry Mode: increment, no shift ===
  write_mem(next++, 0xA9); write_mem(next++, 0x06);  // LDA #$06
  write_mem(next++, 0x8D); write_mem(next++, 0x00); write_mem(next++, 0x40); // STA $4000
  write_mem(next++, 0xA9); write_mem(next++, 0x04);  // LDA #$04
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40); // STA $4001
  write_mem(next++, 0xA9); write_mem(next++, 0x00);  // LDA #$00
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40); // STA $4001

  // === Hello-rutin: 4 rader → clear → loop ===
  uint16_t hello_start = next;

  // -- Hjälpfunktion: skicka LCD-kommando --
  // Vi bygger inline: LDA #cmd, STA $4000, LDA #$04, STA $4001, LDA #$00, STA $4001
  // (RS=0 för kommando, E-puls)

  // Rad 1: "=== 6502 VIA LCD ==="
  // Först: sätt cursor till rad 1 (DDRAM $00): kommando $80
  write_mem(next++, 0xA9); write_mem(next++, 0x80);  // LDA #$80
  write_mem(next++, 0x8D); write_mem(next++, 0x00); write_mem(next++, 0x40); // STA $4000
  write_mem(next++, 0xA9); write_mem(next++, 0x04);  // LDA #$04 (RS=0, E=1)
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40); // STA $4001
  write_mem(next++, 0xA9); write_mem(next++, 0x00);  // LDA #$00 (E=0)
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40); // STA $4001

  // RS=1 (data-läge)
  write_mem(next++, 0xA9); write_mem(next++, 0x01);  // LDA #$01
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40); // STA $4001

  const char* line1 = "W65C02 VIA LCD";
  for (const char* p = line1; *p; p++) {
    write_mem(next++, 0xA9); write_mem(next++, *p);
    write_mem(next++, 0x8D); write_mem(next++, 0x00); write_mem(next++, 0x40);
    write_mem(next++, 0xA9); write_mem(next++, 0x05);
    write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
    write_mem(next++, 0xA9); write_mem(next++, 0x01);
    write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
  }

  // Rad 2: cursor till $40 (kommando $C0)
  write_mem(next++, 0xA9); write_mem(next++, 0xC0);  // LDA #$C0
  write_mem(next++, 0x8D); write_mem(next++, 0x00); write_mem(next++, 0x40);
  write_mem(next++, 0xA9); write_mem(next++, 0x04);
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
  write_mem(next++, 0xA9); write_mem(next++, 0x00);
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);

  write_mem(next++, 0xA9); write_mem(next++, 0x01);  // RS=1
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);

  // Rad 2: smutje.se W65C02
  const char* line2 = "smutje.se W65C02";
  for (const char* p = line2; *p; p++) {
    write_mem(next++, 0xA9); write_mem(next++, *p);
    write_mem(next++, 0x8D); write_mem(next++, 0x00); write_mem(next++, 0x40);
    write_mem(next++, 0xA9); write_mem(next++, 0x05);
    write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
    write_mem(next++, 0xA9); write_mem(next++, 0x01);
    write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
  }

  // Clear display (kommando $01)
  write_mem(next++, 0xA9); write_mem(next++, 0x01);  // LDA #$01
  write_mem(next++, 0x8D); write_mem(next++, 0x00); write_mem(next++, 0x40);
  write_mem(next++, 0xA9); write_mem(next++, 0x04);
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);
  write_mem(next++, 0xA9); write_mem(next++, 0x00);
  write_mem(next++, 0x8D); write_mem(next++, 0x01); write_mem(next++, 0x40);

  // Vänta lite efter clear (clear tar ~1.6ms) — skicka några NOP/read
  // Vi gör bara JMP tillbaka till start istället:
  write_mem(next++, 0x4C);                            // JMP
  write_mem(next++, hello_start & 0xFF);               // låg byte
  write_mem(next++, (hello_start >> 8) & 0xFF);        // hög byte

  // Reset-sekvens
  for (int i = 0; i < 5; i++) pulse();
  digitalWrite(RESB, HIGH);

  // ~2000 cykler för det stora 4-radersprogrammet (ca 40 sek vid 20 Hz)
  for (int i = 0; i < 2000; i++) pulse();
}

void loop() {
  // Kör kontinuerligt så vi ser JMP-loopen
  pulse();
}
