#include "mcugdx.h"
#include "motor.h"
#include "webserver.h"
#include "bme280.h"
#include "config.h"

#define TAG "Window opener"

extern "C" void app_main(void) {
	mcugdx_init();
	mcugdx_button_create(3, 50, MCUGDX_KEY_SPACE);
	config_read();
	config_print();
    motor_init();
    bme280_init(4, 5);
    webserver_init();

	while (true) {
		mcugdx_button_event_t event;
		config_t *config = config_lock();
		bool manual = config->manual;
		int32_t min_temp = config->min_temp;
		int32_t max_temp = config->max_temp;
		while (mcugdx_button_get_event(&event)) {
			if (event.type == MCUGDX_BUTTON_PRESSED) {
				mcugdx_log(TAG, "Button pressed");
                motor_toggle();
				config->manual = manual = true;
				config_unlock();
				config_save();
				config = config_lock();
			}
		}
		config_unlock();

		bme280_update();
		float temp = bme280_temperature();

		if (!manual) {
			if (temp < (float)min_temp && !motor_is_closed()) {
				mcugdx_log(TAG, "Temperature %f < min temperature %li, closing window", temp, min_temp);
				motor_toggle();
			}
			if (temp > (float)max_temp && motor_is_closed()) {
				mcugdx_log(TAG, "Temperature %f > max temperature %li, opening window", temp, max_temp);
				motor_toggle();
			}
		}

		motor_update();
		config_unlock();
		mcugdx_sleep(100);
	}
}