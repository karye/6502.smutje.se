// Steg 1 — Strömmatning och klocka
// CPU:n får ström och klockpuls. Lysdiod på klocklinjen blinkar.

#include <Arduino.h>

#define PHI2 2   // Arduino D2 → CPU pin 37
#define RESB 4   // Arduino D4 → CPU pin 40
#define CLOCK_DELAY 1   // ms per klockfas (1 = ~500 Hz, 10 = ~50 Hz)

void pulse() {
  digitalWrite(PHI2, HIGH);
  delay(500);
  digitalWrite(PHI2, LOW);
  delay(500);
}

void setup() {
  pinMode(PHI2, OUTPUT);
  pinMode(RESB, OUTPUT);
  digitalWrite(RESB, LOW);  // CPU i reset (gör inget)
  digitalWrite(PHI2, LOW);
  Serial.begin(115200);
  Serial.println("Steg 1 — Klocka och ström");
  Serial.println("Lysdioden på PHI2 ska blinka 1 Hz");
}

void loop() {
  pulse();  // Blinka lysdioden i oändlighet
}
