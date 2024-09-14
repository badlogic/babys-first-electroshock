#include "mcugdx.h"
#include "stdio.h"
#include "esp_timer.h"

#define TAG "MCUGDX"

void mcugdx_init() {
}

double mcugdx_time() {
	return (double) esp_timer_get_time() / 1000000;
}
