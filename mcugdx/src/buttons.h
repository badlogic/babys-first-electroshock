#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "keycodes.h"

typedef uint8_t mcugdx_button_handle_t;

typedef enum {
	MCUGDX_BUTTON_PRESSED,
	MCUGDX_BUTTON_RELEASED
} mcugdx_button_event_type_t;

typedef struct {
	mcugdx_button_handle_t button;
	mcugdx_button_event_type_t type;
	int64_t timestamp;
	int pin;
	mcugdx_keycode_t keycode;
} mcugdx_button_event_t;

mcugdx_button_handle_t mcugdx_button_create(int pin, uint32_t debounce_time_ms, uint16_t keycode);
void mcugdx_button_destroy(mcugdx_button_handle_t handle);
bool mcugdx_button_is_pressed(mcugdx_button_handle_t handle);
bool mcugdx_button_get_event(mcugdx_button_event_t *event);
void mcugdx_button_clear_events(void);

#ifdef __cplusplus
}
#endif
