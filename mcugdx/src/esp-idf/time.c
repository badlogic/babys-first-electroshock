#include "time.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "MCUGDX"

double mcugdx_time() {
	return (double) esp_timer_get_time() / 1000000;
}

void mcugdx_sleep(uint32_t millies) {
	vTaskDelay(pdMS_TO_TICKS(millies));
}