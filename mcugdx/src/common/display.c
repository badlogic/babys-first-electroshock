#include "display.h"
#include <string.h>

extern mcugdx_display_t display;

void mcugdx_display_clear(void) {
	memset(display.frame_buffer, 0, display.width * display.height * sizeof(uint16_t));
}

void mcugdx_display_clear_color(uint16_t color) {
	color = swap_bytes(color);
	uint32_t *frame_buffer32 = (uint32_t *) display.frame_buffer;
	uint32_t color32 = ((uint32_t) color << 16) | color;
	size_t count32 = (display.width * display.height) / 2;
	size_t remainder = (display.width * display.height) % 2;

	for (size_t i = 0; i < count32; i++) {
		frame_buffer32[i] = color32;
	}

	if (remainder) {
		display.frame_buffer[display.width * display.height - 1] = color;
	}
}

void mcugdx_display_set_pixel(int32_t x, int32_t y, uint16_t color) {
	if (x < 0 || x >= (int32_t) display.width) return;
	if (y < 0 || y >= (int32_t) display.height) return;

	uint16_t reversed_color = color;
	display.frame_buffer[x + display.width * y] = reversed_color;
}

void mcugdx_display_hline(int32_t x1, int32_t x2, int32_t y, uint16_t color) {
	if (x1 > x2) {
		int32_t tmp = x2;
		x2 = x1;
		x1 = tmp;
	}

	if (x1 >= (int32_t) display.width) return;
	if (x2 < 0) return;
	if (y < 0) return;
	if (y >= (int32_t) display.height) return;

	if (x1 < 0) x1 = 0;
	if (x2 >= (int32_t) display.width) x2 = (int32_t) display.width - 1;

	uint16_t reversed_color = swap_bytes(color);
	uint16_t *pixels = display.frame_buffer + y * (int32_t) display.width + x1;
	int32_t num_pixels = x2 - x1 + 1;
	while (num_pixels--) {
		*pixels++ = reversed_color;
	}
}

void mcugdx_display_rect(int32_t x1, int32_t y1, int32_t width, int32_t height, uint16_t color) {
	if (width <= 0 || height <= 0) return;

	color = swap_bytes(color);

	int32_t x2 = x1 + width - 1;
	int32_t y2 = y1 + height - 1;

	x1 = (x1 < 0) ? 0 : (x1 >= (int32_t) display.width ? (int32_t) display.width - 1 : x1);
	y1 = (y1 < 0) ? 0 : (y1 >= (int32_t) display.height ? (int32_t) display.height - 1 : y1);
	x2 = (x2 < 0) ? 0 : (x2 >= (int32_t) display.width ? (int32_t) display.width - 1 : x2);
	y2 = (y2 < 0) ? 0 : (y2 >= (int32_t) display.height ? (int32_t) display.height - 1 : y2);

	if (x1 > x2 || y1 > y2) return;

	int32_t clipped_width = x2 - x1 + 1;
	int32_t next_row = display.width - clipped_width;

	uint32_t color32 = ((uint32_t) color << 16) | color;
	uint32_t *pixel32 = (uint32_t *) (display.frame_buffer + y1 * display.width + x1);

	for (int32_t y = y1; y <= y2; y++) {
		int32_t x = 0;
		for (; x < clipped_width - 1; x += 2) {
			*pixel32++ = color32;
		}
		if (x < clipped_width) {
			*(uint16_t *) pixel32 = color;
			pixel32 = (uint32_t *) ((uint16_t *) pixel32 + 1);
		}
		pixel32 = (uint32_t *) ((uint16_t *) pixel32 + next_row);
	}
}

