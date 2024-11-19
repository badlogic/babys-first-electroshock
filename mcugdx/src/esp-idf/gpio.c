#include "gpio.h"
#include "log.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#define TAG "mcugdx_gpio"

static adc_oneshot_unit_handle_t adc1_handle;
static adc_oneshot_unit_handle_t adc2_handle;
static bool adc_initialized = false;

static adc_cali_handle_t adc1_cali_handle = NULL;
static adc_cali_handle_t adc2_cali_handle = NULL;

#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES LEDC_TIMER_8_BIT
#define LEDC_FREQUENCY 25000

typedef struct {
	ledc_channel_t channel;
	int gpio_num;
	bool in_use;
} mcugdx_ledc_channel_config_t;

static mcugdx_ledc_channel_config_t ledc_channels[8] = {0};
static bool ledc_initialized = false;

static int find_free_channel(void) {
	for (int i = 0; i < 8; i++) {
		if (!ledc_channels[i].in_use) {
			return i;
		}
	}
	return -1;
}

// Find which channel a pin is assigned to
static int find_channel_for_pin(int pin) {
	for (int i = 0; i < 8; i++) {
		if (ledc_channels[i].in_use && ledc_channels[i].gpio_num == pin) {
			return i;
		}
	}
	return -1;
}

// Initialize LEDC if not already done
static esp_err_t init_ledc_if_needed(void) {
	if (ledc_initialized) {
		return ESP_OK;
	}

	// Configure LEDC timer
	ledc_timer_config_t ledc_timer = {
			.speed_mode = LEDC_MODE,
			.timer_num = LEDC_TIMER,
			.duty_resolution = LEDC_DUTY_RES,
			.freq_hz = LEDC_FREQUENCY,
			.clk_cfg = LEDC_AUTO_CLK};
	esp_err_t err = ledc_timer_config(&ledc_timer);
	if (err != ESP_OK) {
		return err;
	}

	ledc_initialized = true;
	return ESP_OK;
}

typedef struct {
    int gpio_num;
    bool in_use;
} adc_pin_config_t;

static adc_pin_config_t adc1_pins[10] = {0};  // ADC1: pins 1-10
static adc_pin_config_t adc2_pins[10] = {0};  // ADC2: pins 11-20

static int get_adc_channel(int pin) {
	// ADC1: GPIO1-10
	if (pin >= 1 && pin <= 10) {
		return pin - 1;// ADC1 channels 0-9
	}
	// ADC2: GPIO11-20
	else if (pin >= 11 && pin <= 20) {
		return pin - 11;// ADC2 channels 0-9
	}
	return -1;
}

static int get_adc_unit(int pin) {
	if (pin >= 1 && pin <= 10) return ADC_UNIT_1;
	if (pin >= 11 && pin <= 20) return ADC_UNIT_2;
	return -1;
}

static void track_adc_pin(int pin, bool in_use) {
    int adc_unit = get_adc_unit(pin);
    int channel = get_adc_channel(pin);

    if (adc_unit == ADC_UNIT_1 && channel >= 0) {
        adc1_pins[channel].gpio_num = pin;
        adc1_pins[channel].in_use = in_use;
    } else if (adc_unit == ADC_UNIT_2 && channel >= 0) {
        adc2_pins[channel].gpio_num = pin;
        adc2_pins[channel].in_use = in_use;
    }
}

static bool is_adc_unit_in_use(adc_unit_t unit) {
    adc_pin_config_t *pins = (unit == ADC_UNIT_1) ? adc1_pins : adc2_pins;
    for (int i = 0; i < 10; i++) {
        if (pins[i].in_use) {
            return true;
        }
    }
    return false;
}

