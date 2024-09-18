#include "mcugdx.h"
#include <stdio.h>

#define TAG "Ultrasonic example"

extern "C" void app_main() {
	mcugdx_neopixels_config_t config = {
			.num_leds = 31,
			.pin = 1};
	if (!mcugdx_neopixels_init(&config)) return;
	constexpr uint8_t brightness = 32;
	while (true) {
		for (int i = 0; i < config.num_leds; i++) {
			int r = 0, g = 0, b = 0;

			switch (i % 3) {
				case 0:
					r = brightness;
					break;
				case 1:
					g = brightness;
					break;
				case 2:
					b = brightness;
					break;
			}

			mcugdx_neopixels_set(i, r, g, b);
		}
		mcugdx_neopixels_show();
		mcugdx_sleep(2000);
	}
}