void mcugdx_display_blit(mcugdx_image_t *src, int32_t x, int32_t y) {
	int32_t dst_x1 = x;
	int32_t dst_y1 = y;
	int32_t dst_x2 = x + src->width - 1;
	int32_t dst_y2 = y + src->height - 1;
	int32_t src_x1 = 0;
	int32_t src_y1 = 0;

	if (dst_x1 >= (int32_t) display.width) return;
	if (dst_x2 < 0) return;
	if (dst_y1 >= (int32_t) display.height) return;
	if (dst_y2 < 0) return;

	if (dst_x1 < 0) {
		src_x1 -= dst_x1;
		dst_x1 = 0;
	}
	if (dst_y1 < 0) {
		src_y1 -= dst_y1;
		dst_y1 = 0;
	}
	if (dst_x2 >= (int32_t) display.width) dst_x2 = (int32_t) display.width - 1;
	if (dst_y2 >= (int32_t) display.height) dst_y2 = (int32_t) display.height - 1;

	int32_t clipped_width = dst_x2 - dst_x1 + 1;
	int32_t clipped_height = dst_y2 - dst_y1 + 1;

	uint16_t *dst_pixel = display.frame_buffer + dst_y1 * display.width + dst_x1;
	uint16_t *src_pixel = src->pixels + src_y1 * src->width + src_x1;

	for (int32_t row = 0; row < clipped_height; row++) {
		memcpy(dst_pixel, src_pixel, clipped_width * sizeof(uint16_t));
		dst_pixel += display.width;
		src_pixel += src->width;
	}
}

void mcugdx_display_blit_keyed(mcugdx_image_t *src, int32_t x, int32_t y, uint16_t color_key) {
	int32_t dst_x1 = (x < 0) ? 0 : x;
	int32_t dst_y1 = (y < 0) ? 0 : y;
	int32_t dst_x2 = (x + src->width > display.width) ? display.width - 1 : x + src->width - 1;
	int32_t dst_y2 = (y + src->height > display.height) ? display.height - 1 : y + src->height - 1;

	if (dst_x1 >= (int32_t) display.width || dst_x2 < 0 || dst_y1 >= (int32_t) display.height || dst_y2 < 0) return;

	int32_t src_x1 = (x < 0) ? -x : 0;
	int32_t src_y1 = (y < 0) ? -y : 0;
	int32_t clipped_width = dst_x2 - dst_x1 + 1;
	int32_t dst_next_row = display.width - clipped_width;
	int32_t src_next_row = src->width - clipped_width;

	uint32_t *dst_pixel32 = (uint32_t *) (display.frame_buffer + dst_y1 * display.width + dst_x1);
	uint32_t *src_pixel32 = (uint32_t *) (src->pixels + src_y1 * src->width + src_x1);

	for (int32_t y = dst_y1; y <= dst_y2; y++) {
		int32_t x = 0;
		for (; x < clipped_width - 1; x += 2) {
			uint32_t src_colors = *src_pixel32++;
			uint32_t dst_colors = *dst_pixel32;

			if ((src_colors & 0xFFFF) != color_key) {
				dst_colors = (dst_colors & 0xFFFF0000) | (src_colors & 0xFFFF);
			}
			if ((src_colors >> 16) != color_key) {
				dst_colors = (dst_colors & 0x0000FFFF) | (src_colors & 0xFFFF0000);
			}

			*dst_pixel32++ = dst_colors;
		}
		if (x < clipped_width) {
			uint16_t src_color = ((uint16_t *) src_pixel32)[0];
			if (src_color != color_key) {
				((uint16_t *) dst_pixel32)[0] = src_color;
			}
			dst_pixel32 = (uint32_t *) ((uint16_t *) dst_pixel32 + 1);
			src_pixel32 = (uint32_t *) ((uint16_t *) src_pixel32 + 1);
		}
		dst_pixel32 = (uint32_t *) ((uint16_t *) dst_pixel32 + dst_next_row);
		src_pixel32 = (uint32_t *) ((uint16_t *) src_pixel32 + src_next_row);
	}
}

