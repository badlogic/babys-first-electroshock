#include "motor.h"
#include "mcugdx.h"
#include "driver/gpio.h"

#define TAG "motor"

#define MOTOR_CLOSE_PIN (gpio_num_t) 1
#define MOTOR_OPEN_PIN (gpio_num_t) 2
#define MOTOR_ON_TIME_SECS 6

motor_t motor;

void motor_init() {
    mcugdx_mutex_init(&motor.mutex);
    	gpio_config_t io_conf = {
			.pin_bit_mask = (1ULL << MOTOR_CLOSE_PIN) | (1ULL << MOTOR_OPEN_PIN),
			.mode = GPIO_MODE_OUTPUT,
			.pull_up_en = GPIO_PULLUP_DISABLE,
			.pull_down_en = GPIO_PULLDOWN_DISABLE,
			.intr_type = GPIO_INTR_DISABLE};
	gpio_config(&io_conf);
    motor.is_closed = true;
    motor.state = MOTOR_IDLE;
}

void motor_print(void) {
	const char *state = NULL;
	switch (motor.state) {
		case MOTOR_CLOSING:
			state = "closing";
			break;
		case MOTOR_OPENING:
			state = "opening";
			break;
		default:
			state = "idle";
			break;
	}
	mcugdx_log(TAG, "is_closed: %s, state: %s", motor.is_closed ? "true" : "false", state);
}

void motor_update(void) {
    mcugdx_mutex_lock_l(&motor.mutex, __FILE__, __LINE__);
	switch (motor.state) {
		case MOTOR_CLOSING:
			gpio_set_level(MOTOR_CLOSE_PIN, 1);
			gpio_set_level(MOTOR_OPEN_PIN, 0);
			break;
		case MOTOR_OPENING:
			gpio_set_level(MOTOR_OPEN_PIN, 1);
			gpio_set_level(MOTOR_CLOSE_PIN, 0);
			break;
		default:
			gpio_set_level(MOTOR_CLOSE_PIN, 0);
			gpio_set_level(MOTOR_OPEN_PIN, 0);
			break;
	}

	if (mcugdx_time() - motor.on_start_time > MOTOR_ON_TIME_SECS && motor.state != MOTOR_IDLE) {
		mcugdx_log(TAG, "Motor on time reached");
		motor.state = MOTOR_IDLE;
		motor_print();
	}
    mcugdx_mutex_unlock_l(&motor.mutex, __FILE__, __LINE__);
}

void motor_toggle(void) {
    mcugdx_mutex_lock_l(&motor.mutex, __FILE__, __LINE__);
	if (motor.is_closed) {
		motor.state = MOTOR_OPENING;
		motor.is_closed = false;
	} else {
		motor.state = MOTOR_CLOSING;
		motor.is_closed = true;
	}
	motor.on_start_time = mcugdx_time();
    motor_print();
    mcugdx_mutex_unlock_l(&motor.mutex, __FILE__, __LINE__);
}

bool motor_is_closed(void) {
    mcugdx_mutex_lock_l(&motor.mutex, __FILE__, __LINE__);
    bool result = motor.is_closed;
    mcugdx_mutex_unlock_l(&motor.mutex, __FILE__, __LINE__);
    return result;
}

motor_state_t motor_state(void) {
    mcugdx_mutex_lock_l(&motor.mutex, __FILE__, __LINE__);
    motor_state_t result = motor.state;
    mcugdx_mutex_unlock_l(&motor.mutex, __FILE__, __LINE__);
    return result;
}