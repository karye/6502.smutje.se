// ==============================================================
// Steg 4 — Manuell stegning med knappar
// ==============================================================
// Två tryckknappar ger full kontroll över exekveringen:
// Knapp 1 (D11): en klockcykel per tryck
// Knapp 2 (D12): kör tills CPU:n påbörjar nästa instruktion (SYNC)
// W65C02S är statisk CMOS — klockan kan stoppas hur länge som helst.

#include <Arduino.h>

// --- Pindefinitioner ---
#define PHI2     2    // Arduino D2 → CPU pin 37 (klocka)
#define RESB     4    // Arduino D4 → CPU pin 40 (reset)
#define RWB      3    // Arduino D3 → CPU pin 34 (Read/Write)
#define SYNC     13   // Arduino D13 → CPU pin 7 (opcode fetch-indikator)
#define BTN_CLK  11   // Knapp 1: en klockcykel (D11 → GND, INPUT_PULLUP)
#define BTN_INSTR 12  // Knapp 2: en instruktion (D12 → GND, INPUT_PULLUP)

#define CLOCK_HZ 500                         // Klockfrekvens i Hz
#define CLOCK_DELAY (1000 / (CLOCK_HZ * 2))  // ms per klockfas

// --- Minnesarrayer ---
uint8_t ram[1024];       // $0000–$03FF
uint8_t vectors[6];      // $FFFA–$FFFF

// --- read_mem() — slå upp minnesvärde ---
uint8_t read_mem(uint16_t addr) {
  if (addr >= 0xFFFA && addr <= 0xFFFF) return vectors[addr - 0xFFFA];
  if (addr < 1024) return ram[addr];
  return 0xEA;  // NOP — ofarlig fallback
}

// --- write_mem() — spara minnesvärde ---
void write_mem(uint16_t addr, uint8_t val) {
  if (addr >= 0xFFFA && addr <= 0xFFFF)
    { vectors[addr - 0xFFFA] = val; return; }
  if (addr < 1024) ram[addr] = val;
}

// --- pulse() — en klockcykel med minnesemulering ---
void pulse() {
  digitalWrite(PHI2, LOW);
  delay(CLOCK_DELAY);

  uint16_t a = (PINK << 0) | (PINF << 8);
  bool rw = digitalRead(RWB);

  // Styr databussen: ut vid läsning, tri-state vid skrivning
  if (rw) { DDRA = 0xFF; PORTA = read_mem(a); }
  else    { DDRA = 0x00; }

  digitalWrite(PHI2, HIGH);
  delay(CLOCK_DELAY);

  if (!rw) write_mem(a, PINA);

  // Diagnostik
  Serial.print(rw ? "R" : "W");
  Serial.print(" $");
  Serial.println(a, HEX);
}

// --- setup() — konfigurera pinnar och ladda NOP-program ---
void setup() {
  Serial.begin(115200);
  pinMode(PHI2, OUTPUT);
  pinMode(RESB, OUTPUT);
  pinMode(RWB,  INPUT);
  pinMode(SYNC, INPUT);          // SYNC = HÖG när CPU hämtar ny opcode
  pinMode(BTN_CLK,   INPUT_PULLUP);   // Intern pull-up, knapp till GND = LÅG vid tryck
  pinMode(BTN_INSTR, INPUT_PULLUP);   // Intern pull-up, knapp till GND = LÅG vid tryck
  DDRK = 0x00; DDRF = 0x00; DDRA = 0x00;

  // Ladda NOP-program (samma som steg 3)
  write_mem(0xFFFC, 0x00);
  write_mem(0xFFFD, 0x80);
  write_mem(0x8000, 0xEA);

  Serial.println("Steg 4 — Knappstegning");
  Serial.println("Knapp 1 = klocksteg, Knapp 2 = instruktionssteg");

  // Reset-sekvens
  digitalWrite(RESB, LOW);
  for (int i = 0; i < 5; i++) pulse();
  digitalWrite(RESB, HIGH);
}

// --- loop() — vänta på knapptryck, kör sedan ---
// INPUT_PULLUP gör att pinnen läses som LÅG när knappen trycks in.
// Debounce: 50 ms väntan efter tryck och efter släpp.
void loop() {
  // --- Knapp 1: en klockcykel per tryck ---
  if (!digitalRead(BTN_CLK)) {       // Knapp nedtryckt? (LÅG = tryckt)
    delay(50);                       // Debounce — vänta ut kontaktstudsar
    while (!digitalRead(BTN_CLK));   // Vänta på att knappen släpps
    delay(50);                       // Debounce efter släpp
    pulse();                         // Kör en klockcykel
  }

  // --- Knapp 2: kör till nästa instruktion ---
  // SYNC går HÖG när CPU:n hämtar första byten av en ny instruktion.
  // Vi pulserar klockan tills SYNC blir hög → CPU:n är i starten av
  // nästa instruktion. Perfekt för att stega genom programmet!
  if (!digitalRead(BTN_INSTR)) {
    delay(50);
    while (!digitalRead(BTN_INSTR));
    delay(50);
    do { pulse(); } while (!digitalRead(SYNC));
  }
}
