#include "display.h"
#include <stdlib.h>
#include <string.h>

#include "MiniFB.h"
#include <stdio.h>
#include "MiniFB_enums.h"

#define reverse_color(color) (((color) >> 8) | ((color) << 8))

uint32_t *frame_buffer_32;
struct mfb_window *window;
mcugdx_display_t display;

void mcugdx_display_init(mcugdx_display_config_t *display_cfg) {
	display.native_width = display.width = display_cfg->native_width;
	display.native_height = display.height = display_cfg->native_height;
	display.orientation = MCUGDX_PORTRAIT;
	display.frame_buffer = calloc(display.width * display.height, sizeof(uint16_t));
	frame_buffer_32 = calloc(display.width * display.height, sizeof(uint32_t));
	window = NULL;

	mcugdx_display_set_orientation(MCUGDX_PORTRAIT);
}

void mcugdx_display_set_orientation(mcugdx_display_orientation_t orientation) {
	if (orientation == MCUGDX_LANDSCAPE) {
		display.width = display.native_height;
		display.height = display.native_width;
	} else {
		display.width = display.native_width;
		display.height = display.native_height;
	}
	if (window) {
		mfb_close(window);
		mfb_update_events(window);
	}
	window = mfb_open_ex("mcugdx", display.width * 2, display.height * 2, 0);
}

void mcugdx_display_show(void) {
	uint32_t *dst = frame_buffer_32;
	uint16_t *src = display.frame_buffer;
	int num_pixels = display.width * display.height;

	for (int i = 0; i < num_pixels; i++) {
		uint16_t pixel = reverse_color(src[i]);

		uint8_t r = (pixel >> 11) & 0x1F;
		uint8_t g = (pixel >> 5) & 0x3F;
		uint8_t b = pixel & 0x1F;

		uint8_t r8 = (r << 3) | (r >> 2);
		uint8_t g8 = (g << 2) | (g >> 4);
		uint8_t b8 = (b << 3) | (b >> 2);

		dst[i] = (0xFF << 24) | (r8 << 16) | (g8 << 8) | b8;
	}

	mfb_update_ex(window, frame_buffer_32, display.width, display.height);
	if (!mfb_wait_sync(window)) {
		exit(0);
	}
}
