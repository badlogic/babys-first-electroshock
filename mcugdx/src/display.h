#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MCUGDX_ST7789,
    MCUGDX_ILI9341
} mcugdx_display_driver_t;

typedef enum {
    MCUGDX_PORTRAIT,
    MCUGDX_LANDSCAPE
} mcugdx_display_orientation_t;

typedef struct {
    mcugdx_display_driver_t driver;
    int native_width;
    int native_height;
    int mosi;
    int sck;
    int dc;
    int cs;
} mcugdx_display_config_t;

void mcugdx_display_init(mcugdx_display_config_t *display_cfg);

void mcugdx_display_set_orientation(mcugdx_display_orientation_t orientation);

void mcugdx_display_clear(void);

void mcugdx_display_clear_color(uint16_t color);

void mcugdx_display_set_pixel(int x, int y, uint16_t color);

void mcugdx_display_hline(int32_t x1, int32_t x2, int32_t y, uint32_t color);

void mcugdx_display_rect(int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t color);

void mcugdx_display_show(void);

int mcugdx_display_width(void);

int mcugdx_display_height(void);

uint16_t *mcugdx_display_frame_buffer(void);

#ifdef __cplusplus
}
#endif
