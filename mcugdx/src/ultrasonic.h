#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int trigger;
    int echo;
    double interval;
} mcugdx_ultrasonic_config_t;

bool mcugdx_ultrasonic_init(mcugdx_ultrasonic_config_t *config);

bool mcugdx_ultrasonic_measure(uint32_t max_distance, uint32_t *distance_cm);

#ifdef __cplusplus
}
#endif
