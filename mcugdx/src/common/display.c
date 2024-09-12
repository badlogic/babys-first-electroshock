#include "display.h"
#include <string.h>

#define reverse_color(color) (((color) >> 8) | ((color) << 8))

extern mcugdx_display_t display;

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

int mcugdx_display_width(void) {
	return display.width;
}

int mcugdx_display_height(void) {
	return display.height;
}

uint16_t *mcugdx_display_frame_buffer(void) {
	return display.frame_buffer;
}
