#include "buttons.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "log.h"
#include "esp_timer.h"

#define TAG "mcugdx_button"
#define ESP_INTR_FLAG_DEFAULT 0
#define MAX_BUTTONS 32
#define INVALID_HANDLE 0

typedef struct {
	int pin;
	uint32_t debounce_time_ms;
	uint16_t keycode;
	int64_t last_change_time;
	bool last_state;
} mcugdx_button_t;

static mcugdx_button_t buttons[MAX_BUTTONS] = {0};
static QueueHandle_t event_queue = NULL;
static bool isr_installed = false;

static void IRAM_ATTR gpio_isr_handler(void *arg) {
	uint32_t gpio_num = (uint32_t) arg;
	int64_t now = esp_timer_get_time() / 1000;

	for (int i = 0; i < MAX_BUTTONS; i++) {
		if (buttons[i].pin == gpio_num) {
			if ((now - buttons[i].last_change_time) > buttons[i].debounce_time_ms) {
				bool current_state = !gpio_get_level(gpio_num);
				if (current_state != buttons[i].last_state) {
					mcugdx_button_event_t event = {
							.button = i + 1,
							.type = current_state ? MCUGDX_BUTTON_PRESSED : MCUGDX_BUTTON_RELEASED,
							.timestamp = now,
                            .pin = buttons[i].pin,
                            .keycode = buttons[i].keycode};
					xQueueSendFromISR(event_queue, &event, NULL);
					buttons[i].last_state = current_state;
					buttons[i].last_change_time = now;
				}
			}
			break;
		}
	}
}

static void mcugdx_button_init(void) {
	if (event_queue == NULL) {
		for (int i = 0; i < MAX_BUTTONS; i++) {
			buttons[i].pin = -1;
		}
		event_queue = xQueueCreate(32, sizeof(mcugdx_button_event_t));
	}
}

mcugdx_button_handle_t mcugdx_button_create(int pin, uint32_t debounce_time_ms, mcugdx_keycode_t keycode) {
	mcugdx_button_init();
	for (int i = 0; i < MAX_BUTTONS; i++) {
		if (buttons[i].pin == -1) {
			buttons[i].pin = pin;
			buttons[i].debounce_time_ms = debounce_time_ms;
			buttons[i].keycode = keycode;
			buttons[i].last_change_time = 0;
			buttons[i].last_state = false;

			gpio_config_t io_conf = {
					.pin_bit_mask = (1ULL << pin),
					.mode = GPIO_MODE_INPUT,
					.pull_up_en = GPIO_PULLUP_ENABLE,
					.pull_down_en = GPIO_PULLDOWN_DISABLE,
					.intr_type = GPIO_INTR_ANYEDGE};
			gpio_config(&io_conf);
			if (!isr_installed) {
                gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
                isr_installed = true;
            }
			gpio_isr_handler_add(pin, gpio_isr_handler, (void *) pin);

			return i + 1;
		}
	}
	return INVALID_HANDLE;
}

void mcugdx_button_destroy(mcugdx_button_handle_t handle) {
	if (handle > 0 && handle <= MAX_BUTTONS) {
		int pin = buttons[handle - 1].pin;
		gpio_isr_handler_remove(pin);
		buttons[handle - 1].pin = -1;
	}
}

bool mcugdx_button_is_pressed(mcugdx_button_handle_t handle) {
	if (handle > 0 && handle <= MAX_BUTTONS) {
		return !gpio_get_level(buttons[handle - 1].pin);
	}
	return false;
}

bool mcugdx_button_get_event(mcugdx_button_event_t *event) {
	if (event_queue == NULL) {
		mcugdx_loge(TAG, "Event queue is null");
		return false;
	}
	return xQueueReceive(event_queue, event, 0) == pdTRUE;
}

void mcugdx_button_clear_events(void) {
	if (event_queue != NULL) {
		xQueueReset(event_queue);
	}
}