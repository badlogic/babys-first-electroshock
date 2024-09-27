#include "buttons.h"
#include <MiniFB.h>
#include <MiniFB_enums.h>
#include <string.h>

#define MAX_BUTTONS 32
#define MAX_EVENTS 32

typedef struct {
	int pin;
	uint32_t debounce_time_ms;
	mcugdx_keycode_t keycode;
	bool is_pressed;
	uint64_t last_event_time;
} mcugdx_button_t;

static mcugdx_button_t buttons[MAX_BUTTONS] = {0};
static mcugdx_button_event_t event_queue[MAX_EVENTS] = {0};
static int event_queue_head = 0;
static int event_queue_tail = 0;
static bool is_initialized = false;
static struct mfb_timer *timer;

static void enqueue_event(mcugdx_button_event_t *event) {
	if ((event_queue_tail + 1) % MAX_EVENTS == event_queue_head) {
		// Queue is full, drop the oldest event
		event_queue_head = (event_queue_head + 1) % MAX_EVENTS;
	}
	memcpy(&event_queue[event_queue_tail], event, sizeof(mcugdx_button_event_t));
	event_queue_tail = (event_queue_tail + 1) % MAX_EVENTS;
}

mcugdx_button_handle_t mcugdx_button_create(int pin, uint32_t debounce_time_ms, uint16_t keycode) {
	if (!is_initialized) {
		for (int i = 0; i < MAX_BUTTONS; i++) {
			buttons[i].pin = -1;
		}
		timer = mfb_timer_create();
		is_initialized = true;
	}

	for (int i = 0; i < MAX_BUTTONS; i++) {
		if (buttons[i].pin == -1) {
			buttons[i].pin = pin;
			buttons[i].debounce_time_ms = debounce_time_ms;
			buttons[i].keycode = keycode;
			buttons[i].is_pressed = false;
			buttons[i].last_event_time = 0;
			return i;
		}
	}
	return 0;
}

void mcugdx_button_destroy(mcugdx_button_handle_t handle) {
	if (handle >= 0 && handle < MAX_BUTTONS) {
		buttons[handle].pin = -1;
	}
}

bool mcugdx_button_is_pressed(mcugdx_button_handle_t handle) {
	if (handle >= 0 && handle < MAX_BUTTONS && buttons[handle].pin != -1) {
		return buttons[handle].is_pressed;
	}
	return false;
}

bool mcugdx_button_get_event(mcugdx_button_event_t *event) {
	if (event_queue_head != event_queue_tail) {
		memcpy(event, &event_queue[event_queue_head], sizeof(mcugdx_button_event_t));
		event_queue_head = (event_queue_head + 1) % MAX_EVENTS;
		return true;
	}
	return false;
}

void mcugdx_button_clear_events(void) {
	event_queue_head = event_queue_tail = 0;
}

void mcugdx_desktop_update_button(struct mfb_window *window, mfb_key key_code, mfb_key_mod mod, bool is_pressed) {
    (void)window;
    (void)mod;
	uint16_t current_time = mfb_timer_now(timer) * 1000;
	for (int i = 0; i < MAX_BUTTONS; i++) {
		if (buttons[i].pin != -1 && buttons[i].keycode == (mcugdx_keycode_t)key_code) {
			uint64_t time_diff = current_time - buttons[i].last_event_time;
			if (time_diff >= buttons[i].debounce_time_ms) {
                if (buttons[i].is_pressed == is_pressed) return;
				mcugdx_button_event_t button_event = {
						.button = i,
						.type = is_pressed ? MCUGDX_BUTTON_PRESSED : MCUGDX_BUTTON_RELEASED,
						.timestamp = current_time,
						.pin = buttons[i].pin,
						.keycode = buttons[i].keycode};
				enqueue_event(&button_event);
				buttons[i].is_pressed = is_pressed;
				buttons[i].last_event_time = current_time;
			}
			break;
		}
	}
}
