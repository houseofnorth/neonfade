#include "Leds.h"
#include "config.h"

void Leds::init() {
  pinMode(LED_A, OUTPUT);
  pinMode(LED_B, OUTPUT);
  off();
}

void Leds::off()   { digitalWrite(LED_A, LOW);  digitalWrite(LED_B, LOW);  }
void Leds::left()  { digitalWrite(LED_A, HIGH); digitalWrite(LED_B, LOW);  }
void Leds::right() { digitalWrite(LED_A, LOW);  digitalWrite(LED_B, HIGH); }
void Leds::both()  { digitalWrite(LED_A, HIGH); digitalWrite(LED_B, HIGH); }

void Leds::update(MotorDir dir, bool buttonHeld) {
  if (buttonHeld)     { both();  return; }
  if (dir == FORWARD) { left();  return; }
  if (dir == REVERSE) { right(); return; }
  off();
}

void Leds::blinkOne(int pin, int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH); delay(80);
    digitalWrite(pin, LOW);  delay(80);
  }
}

void Leds::blinkBoth(int times) {
  for (int i = 0; i < times; i++) {
    both(); delay(200);
    off();  delay(200);
  }
}
