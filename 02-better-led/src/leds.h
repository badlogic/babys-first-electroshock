#pragma once
#include <Arduino.h>

template <size_t N> struct LedAnimationFrame {
  uint8_t led_states[N];
  unsigned long duration;
};

template <size_t N> struct LedAnimation {
  uint8_t led_pins[N];
  LedAnimationFrame<N> *frames;
  unsigned long last_time;
  int current_frame;

  LedAnimation(const uint8_t _led_pins[N], const LedAnimationFrame<N> *_frames)
      : frames(_frames), last_time(0xffffffff), current_frame(0) {
    for (size_t i = 0; i < N; i++) {
      led_pins[i] = _led_pins[i];
    }
  }

  void update() {
    unsigned long time = millis();

    // First time we update, set things up and show the first frame.
    if (last_time == 0xffffffff) {
      last_time = time;
      current_frame = 0;
      for (size_t i = 0; i < N; ++i) {
        digitalWrite(led_pins[i], frames[current_frame].led_states[i]);
      }
      return;
    }

    // Has this frame been shown long enough? Switch to the next.
    if (time - last_time >= frames[current_frame].duration) {
      last_time = time;
      current_frame++;

      // Last frame reached? Wrap around.
      if (frames[current_frame].duration == 0xFFFFFFFF) {
        current_frame = 0;
      }

      // Display the frame.
      for (size_t i = 0; i < N; ++i) {
        digitalWrite(led_pins[i], frames[current_frame].led_states[i]);
      }
    }
  }

  void reset() {
    current_frame = 0;
    last_time = 0xffffffff;
  }
};