static esp_err_t init_adc_if_needed(void) {
	if (adc_initialized) {
		return ESP_OK;
	}

	// Initialize ADC1
	adc_oneshot_unit_init_cfg_t adc1_config = {
			.unit_id = ADC_UNIT_1,
			.ulp_mode = ADC_ULP_MODE_DISABLE,
	};
	esp_err_t err = adc_oneshot_new_unit(&adc1_config, &adc1_handle);
	if (err != ESP_OK) {
		mcugdx_loge(TAG, "Failed to initialize ADC1");
		return err;
	}

	// Initialize ADC2
	adc_oneshot_unit_init_cfg_t adc2_config = {
			.unit_id = ADC_UNIT_2,
			.ulp_mode = ADC_ULP_MODE_DISABLE,
	};
	err = adc_oneshot_new_unit(&adc2_config, &adc2_handle);
	if (err != ESP_OK) {
		mcugdx_loge(TAG, "Failed to initialize ADC2");
		return err;
	}

	// Initialize calibration for ADC1
	adc_cali_handle_t handle1 = NULL;
	adc_cali_curve_fitting_config_t cali_config1 = {
			.unit_id = ADC_UNIT_1,
			.atten = ADC_ATTEN_DB_12,
			.bitwidth = ADC_BITWIDTH_DEFAULT,
	};
	err = adc_cali_create_scheme_curve_fitting(&cali_config1, &handle1);
	if (err == ESP_OK) {
		adc1_cali_handle = handle1;
	}

	// Initialize calibration for ADC2
	adc_cali_handle_t handle2 = NULL;
    adc_cali_curve_fitting_config_t cali_config2 = {
        .unit_id = ADC_UNIT_2,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
	err = adc_cali_create_scheme_curve_fitting(&cali_config2, &handle2);
	if (err == ESP_OK) {
		adc2_cali_handle = handle2;
	}

	adc_initialized = true;
	return ESP_OK;
}

static esp_err_t configure_adc_channel(int pin) {
	int channel = get_adc_channel(pin);
	if (channel < 0) {
		return ESP_ERR_INVALID_ARG;
	}

	adc_oneshot_chan_cfg_t config = {
			.atten = ADC_ATTEN_DB_12,
			.bitwidth = ADC_BITWIDTH_DEFAULT,
	};

	adc_unit_t unit = get_adc_unit(pin);
	if (unit == ADC_UNIT_1) {
		return adc_oneshot_config_channel(adc1_handle, (adc_channel_t) channel, &config);
	} else if (unit == ADC_UNIT_2) {
		return adc_oneshot_config_channel(adc2_handle, (adc_channel_t) channel, &config);
	}

	return ESP_ERR_INVALID_ARG;
}

void mcugdx_gpio_pin_mode(int pin, mcugdx_gpio_pin_mode_t mode, mcugdx_gpio_pull_t pull) {
   esp_err_t err;

   // Clean up any existing LEDC/PWM configuration
   int ledc_channel = find_channel_for_pin(pin);
   if (ledc_channel >= 0) {
       ledc_stop(LEDC_MODE, ledc_channels[ledc_channel].channel, 0);
       ledc_channels[ledc_channel].in_use = false;
       ledc_channels[ledc_channel].gpio_num = -1;
   }

   // Clean up ADC configuration only for this pin
   int adc_unit = get_adc_unit(pin);
   if (adc_unit >= 0) {
       track_adc_pin(pin, false);

       // Only delete ADC unit if no pins are using it anymore
       if (!is_adc_unit_in_use(adc_unit)) {
           if (adc_unit == ADC_UNIT_1 && adc1_handle != NULL) {
               adc_oneshot_del_unit(adc1_handle);
               adc1_handle = NULL;
               if (adc1_cali_handle != NULL) {
                   adc_cali_delete_scheme_curve_fitting(adc1_cali_handle);
                   adc1_cali_handle = NULL;
               }
           } else if (adc_unit == ADC_UNIT_2 && adc2_handle != NULL) {
               adc_oneshot_del_unit(adc2_handle);
               adc2_handle = NULL;
               if (adc2_cali_handle != NULL) {
                   adc_cali_delete_scheme_curve_fitting(adc2_cali_handle);
                   adc2_cali_handle = NULL;
               }
           }
           if (!is_adc_unit_in_use(ADC_UNIT_1) && !is_adc_unit_in_use(ADC_UNIT_2)) {
               adc_initialized = false;
           }
       }
   }

   // Configure new mode
   if (mode == MCUGDX_ANALOG_INPUT) {
       err = init_adc_if_needed();
       if (err != ESP_OK) {
           mcugdx_loge(TAG, "Failed to initialize ADC for pin %d", pin);
           return;
       }
       err = configure_adc_channel(pin);
       if (err != ESP_OK) {
           mcugdx_loge(TAG, "Failed to configure ADC channel for pin %d", pin);
           return;
       }
       track_adc_pin(pin, true);
       return;
   } else if (mode == MCUGDX_ANALOG_OUTPUT) {
       err = init_ledc_if_needed();
       if (err != ESP_OK) {
           mcugdx_loge(TAG, "Failed to initialize LEDC for pin %d", pin);
           return;
       }

       int channel_num = find_free_channel();
       if (channel_num < 0) {
           mcugdx_loge(TAG, "No free LEDC channels available");
           return;
       }

       ledc_channel_config_t ledc_conf = (ledc_channel_config_t){
           .speed_mode = LEDC_MODE,
           .channel = channel_num,
           .timer_sel = LEDC_TIMER,
           .intr_type = LEDC_INTR_DISABLE,
           .gpio_num = pin,
           .duty = 0,
           .hpoint = 0};

       err = ledc_channel_config(&ledc_conf);
       if (err != ESP_OK) {
           mcugdx_loge(TAG, "Failed to configure LEDC channel for pin %d", pin);
           return;
       }

       // Store the channel configuration
       ledc_channels[channel_num].channel = channel_num;
       ledc_channels[channel_num].gpio_num = pin;
       ledc_channels[channel_num].in_use = true;

       return;
   }

   // Digital I/O configuration
   err = gpio_set_direction((gpio_num_t) pin, mode == MCUGDX_DIGITAL_INPUT ? GPIO_MODE_INPUT : GPIO_MODE_OUTPUT);
   if (err != ESP_OK) {
       mcugdx_loge(TAG, "Could not set output direction on pin %i", pin);
       return;
   }

   switch (pull) {
       case MCUGDX_PULL_DOWN:
           err = gpio_set_pull_mode((gpio_num_t) pin, GPIO_PULLDOWN_ONLY);
           break;
       case MCUGDX_PULL_UP:
           err = gpio_set_pull_mode((gpio_num_t) pin, GPIO_PULLUP_ONLY);
           break;
       default:
           err = gpio_set_pull_mode((gpio_num_t) pin, GPIO_FLOATING);
           break;
   }
   if (err != ESP_OK) {
       mcugdx_loge(TAG, "Could not set pull mode on pin %i", pin);
       return;
   }
}

void mcugdx_gpio_digital_out(int pin, int value) {
	esp_err_t err = gpio_set_level((gpio_num_t) pin, value != 0 ? 1 : 0);
	if (err != ESP_OK) {
		mcugdx_loge(TAG, "Could not set level on pin %i", pin);
		return;
	}
}

int mcugdx_gpio_digital_in(int pin) {
	return gpio_get_level((gpio_num_t) pin);
}

void mcugdx_gpio_analog_out(int pin, int value) {
	int channel_num = find_channel_for_pin(pin);
	if (channel_num < 0) {
		mcugdx_loge(TAG, "Pin %d not configured for LEDC", pin);
		return;
	}

	// Clamp value to 8-bit range (0-255)
	value = value < 0 ? 0 : (value > 255 ? 255 : value);

	esp_err_t err = ledc_set_duty(LEDC_MODE, ledc_channels[channel_num].channel, value);
	if (err != ESP_OK) {
		mcugdx_loge(TAG, "Failed to set duty cycle for pin %d", pin);
		return;
	}

	err = ledc_update_duty(LEDC_MODE, ledc_channels[channel_num].channel);
	if (err != ESP_OK) {
		mcugdx_loge(TAG, "Failed to update duty cycle for pin %d", pin);
		return;
	}
}

int mcugdx_gpio_analog_in(int pin) {
	if (!adc_initialized) {
		mcugdx_loge(TAG, "ADC not initialized");
		return -1;
	}

	int channel = get_adc_channel(pin);
	if (channel < 0) {
		mcugdx_loge(TAG, "Pin %d does not support analog input", pin);
		return -1;
	}

	int adc_reading;
	esp_err_t err;
	adc_unit_t unit = get_adc_unit(pin);

	if (unit == ADC_UNIT_1) {
		err = adc_oneshot_read(adc1_handle, (adc_channel_t) channel, &adc_reading);
		if (err == ESP_OK && adc1_cali_handle != NULL) {
			int voltage;
			adc_cali_raw_to_voltage(adc1_cali_handle, adc_reading, &voltage);
			return voltage;
		}
	} else if (unit == ADC_UNIT_2) {
		err = adc_oneshot_read(adc2_handle, (adc_channel_t) channel, &adc_reading);
		if (err == ESP_OK && adc2_cali_handle != NULL) {
			int voltage;
			adc_cali_raw_to_voltage(adc2_cali_handle, adc_reading, &voltage);
			return voltage;
		}
	}

	if (err != ESP_OK) {
		mcugdx_loge(TAG, "Failed to read ADC value from pin %d", pin);
		return -1;
	}

	return adc_reading;
}