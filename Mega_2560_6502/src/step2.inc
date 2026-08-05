// Steg 2 — Adressbuss och reset-sekvens
// CPU:ns A0–A15 läses av Arduino. Reset-vektorn $FFFC/$FFFD verifieras.

#include <Arduino.h>

#define PHI2 2
#define RESB 4

void pulse() {
  digitalWrite(PHI2, LOW);
  delay(10);
  digitalWrite(PHI2, HIGH);
  delay(10);
}

void setup() {
  Serial.begin(115200);
  pinMode(PHI2, OUTPUT);
  pinMode(RESB, OUTPUT);
  DDRK = 0x00;  // PORTK (A8–A15) = INPUT
  DDRF = 0x00;  // PORTF (A0–A7)  = INPUT

  Serial.println("Steg 2 — Adressbuss och reset");

  // Reset-sekvens
  digitalWrite(RESB, LOW);
  for (int i = 0; i < 5; i++) pulse();
  digitalWrite(RESB, HIGH);

  // CPU:n läser reset-vektorn
  for (int i = 0; i < 10; i++) pulse();
  Serial.println("Klar — nu loopar vi:");
}

void loop() {
  digitalWrite(PHI2, LOW);
  delay(10);

  uint16_t addr = (PINK << 0) | (PINF << 8);

  digitalWrite(PHI2, HIGH);
  delay(10);

  Serial.print("R $");
  Serial.println(addr, HEX);
}