void mcugdx_display_blit_region(mcugdx_image_t *src, int32_t dst_x, int32_t dst_y, int32_t src_x, int32_t src_y, int32_t src_width, int32_t src_height) {
	if (src_width <= 0 || src_height <= 0) return;

	int32_t dst_x2 = dst_x + src_width - 1;
	int32_t dst_y2 = dst_y + src_height - 1;

	int32_t clip_left = (dst_x < 0) ? -dst_x : 0;
	int32_t clip_top = (dst_y < 0) ? -dst_y : 0;
	int32_t clip_right = (dst_x2 >= (int32_t) display.width) ? (int32_t) display.width - dst_x - 1 : src_width - 1;
	int32_t clip_bottom = (dst_y2 >= (int32_t) display.height) ? (int32_t) display.height - dst_y - 1 : src_height - 1;

	if (clip_left > clip_right || clip_top > clip_bottom) return;

	int32_t clipped_width = clip_right - clip_left + 1;
	int32_t clipped_height = clip_bottom - clip_top + 1;

	src_x += clip_left;
	src_y += clip_top;
	dst_x += clip_left;
	dst_y += clip_top;

	uint16_t *dst_base = display.frame_buffer + dst_y * display.width + dst_x;
	uint16_t *src_base = src->pixels + src_y * src->width + src_x;

	if (clipped_width > 16) {
		for (int32_t y = 0; y < clipped_height; y++) {
			memcpy(dst_base, src_base, clipped_width * sizeof(uint16_t));
			dst_base += display.width;
			src_base += src->width;
		}
	} else {
		uint32_t *dst_word = (uint32_t *) dst_base;
		uint32_t *src_word = (uint32_t *) src_base;
		int32_t dst_stride = display.width / 2;
		int32_t src_stride = src->width / 2;
		int32_t word_width = clipped_width / 2;
		int32_t remainder = clipped_width % 2;

		for (int32_t y = 0; y < clipped_height; y++) {
			for (int32_t x = 0; x < word_width; x++) {
				dst_word[x] = src_word[x];
			}
			if (remainder) {
				((uint16_t *) (&dst_word[word_width]))[0] = ((uint16_t *) (&src_word[word_width]))[0];
			}
			dst_word += dst_stride;
			src_word += src_stride;
		}
	}
}

void mcugdx_display_blit_region_keyed(mcugdx_image_t *src, int32_t dst_x, int32_t dst_y, int32_t src_x, int32_t src_y, int32_t src_width, int32_t src_height, uint16_t color_key) {
	if (src_width <= 0 || src_height <= 0) return;

	int32_t dst_x2 = dst_x + src_width - 1;
	int32_t dst_y2 = dst_y + src_height - 1;

	int32_t clip_left = (dst_x < 0) ? -dst_x : 0;
	int32_t clip_top = (dst_y < 0) ? -dst_y : 0;
	int32_t clip_right = (dst_x2 >= (int32_t) display.width) ? (int32_t) display.width - dst_x - 1 : src_width - 1;
	int32_t clip_bottom = (dst_y2 >= (int32_t) display.height) ? (int32_t) display.height - dst_y - 1 : src_height - 1;

	if (clip_left > clip_right || clip_top > clip_bottom) return;

	int32_t clipped_width = clip_right - clip_left + 1;
	int32_t clipped_height = clip_bottom - clip_top + 1;

	src_x += clip_left;
	src_y += clip_top;
	dst_x += clip_left;
	dst_y += clip_top;

	uint32_t *dst_pixel32 = (uint32_t *) (display.frame_buffer + dst_y * display.width + dst_x);
	uint32_t *src_pixel32 = (uint32_t *) (src->pixels + src_y * src->width + src_x);

	int32_t dst_stride = display.width / 2;
	int32_t src_stride = src->width / 2;

	for (int32_t y = 0; y < clipped_height; y++) {
		int32_t x = 0;
		for (; x < clipped_width - 1; x += 2) {
			uint32_t src_colors = *src_pixel32++;
			uint32_t dst_colors = *dst_pixel32;

			if ((src_colors & 0xFFFF) != color_key) {
				dst_colors = (dst_colors & 0xFFFF0000) | (src_colors & 0xFFFF);
			}
			if ((src_colors >> 16) != color_key) {
				dst_colors = (dst_colors & 0x0000FFFF) | (src_colors & 0xFFFF0000);
			}

			*dst_pixel32++ = dst_colors;
		}
		if (x < clipped_width) {
			uint16_t src_color = ((uint16_t *) src_pixel32)[0];
			if (src_color != color_key) {
				((uint16_t *) dst_pixel32)[0] = src_color;
			}
			dst_pixel32 = (uint32_t *) ((uint16_t *) dst_pixel32 + 1);
			src_pixel32 = (uint32_t *) ((uint16_t *) src_pixel32 + 1);
		}
		dst_pixel32 += dst_stride - (clipped_width + 1) / 2;
		src_pixel32 += src_stride - (clipped_width + 1) / 2;
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
