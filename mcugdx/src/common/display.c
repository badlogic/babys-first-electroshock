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

void mcugdx_display_hline(int32_t x1, int32_t x2, int32_t y, uint16_t color) {
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

void mcugdx_display_rect(int32_t x1, int32_t y1, int32_t width, int32_t height, uint16_t color) {
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

void mcugdx_display_blit(mcugdx_image_t *src, int32_t x, int32_t y) {
    int32_t dst_x1 = x;
    int32_t dst_y1 = y;
    int32_t dst_x2 = x + src->width - 1;
    int32_t dst_y2 = y + src->height - 1;
    int32_t src_x1 = 0;
    int32_t src_y1 = 0;

    if (dst_x1 >= display.width) return;
    if (dst_x2 < 0) return;
    if (dst_y1 >= display.height) return;
    if (dst_y2 < 0) return;

    if (dst_x1 < 0) {
        src_x1 -= dst_x1;
        dst_x1 = 0;
    }
    if (dst_y1 < 0) {
        src_y1 -= dst_y1;
        dst_y1 = 0;
    }
    if (dst_x2 >= display.width) dst_x2 = display.width - 1;
    if (dst_y2 >= display.height) dst_y2 = display.height - 1;

    int32_t clipped_width = dst_x2 - dst_x1 + 1;
    int32_t clipped_height = dst_y2 - dst_y1 + 1;

    uint16_t *dst_pixel = display.frame_buffer + dst_y1 * display.width + dst_x1;
    uint16_t *src_pixel = src->pixels + src_y1 * src->width + src_x1;

    for (int32_t row = 0; row < clipped_height; row++) {
        memcpy(dst_pixel, src_pixel, clipped_width * sizeof(uint16_t));
        dst_pixel += display.width;  // Move to the next row in the display
        src_pixel += src->width;     // Move to the next row in the source image
    }
}

/*void mcugdx_display_blit(mcugdx_image_t *src, int32_t x, int32_t y) {
	int32_t dst_x1 = x;
	int32_t dst_y1 = y;
	int32_t dst_x2 = x + src->width - 1;
	int32_t dst_y2 = y + src->height - 1;
	int32_t src_x1 = 0;
	int32_t src_y1 = 0;

	if (dst_x1 >= display.width) return;
	if (dst_x2 < 0) return;
	if (dst_y1 >= display.height) return;
	if (dst_y2 < 0) return;

	if (dst_x1 < 0) {
		src_x1 -= dst_x1;
		dst_x1 = 0;
	}
	if (dst_y1 < 0) {
		src_y1 -= dst_y1;
		dst_y1 = 0;
	}
	if (dst_x2 >= display.width) dst_x2 = display.width - 1;
	if (dst_y2 >= display.height) dst_y2 = display.height - 1;

	int32_t clipped_width = dst_x2 - dst_x1 + 1;
	int32_t dst_next_row = display.width - clipped_width;
	int32_t src_next_row = src->width - clipped_width;
	uint16_t *dst_pixel = display.frame_buffer + dst_y1 * display.width + dst_x1;
	uint16_t *src_pixel = src->pixels + src_y1 * src->width + src_x1;
	for (y = dst_y1; y <= dst_y2; y++) {
		for (int32_t i = 0; i < clipped_width; i++) {
			*dst_pixel++ = *src_pixel++;
		}
		dst_pixel += dst_next_row;
		src_pixel += src_next_row;
	}
}*/

void mcugdx_display_blit_keyed(mcugdx_image_t *src, int32_t x, int32_t y, uint16_t color_key) {
	int32_t dst_x1 = x;
	int32_t dst_y1 = y;
	int32_t dst_x2 = x + src->width - 1;
	int32_t dst_y2 = y + src->height - 1;
	int32_t src_x1 = 0;
	int32_t src_y1 = 0;

	if (dst_x1 >= display.width) return;
	if (dst_x2 < 0) return;
	if (dst_y1 >= display.height) return;
	if (dst_y2 < 0) return;

	if (dst_x1 < 0) {
		src_x1 -= dst_x1;
		dst_x1 = 0;
	}
	if (dst_y1 < 0) {
		src_y1 -= dst_y1;
		dst_y1 = 0;
	}
	if (dst_x2 >= display.width) dst_x2 = display.width - 1;
	if (dst_y2 >= display.height) dst_y2 = display.height - 1;

	int32_t clipped_width = dst_x2 - dst_x1 + 1;
	int32_t dst_next_row = display.width - clipped_width;
	int32_t src_next_row = src->width - clipped_width;
	uint16_t *dst_pixel = display.frame_buffer + dst_y1 * display.width + dst_x1;
	uint16_t *src_pixel = src->pixels + src_y1 * src->width + src_x1;
	for (y = dst_y1; y <= dst_y2; y++) {
		for (int32_t i = 0; i < clipped_width; i++) {
			uint16_t src_color = *src_pixel;
			uint16_t dst_color = *dst_pixel;
			*dst_pixel = src_color != color_key ? src_color : dst_color;
			src_pixel++;
			dst_pixel++;
		}
		dst_pixel += dst_next_row;
		src_pixel += src_next_row;
	}
}

void mcugdx_display_blit_region(mcugdx_image_t *src, int32_t dst_x, int32_t dst_y, int32_t src_x, int32_t src_y, int32_t src_width, int32_t src_height) {
	int32_t dst_x1 = dst_x;
	int32_t dst_y1 = dst_y;
	int32_t dst_x2 = dst_x + src_width - 1;
	int32_t dst_y2 = dst_y + src_height - 1;
	int32_t src_x1 = src_x;
	int32_t src_y1 = src_y;

	if (dst_x1 >= display.width) return;
	if (dst_x2 < 0) return;
	if (dst_y1 >= display.height) return;
	if (dst_y2 < 0) return;

	if (dst_x1 < 0) {
		src_x1 -= dst_x1;
		dst_x1 = 0;
	}
	if (dst_y1 < 0) {
		src_y1 -= dst_y1;
		dst_y1 = 0;
	}
	if (dst_x2 >= display.width) dst_x2 = display.width - 1;
	if (dst_y2 >= display.height) dst_y2 = display.height - 1;

	int32_t clipped_width = dst_x2 - dst_x1 + 1;
	int32_t dst_next_row = display.width - clipped_width;
	int32_t src_next_row = src->width - clipped_width;
	uint16_t *dst_pixel = display.frame_buffer + dst_y1 * display.width + dst_x1;
	uint16_t *src_pixel = src->pixels + src_y1 * src->width + src_x1;
	for (int32_t y = dst_y1; y <= dst_y2; y++) {
		for (int32_t i = 0; i < clipped_width; i++) {
			*dst_pixel++ = *src_pixel++;
		}
		dst_pixel += dst_next_row;
		src_pixel += src_next_row;
	}
}

void mcugdx_display_blit_region_keyed(mcugdx_image_t *src, int32_t dst_x, int32_t dst_y, int32_t src_x, int32_t src_y, int32_t src_width, int32_t src_height, uint16_t color_key) {
	int32_t dst_x1 = dst_x;
	int32_t dst_y1 = dst_y;
	int32_t dst_x2 = dst_x + src_width - 1;
	int32_t dst_y2 = dst_y + src_height - 1;
	int32_t src_x1 = src_x;
	int32_t src_y1 = src_y;

	if (dst_x1 >= display.width) return;
	if (dst_x2 < 0) return;
	if (dst_y1 >= display.height) return;
	if (dst_y2 < 0) return;

	if (dst_x1 < 0) {
		src_x1 -= dst_x1;
		dst_x1 = 0;
	}
	if (dst_y1 < 0) {
		src_y1 -= dst_y1;
		dst_y1 = 0;
	}
	if (dst_x2 >= display.width) dst_x2 = display.width - 1;
	if (dst_y2 >= display.height) dst_y2 = display.height - 1;

	int32_t clipped_width = dst_x2 - dst_x1 + 1;
	int32_t dst_next_row = display.width - clipped_width;
	int32_t src_next_row = src->width - clipped_width;
	uint16_t *dst_pixel = display.frame_buffer + dst_y1 * display.width + dst_x1;
	uint16_t *src_pixel = src->pixels + src_y1 * src->width + src_x1;
	for (dst_y = dst_y1; dst_y <= dst_y2; dst_y++) {
		for (int32_t i = 0; i < clipped_width; i++) {
			uint16_t src_color = *src_pixel;
			uint16_t dst_color = *dst_pixel;
			*dst_pixel = src_color != color_key ? src_color : dst_color;
			src_pixel++;
			dst_pixel++;
		}
		dst_pixel += dst_next_row;
		src_pixel += src_next_row;
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
