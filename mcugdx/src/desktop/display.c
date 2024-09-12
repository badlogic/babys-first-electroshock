#include "display.h"
#include <stdlib.h>
#include <string.h>

#include "MiniFB.h"
#include <stdio.h>
#include "MiniFB_enums.h"

#define reverse_color(color) (((color) >> 8) | ((color) << 8))

typedef struct {
	int native_width;
	int native_height;
	mcugdx_display_orientation_t orientation;
	int width;
	int height;
	uint16_t *frame_buffer;
	uint32_t *frame_buffer_32;
	struct mfb_window *window;
} mcugdx_display_t;

mcugdx_display_t display;

void mcugdx_display_init(mcugdx_display_config_t *display_cfg) {
	display.native_width = display.width = display_cfg->native_width;
	display.native_height = display.height = display_cfg->native_height;
	display.orientation = MCUGDX_PORTRAIT;
	display.frame_buffer = calloc(display.width * display.height, sizeof(uint16_t));
	display.frame_buffer_32 = calloc(display.width * display.height, sizeof(uint32_t));
	display.window = NULL;

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
	if (display.window) {
		mfb_close(display.window);
		mfb_update_events(display.window);
	}
	display.window = mfb_open_ex("mcugdx", display.width * 2, display.height * 2, 0);
}

void mcugdx_display_clear(void) {
	memset(display.frame_buffer, 0, display.width * display.height * sizeof(uint16_t));
}

void mcugdx_display_clear_color(uint16_t color) {
	uint16_t *frame_buffer = display.frame_buffer;
	uint16_t reversed_color = reverse_color(color);
	for (int i = 0, n = display.width * display.height; i < n; i++) {
		frame_buffer[i] = reversed_color;
	}
}

void mcugdx_display_set_pixel(int x, int y, uint16_t color) {
	if (x < 0 || x >= display.width) return;
	if (y < 0 || y >= display.height) return;

	uint16_t reversed_color = color;
	display.frame_buffer[x + display.width * y] = reversed_color;
}

void mcugdx_display_hline(int32_t x1, int32_t x2, int32_t y, uint32_t color) {
	if (x1 > x2) {
		int32_t tmp = x2;
		x2 = x1;
		x1 = tmp;
	}

	if (x1 >= display.width) return;
	if (x2 < 0) return;
	if (y < 0) return;
	if (y >= display.height) return;

	if (x1 < 0) x1 = 0;
	if (x2 >= display.width) x2 = display.width - 1;

	uint16_t reversed_color = reverse_color(color);
	uint16_t *pixels = display.frame_buffer + y * display.width + x1;
	int32_t num_pixels = x2 - x1 + 1;
	while (num_pixels--) {
		*pixels++ = reversed_color;
	}
}

void mcugdx_display_rect(int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t color) {
	if (width <= 0) return;
	if (height <= 0) return;

	int32_t x2 = x1 + width - 1;
	int32_t y2 = y1 + height - 1;

	if (x1 >= display.width) return;
	if (x2 < 0) return;
	if (y1 >= display.height) return;
	if (y2 < 0) return;

	if (x1 < 0) x1 = 0;
	if (y1 < 0) y1 = 0;
	if (x2 >= display.width) x2 = display.width - 1;
	if (y2 >= display.height) y2 = display.height - 1;

	uint16_t reversed_color = reverse_color(color);
	int32_t clipped_width = x2 - x1 + 1;
	int32_t next_row = display.width - clipped_width;
	uint16_t *pixel = display.frame_buffer + y1 * display.width + x1;
	for (int32_t y = y1; y <= y2; y++) {
		for (int32_t i = 0; i < clipped_width; i++) {
			*pixel++ = reversed_color;
		}
		pixel += next_row;
	}
}

void mcugdx_display_show(void) {
	uint32_t *dst = display.frame_buffer_32;
	uint16_t *src = display.frame_buffer;
	int num_pixels = display.width * display.height;

	for (int i = 0; i < num_pixels; i++) {
		uint16_t pixel = reverse_color(src[i]);

		// Extract 5-6-5 RGB components from 16-bit pixel
		uint8_t r = (pixel >> 11) & 0x1F;// 5-bit red
		uint8_t g = (pixel >> 5) & 0x3F; // 6-bit green
		uint8_t b = pixel & 0x1F;        // 5-bit blue

		// Expand to 8-bit color
		uint8_t r8 = (r << 3) | (r >> 2);// Scale 5-bit to 8-bit
		uint8_t g8 = (g << 2) | (g >> 4);// Scale 6-bit to 8-bit
		uint8_t b8 = (b << 3) | (b >> 2);// Scale 5-bit to 8-bit

		// Set 32-bit pixel with alpha = 0xFF
		dst[i] = (0xFF << 24) | (r8 << 16) | (g8 << 8) | b8;
	}

	mfb_update_ex(display.window, display.frame_buffer_32, display.width, display.height);
	if (!mfb_wait_sync(display.window)) {
		exit(0);
	}
}

int mcugdx_display_width(void) {
	return display.width;
}

int mcugdx_display_height(void) {
	return display.height;
}

uint16_t *mcugdx_display_frame_buffer(void) {
	return display.frame_buffer;
}
