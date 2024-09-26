/*#include "mcugdx.h"
#include <stdio.h>

#define TAG "Buttons example"

extern "C" void app_main() {
	mcugdx_log(TAG, "Stuff");
}
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define BUTTON_PIN 1
#define DEBOUNCE_TIME 50 // milliseconds

static const char *TAG = "BUTTON_EXAMPLE";

extern "C" void app_main(void)
{
    // Configure button GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    ESP_LOGI(TAG, "Button configured on pin %d with internal pull-up", BUTTON_PIN);

    int last_state = 1; // Assuming button not pressed initially (pull-up active)
    while (1) {
        int current_state = gpio_get_level((gpio_num_t)BUTTON_PIN);

        if (current_state == 0 && last_state == 1) {
            ESP_LOGI(TAG, "Button pressed!");
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_TIME)); // Simple debounce
        }

        last_state = current_state;
        vTaskDelay(pdMS_TO_TICKS(10)); // Poll every 10ms
    }
}