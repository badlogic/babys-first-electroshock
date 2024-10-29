#include "mcugdx.h"
#include "stdio.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "MCUGDX"

void mcugdx_init() {
}

void mcugdx_quit() {
	esp_restart();
}

double mcugdx_time() {
	return (double) esp_timer_get_time() / 1000000;
}

void mcugdx_sleep(uint32_t millies) {
	vTaskDelay(pdMS_TO_TICKS(millies));
}
