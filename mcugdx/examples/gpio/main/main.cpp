#include "mcugdx.h"

#define TAG "Screaming cauldron"
#define LED_PIN 14
#define BLINK_TIME 5    // 5 seconds
#define RAMP_TIME 2     // 2 seconds

extern "C" void app_main() {
   mcugdx_init();

   while (true) {
       mcugdx_gpio_pin_mode(LED_PIN, MCUGDX_DIGITAL_OUTPUT, MCUGDX_PULL_NONE);
       double start_time = mcugdx_time();
       int value = 1;

       while (mcugdx_time() - start_time < BLINK_TIME) {
           mcugdx_gpio_digital_out(LED_PIN, value);
           value = !value;
           mcugdx_sleep(100);
           mcugdx_log(TAG, "Blinky at %f", mcugdx_time());
       }

       mcugdx_gpio_pin_mode(LED_PIN, MCUGDX_ANALOG_OUTPUT, MCUGDX_PULL_NONE);

       start_time = mcugdx_time();
       while (mcugdx_time() - start_time < RAMP_TIME) {
           float progress = (mcugdx_time() - start_time) / (float)RAMP_TIME;
           int duty = (int)(progress * 255);
           mcugdx_gpio_analog_out(LED_PIN, duty);
           mcugdx_sleep(20);
           mcugdx_log(TAG, "Ramping up: %d", duty);
       }

       start_time = mcugdx_time();
       while (mcugdx_time() - start_time < RAMP_TIME) {
           float progress = (mcugdx_time() - start_time) / (float)RAMP_TIME;
           int duty = (int)((1.0 - progress) * 255);
           mcugdx_gpio_analog_out(LED_PIN, duty);
           mcugdx_sleep(20);
           mcugdx_log(TAG, "Ramping down: %d", duty);
       }
   }
}