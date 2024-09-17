#include "mcugdx.h"
#include <stdio.h>

#define TAG "Ultrasonic example"

extern "C" void app_main() {
	mcugdx_ultrasonic_config_t config = {
		.trigger = 1,
		.echo = 2
	};
	mcugdx_ultrasonic_init(&config);

	while (true) {
		uint32_t distance = 0;
		if (mcugdx_ultrasonic_measure(240, &distance)) {
			mcugdx_log(TAG, "Distance: %li", distance);
			printf(">min:0,d:%li,max:100\n", distance);
		}
		mcugdx_sleep(100);
	}
}
