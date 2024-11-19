#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MCUGDX_DIGITAL_OUTPUT,
    MCUGDX_DIGITAL_INPUT,
    MCUGDX_ANALOG_OUTPUT,
    MCUGDX_ANALOG_INPUT
} mcugdx_gpio_pin_mode_t;

typedef enum {
    MCUGDX_PULL_NONE,
    MCUGDX_PULL_UP,
    MCUGDX_PULL_DOWN
} mcugdx_gpio_pull_t;

void mcugdx_gpio_pin_mode(int pin, mcugdx_gpio_pin_mode_t mode, mcugdx_gpio_pull_t pull);
void mcugdx_gpio_digital_out(int pin, int value);
int mcugdx_gpio_digital_in(int pin);
void mcugdx_gpio_analog_out(int pin, int value);
int mcugdx_gpio_analog_in(int pin);

#pragma once

#ifdef __cplusplus
}
#endif
