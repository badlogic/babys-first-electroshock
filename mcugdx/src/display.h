#pragma once

#include <stdint.h>
#include "result.h"
#include "image.h"

#ifdef __cplusplus
extern "C" {
#endif

#define swap_bytes(color) __builtin_bswap16(color)
#define MCUGDX_RED 0b1111100000000000
#define MCUGDX_GREEN 0b11111100000
#define MCUGDX_BLUE 0b11111
#define MCUGDX_BLACK 0x0
#define MCUGDX_WHITE 0xffff
#define MCUGDX_PINK 0b1111100000011111

typedef enum {
	MCUGDX_ST7789,
	MCUGDX_ST7796,
	MCUGDX_ILI9341
} mcugdx_display_driver_t;

typedef enum {
	MCUGDX_PORTRAIT,
	MCUGDX_LANDSCAPE
} mcugdx_display_orientation_t;

typedef struct {
	mcugdx_display_driver_t driver;
	uint32_t native_width;
	uint32_t native_height;
	int mosi;
	int sck;
	int dc;
	int cs;
	int reset;
} mcugdx_display_config_t;

typedef struct {
	uint32_t native_width;
	uint32_t native_height;
	mcugdx_display_orientation_t orientation;
	uint32_t width;
	uint32_t height;
	uint16_t *frame_buffer;
} mcugdx_display_t;

mcugdx_result_t mcugdx_display_init(mcugdx_display_config_t *display_cfg);

void mcugdx_display_set_orientation(mcugdx_display_orientation_t orientation);

void mcugdx_display_clear(void);

void mcugdx_display_clear_color(uint16_t color);

void mcugdx_display_set_pixel(int32_t x, int32_t y, uint16_t color);

void mcugdx_display_hline(int32_t x1, int32_t x2, int32_t y, uint16_t color);

void mcugdx_display_rect(int32_t x1, int32_t y1, int32_t width, int32_t height, uint16_t color);

void mcugdx_display_blit(mcugdx_image_t *src, int32_t x, int32_t y);

void mcugdx_display_blit_keyed(mcugdx_image_t *src, int32_t x, int32_t y, uint16_t color_key);

void mcugdx_display_blit_region(mcugdx_image_t *src, int32_t dst_x, int32_t dst_y, int32_t src_x, int32_t src_y, int32_t src_width, int32_t src_height);

void mcugdx_display_blit_region_keyed(mcugdx_image_t *src, int32_t dst_x, int32_t dst_y, int32_t src_x, int32_t src_y, int32_t src_width, int32_t src_height, uint16_t color_key);

void mcugdx_display_show(void);

int mcugdx_display_width(void);

int mcugdx_display_height(void);

uint16_t *mcugdx_display_frame_buffer(void);

#ifdef __cplusplus
}
#endif
