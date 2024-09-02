#include <Arduino.h>

constexpr uint8_t led_pins[] = {2, 3, 4, 5};
constexpr uint8_t num_leds = sizeof(led_pins) / sizeof(led_pins[0]);

void leds_setup() {
  for (int i = 0; i < num_leds; i++) {
    pinMode(led_pins[i], OUTPUT);
  }
}

void leds_set_all(uint8_t value) {
  for (int i = 0; i < num_leds; i++) {
    digitalWrite(led_pins[i], value);
  }
}

void setup() {
  leds_setup();
}

void loop() {
  for (int i = 0; i < num_leds; i++) {
    leds_set_all(LOW);
    digitalWrite(led_pins[i], HIGH);
    delay(250);
  }
}