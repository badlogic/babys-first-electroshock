#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "mutex.h"

bool bme280_init(int sclk, int sda);
bool bme280_update();
float bme280_temperature();
float bme280_pressure();
float bme280_humidity();

#ifdef __cplusplus
}
#endif
