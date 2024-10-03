/**
 * Adapted from
 *
 * @file ultrasonic.c
 *
 * ESP-IDF driver for ultrasonic range meters, e.g. HC-SR04, HY-SRF05 and so on
 *
 * Ported from esp-open-rtos
 * Copyright (C) 2016, 2018 Ruslan V. Uss <unclerus@gmail.com>
 * BSD Licensed as described in the file LICENSE
 */
#include "ultrasonic.h"
#include "mutex.h"
#include "log.h"
#include "time.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <sys/time.h>

#define TAG "mcugdx_ultrasonic"

#define TRIGGER_LOW_DELAY 4
#define TRIGGER_HIGH_DELAY 10
#define PING_TIMEOUT 6000
#define ROUNDTRIP 58

#define timeout_expired(start, len) ((uint32_t) (get_time_us() - (start)) >= (len))

static mcugdx_mutex_t mutex;
static int trigger;
static int echo;
static uint32_t last_distance;
static uint32_t last_time;
static uint32_t interval;

static inline uint32_t get_time_us() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_usec;
}

bool mcugdx_ultrasonic_init(mcugdx_ultrasonic_config_t *config) {
	if (!mcugdx_mutex_init(&mutex)) {
		mcugdx_loge(TAG, "Could not create mutex");
		return false;
	}
	trigger = config->trigger;
	echo = config->echo;
	last_distance = 0;
	last_time = 0;

	gpio_reset_pin(trigger);
	gpio_reset_pin(echo);
	gpio_set_direction(trigger, GPIO_MODE_OUTPUT);
	gpio_set_direction(echo, GPIO_MODE_INPUT);
	gpio_set_level(trigger, 0);

	mcugdx_log(TAG, "Ultrasonic sensor initialized, trigger: %li, echo: %li", trigger, echo);

	return true;
}

bool mcugdx_ultrasonic_measure(uint32_t max_distance, uint32_t *distance) {
	if (!distance) {
		mcugdx_loge(TAG, "No distance argument provided");
		return false;
	}

	uint32_t now = mcugdx_time();
	if (now - last_ultrasonic_time < interval) {
		*distance = last_distance;
		return true;
	}
	last_time = now;

	gpio_set_level(trigger, 0);
	esp_rom_delay_us(TRIGGER_LOW_DELAY);
	gpio_set_level(trigger, 1);
	esp_rom_delay_us(TRIGGER_HIGH_DELAY);
	gpio_set_level(trigger, 0);

	if (gpio_get_level(echo)) {
		mcugdx_loge(TAG, "Previous ping hasn't ended");
		return false;
	}

	uint32_t start = get_time_us();
	while (!gpio_get_level(echo)) {
		if (timeout_expired(start, PING_TIMEOUT)) {
			mcugdx_loge(TAG, "No echo received");
			return false;
		}
	}

	uint32_t echo_start = get_time_us();
	uint32_t time = echo_start;
	uint32_t meas_timeout = echo_start + max_distance * ROUNDTRIP;
	while (gpio_get_level(echo)) {
		time = get_time_us();
		if (timeout_expired(echo_start, meas_timeout)) {
			mcugdx_loge(TAG, "Echo didn't end");
			return false;
		}
	}

	*distance = (time - echo_start) / ROUNDTRIP;
	return true;
}
