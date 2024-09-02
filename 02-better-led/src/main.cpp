#include <Arduino.h>
#include <leds.h>

constexpr uint8_t led_pins[] = {2, 3, 4, 5};
constexpr uint8_t num_leds = sizeof(led_pins) / sizeof(led_pins[0]);

LedAnimationFrame<num_leds> frames[] = {
    {1, 0, 0, 0, 250}, //
    {0, 1, 0, 0, 250}, //
    {0, 0, 1, 0, 250}, //
    {0, 0, 0, 1, 250}, //
    {0, 0, 1, 0, 250}, //
    {0, 1, 0, 0, 250}, //
    {0, 0, 0, 0, 0xffffffff},
};
LedAnimation<num_leds> animation(led_pins, frames);

void setup() {
  for (int i = 0; i < num_leds; i++) {
    pinMode(led_pins[i], OUTPUT);
  }
}

void loop() { animation.update(); }
