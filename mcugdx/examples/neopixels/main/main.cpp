#include "mcugdx.h"
#include <stdio.h>

#define TAG "Neopixels example"

extern "C" void app_main() {
	mcugdx_neopixels_config_t config = {
			.num_leds = 31,
			.pin = 5};
	if (!mcugdx_neopixels_init(&config)) return;
	uint8_t brightness = 255;
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
		mcugdx_log(TAG, "mA: %li", mcugdx_neopixels_power_usage_milli_ampere());
		mcugdx_neopixels_show_max_milli_ampere(60);
		mcugdx_sleep(2000);
	}
}
