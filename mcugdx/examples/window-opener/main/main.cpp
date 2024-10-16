#include "mcugdx.h"
#include "motor.h"
#include "webserver.h"
#include "bme280.h"

#define TAG "Window opener"

extern "C" void app_main(void) {
	mcugdx_init();
	mcugdx_button_create(3, 50, MCUGDX_KEY_SPACE);
    motor_init();
    bme280_init(4, 5);
    webserver_init();

	while (true) {
		mcugdx_button_event_t event;
		while (mcugdx_button_get_event(&event)) {
			if (event.type == MCUGDX_BUTTON_PRESSED) {
				mcugdx_log(TAG, "Button pressed");
                motor_toggle();
			}
		}

		motor_update();
        mcugdx_log(TAG, "Sampling sensor");
        bme280_update();
        mcugdx_log(TAG, "%f %f %f", bme280_temperature(), bme280_pressure(), bme280_humidity());
		mcugdx_sleep(1000);
	}
}