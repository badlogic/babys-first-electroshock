#include "ultrasonic.h"

mcugdx_result_t mcugdx_ultrasonic_init(mcugdx_ultrasonic_config_t *config) {
    (void)config;
    return MCUGDX_OK;
}

mcugdx_result_t mcugdx_ultrasonic_measure(uint32_t max_distance, uint32_t *distance_cm) {
    (void)max_distance;
    (void)distance_cm;
    return MCUGDX_OK;
}
