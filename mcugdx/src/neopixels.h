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

void mcugdx_neopixels_get(uint32_t index, uint8_t *r, uint8_t *g, uint8_t *b);

uint32_t mcugdx_neopixels_power_usage_milli_ampere(void);

void mcugdx_neopixels_show(void);

void mcugdx_neopixels_show_max_milli_ampere(uint32_t max_milli_ampere);

#ifdef __cplusplus
}
#endif
