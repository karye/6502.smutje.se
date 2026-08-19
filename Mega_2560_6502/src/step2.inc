// ==============================================================
// Steg 2 — Adressbuss och reset-sekvens
// ==============================================================
// CPU:ns 16 adresslinjer (A0–A15) kopplas till Arduinons analoga
// portar. Efter kontrollerad reset läser CPU:n alltid från
// $FFFC och $FFFD — reset-vektorn — oavsett vad som finns där.

#include <Arduino.h>

// --- Pindefinitioner ---
#define PHI2 2   // Arduino D2 → CPU pin 37 (klocka)
#define RESB 4   // Arduino D4 → CPU pin 40 (reset, aktiv låg)

#define CLOCK_HZ 500                         // Klockfrekvens i Hz
#define CLOCK_DELAY (1000 / (CLOCK_HZ * 2))  // ms per klockfas

// --- pulse() — en komplett klockcykel ---
// PHI2 växlar LÅG→HÖG→LÅG. CPU:n lägger ut nästa adress
// på stigande flank och håller den stabil under hela höga fasen.
void pulse() {
  digitalWrite(PHI2, LOW);
  delay(CLOCK_DELAY);         // Låg fas — CPU:n förbereder nästa adress
  digitalWrite(PHI2, HIGH);
  delay(CLOCK_DELAY);         // Hög fas — adressen är giltig
}

// --- setup() — konfigurera och kör reset-sekvens ---
void setup() {
  Serial.begin(115200);
  pinMode(PHI2, OUTPUT);       // Arduino driver klockan
  pinMode(RESB, OUTPUT);       // Arduino styr reset

  // Portregister — läs 8 bitar åt gången (mycket snabbare än digitalRead!)
  DDRK = 0x00;                 // PORTK (A8–A15) = INPUT, läser CPU A8–A15
  DDRF = 0x00;                 // PORTF (A0–A7)  = INPUT, läser CPU A0–A7

  Serial.println("Steg 2 — Adressbuss och reset");

  // --- Reset-sekvens ---
  // W65C02S kräver minst 2 klockcykler med RESB låg. Vi kör 5.
  digitalWrite(RESB, LOW);     // Håll CPU i reset
  for (int i = 0; i < 5; i++) pulse();
  digitalWrite(RESB, HIGH);    // Släpp reset — CPU:n startar!

  // CPU:n läser nu automatiskt reset-vektorn $FFFC/$FFFD.
  // Detta är hårdkodat i kislet — ingen mjukvara styr det.
  for (int i = 0; i < 10; i++) pulse();
  Serial.println("Klar — nu loopar vi:");
}

// --- loop() — läs och skriv ut adressbussen i varje cykel ---
void loop() {
  digitalWrite(PHI2, LOW);
  delay(CLOCK_DELAY);

  // Läs adressbussen medan PHI2 är låg — adressen är redan giltig
  uint16_t addr = (PINF << 0) | (PINK << 8);
  // PINF = CPU A0–A7  (låg byte)
  // PINK = CPU A8–A15 (hög byte)

  digitalWrite(PHI2, HIGH);
  delay(CLOCK_DELAY);

  // Skriv ut till serieövervakaren
  Serial.print("R $");          // R = Read (CPU:n läser alltid i steg 2)
  Serial.println(addr, HEX);   // Adress i hexadecimalt format
}
