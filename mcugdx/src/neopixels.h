#pragma once

#include "result.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t num_leds;
    int pin;
} mcugdx_neopixels_config_t;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} mcugdx_neopixel_t;

mcugdx_result_t mcugdx_neopixels_init(mcugdx_neopixels_config_t *config);

void mcugdx_neopixels_set(uint32_t index, uint8_t r, uint8_t g, uint8_t b);

void mcugdx_neopixels_show();

#ifdef __cplusplus
}
#endif