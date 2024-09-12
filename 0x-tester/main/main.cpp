#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/adc.h"
extern "C"
{
#include "esp_ws28xx.h"
}

constexpr adc1_channel_t R_CHAN = ADC1_CHANNEL_3;
constexpr adc1_channel_t G_CHAN = ADC1_CHANNEL_4;
constexpr adc1_channel_t B_CHAN = ADC1_CHANNEL_5;

constexpr int LED_GPIO = 3;
constexpr int LED_NUM = 10;
CRGB *leds;

extern "C" void app_main(void)
{
    printf("Setting up LEDs\n");
    ESP_ERROR_CHECK(ws28xx_init(LED_GPIO, WS2812B, LED_NUM, &leds));
    for (int i = 0; i < LED_NUM; i++)
    {
        leds[i].r = leds[i].g = leds[i].b = 0;
    }
    ESP_ERROR_CHECK(ws28xx_update());

    printf("Setting up ADCs\n");
    adc1_config_width(ADC_WIDTH_BIT_12);

    while (true)
    {
        auto r = adc1_get_raw(R_CHAN);
        auto g = adc1_get_raw(G_CHAN);
        auto b = adc1_get_raw(B_CHAN);

        r = (r * 255) / 4095;
        g = (g * 255) / 4095;
        b = (b * 255) / 4095;

        for (int i = 0; i < LED_NUM; i++)
        {
            leds[i].r = (uint8_t)r;
            leds[i].g = (uint8_t)g;
            leds[i].b = (uint8_t)b;
        }
        ESP_ERROR_CHECK(ws28xx_update());

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}
