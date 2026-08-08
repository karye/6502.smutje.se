// ==============================================================
// Steg 1 — Strömmatning och klocka
// ==============================================================
// CPU:n får +5V, GND och en långsam klockpuls från Arduino D2.
// En lysdiod på klocklinjen blinkar i takt med pulsen.
// Inga andra anslutningar — adress- och databuss är inte inkopplade.

#include <Arduino.h>

// --- Pindefinitioner ---
#define PHI2 2   // Arduino D2 → CPU pin 37 (klockingång)
#define RESB 4   // Arduino D4 → CPU pin 40 (reset, hålls låg tills vidare)

#define CLOCK_HZ 1                            // Klockfrekvens i Hz (1 Hz = synlig blink)
#define CLOCK_DELAY (1000 / (CLOCK_HZ * 2))  // ms per klockfas

// --- pulse() — generera en komplett klockcykel ---
// W65C02S är statisk CMOS — klockan kan vara hur långsam som helst.
// Här: 500 ms hög + 500 ms låg = 1 Hz, perfekt för att se lysdioden blinka.
void pulse() {
  digitalWrite(PHI2, HIGH);   // Stigande flank — CPU:n arbetar (lysdiod lyser)
  delay(500);                 // 500 ms med lysdiod tänd
  digitalWrite(PHI2, LOW);    // Fallande flank — nästa cykel (lysdiod släckt)
  delay(500);                 // 500 ms med lysdiod släckt
}

// --- setup() — konfigurera pinnar, körs en gång vid start ---
void setup() {
  pinMode(PHI2, OUTPUT);       // Arduino driver klockan
  pinMode(RESB, OUTPUT);       // Arduino håller CPU i reset

  digitalWrite(RESB, LOW);     // CPU i reset — gör ingenting, bussarna tri-state
  digitalWrite(PHI2, LOW);     // Klockan börjar låg

  Serial.begin(115200);
  Serial.println("Steg 1 — Klocka och ström");
  Serial.println("Lysdioden på PHI2 ska blinka 1 Hz");
}

// --- loop() — repeteras i oändlighet ---
void loop() {
  pulse();  // En klockcykel → lysdioden blinkar för alltid
}
