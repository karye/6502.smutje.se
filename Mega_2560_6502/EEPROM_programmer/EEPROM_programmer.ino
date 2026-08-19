// ==============================================================
// Steg 10 — EEPROM-programmerare (andra Arduino Mega)
// ==============================================================
// Tar emot en .bin-fil över serieporten, bränner den på
// AT28C256, och verifierar. Körs EN gång i setup().
//
// Använd med: python3 scripts/upload_eeprom.py asm/program_hello.bin /dev/ttyACM0
// Detta är en separat Arduino — INTE samma som datorns klocka/diagnostik.

#include <Arduino.h>

// --- Pindefinitioner ---
#define WE 2       // Arduino D2 → AT28C256 /WE (pin 27)
#define OE 3       // Arduino D3 → AT28C256 /OE (pin 22)

#define EEPROM_SIZE 32768  // 32 KB = 32 768 bytes

// --- Buffert för mottagen data ---
// Arduino Mega har 8 KB SRAM — för lite för hela 32 KB.
// Vi använder därför ett sid-indelat protokoll: 256 bytes åt gången.
uint8_t page[256];       // 256-byte sida
int pages_total = 0;     // Räkna antal mottagna sidor

// --- read_eeprom() — läs en byte från EEPROM ---
uint8_t read_eeprom(uint16_t addr) {
  // Sätt adress
  PORTF = addr & 0xFF;         // A0–A7  (PORTF = Arduino A0–A7)
  PORTK = (addr >> 8) & 0x7F;  // A8–A14 (PORTK = Arduino A8–A14, bit 7 oanvänd)

  DDRA = 0x00;                  // PORTA = INPUT (tri-state)
  digitalWrite(OE, LOW);        // /OE låg → EEPROM kör ut data
  delayMicroseconds(1);         // Vänta på utgångsdata
  uint8_t data = PINA;          // Läs D0–D7
  digitalWrite(OE, HIGH);       // /OE hög → EEPROM tri-state
  return data;
}

// --- write_eeprom() — bränn en byte till EEPROM ---
// AT28C256 har inbyggd timing — vi behöver bara hålla /WE
// låg i minst 10 ms så sköter kretsen resten.
void write_eeprom(uint16_t addr, uint8_t data) {
  // Sätt adress
  PORTF = addr & 0xFF;
  PORTK = (addr >> 8) & 0x7F;

  // Sätt data
  DDRA = 0xFF;                  // PORTA = OUTPUT
  PORTA = data;                 // Lägg ut data på D0–D7
  digitalWrite(OE, HIGH);       // /OE hög — vi ska inte läsa nu

  // Bränn: puls /WE låg i 10 ms
  digitalWrite(WE, LOW);
  delay(10);                    // 10 ms — gott om marginal
  digitalWrite(WE, HIGH);

  DDRA = 0x00;                  // Tri-state databussen
}

// --- setup() — ta emot data, bränn, verifiera ---
void setup() {
  Serial.begin(115200);
  pinMode(WE, OUTPUT);  digitalWrite(WE, HIGH);
  pinMode(OE, OUTPUT);  digitalWrite(OE, HIGH);
  DDRF = 0xFF;   // Adress A0–A7 = OUTPUT
  DDRK = 0xFF;   // Adress A8–A14 = OUTPUT
  DDRA = 0x00;   // Data = INPUT (tri-state från start)

  Serial.println("AT28C256 EEPROM-programmerare");
  Serial.println("Redo att ta emot .bin-fil...");
  Serial.println("Skicka data med: python3 upload_eeprom.py program.bin");

  // --- Ta emot data sida för sida ---
  // Protokoll: 256 bytes per sida. Efter sista sidan: "DONE".
  uint16_t total_addr = 0;
  pages_total = 0;

  while (true) {
    // Vänta på startsignal från Python-skriptet
    while (Serial.available() < 1);
    char cmd = Serial.read();

    if (cmd == 'P') {
      // "PAGE" — nästa 256 bytes är en sida
      pages_total++;
      int bytes_read = Serial.readBytes((char*)page, 256);
      if (bytes_read != 256) {
        Serial.print("FEL: endast ");
        Serial.print(bytes_read);
        Serial.println(" bytes mottagna (väntade 256)");
        return;
      }

      // Bränn sidan
      Serial.print("Bränner sida ");
      Serial.print(pages_total);
      Serial.print(" (adress $");
      Serial.print(total_addr, HEX);
      Serial.print(")... ");

      for (int i = 0; i < 256; i++) {
        write_eeprom(total_addr + i, page[i]);
      }
      Serial.println("OK");
      total_addr += 256;
    }
    else if (cmd == 'D') {
      // "DONE" — alla sidor mottagna
      Serial.println("Alla sidor mottagna.");
      break;
    }
    else {
      Serial.print("Okänt kommando: ");
      Serial.println(cmd);
    }
  }

  // --- Verifiera ---
  Serial.println("Verifierar...");
  int errors = 0;
  for (uint32_t addr = 0; addr < total_addr; addr++) {
    // Vi har inte kvar originaldatan i RAM på ett enkelt sätt
    // efter sidbränningen. I praktiken: använd ett Python-skript
    // som skickar om datan för verifiering, eller lagra checksummor.
    //
    // Här visar vi principen: läs tillbaka och låt Python-skriptet
    // jämföra.
    uint8_t val = read_eeprom(addr);
    Serial.print("V ");
    Serial.print(addr, HEX);
    Serial.print(" ");
    Serial.println(val, HEX);
  }

  Serial.println("KLAR");
  Serial.print("Totalt brända bytes: ");
  Serial.println(total_addr);
}

void loop() {
  // Tom — allt sker i setup()
}
