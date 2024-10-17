#include "config.h"
#include "mcugdx.h"
#include "string.h"

#define TAG "config"
#define NAMESPACE "window-opener"

config_t config = {
        .device_name = "window-opener",
		.ssid = NULL,
		.password = NULL,
		.min_temp = 16,
		.max_temp = 19,
        .manual = false
                };

static mcugdx_mutex_t mutex;
static bool is_initialized = false;

void config_read(void) {
	if (!is_initialized) {
		mcugdx_prefs_init(NAMESPACE);
		mcugdx_mutex_init(&mutex);
	}

	mcugdx_mutex_lock_l(&mutex, __FILE__, __LINE__);
    config.device_name = config.ssid = mcugdx_prefs_read_string("device_name");
    if (!config.device_name || strlen(config.device_name) == 0) {
        mcugdx_loge(TAG, "Couldn't read device_name, using default window-opener");
        config.device_name = "window-opener";
    }
	config.ssid = mcugdx_prefs_read_string("ssid");
	config.password = mcugdx_prefs_read_string("password");
	if (!mcugdx_prefs_read_int("min_temp", &config.min_temp)) {
		mcugdx_loge(TAG, "Couldn't read min_temp, using default 16");
		config.min_temp = 16;
	}
	if (!mcugdx_prefs_read_int("max_temp", &config.max_temp)) {
		mcugdx_loge(TAG, "Couldn't read max_temp, using default 19");
		config.max_temp = 19;
	}
    int32_t manual;
    if (!mcugdx_prefs_read_int("manual", &manual)) {
        mcugdx_loge(TAG, "Couldn't read manual, using default false");
		config.manual = false;
    }
    config.manual = manual != 0;
	mcugdx_mutex_unlock_l(&mutex, __FILE__, __LINE__);
}

void config_print(void) {
	mcugdx_mutex_lock_l(&mutex, __FILE__, __LINE__);
	mcugdx_log(TAG, "name: %s, ssid: %s, password: %s, min_temp: %li, max_temp: %li, manual: %s", config.device_name ? config.device_name : "null", config.ssid ? config.ssid : "null", config.password ? config.password : "null", config.min_temp, config.max_temp, config.manual ? "true" : "false");
	mcugdx_mutex_unlock_l(&mutex, __FILE__, __LINE__);
}

void config_save(void) {
    mcugdx_log(TAG, "Saving config");
    config_print();
	mcugdx_mutex_lock_l(&mutex, __FILE__, __LINE__);
    if (!mcugdx_prefs_write_string("device_name", config.device_name ? config.device_name : "window-opener")) {
		mcugdx_loge(TAG, "Could not save ssid");
	}
	if (!mcugdx_prefs_write_string("ssid", config.ssid ? config.ssid : "")) {
		mcugdx_loge(TAG, "Could not save ssid");
	}
	if (!mcugdx_prefs_write_string("password", config.password ? config.password : "")) {
		mcugdx_loge(TAG, "Could not save password");
	}
	if (!mcugdx_prefs_write_int("min_temp", config.min_temp)) {
		mcugdx_loge(TAG, "Could not save min_temp");
	}
	if (!mcugdx_prefs_write_int("max_temp", config.max_temp)) {
		mcugdx_loge(TAG, "Could not save max_temp");
	}
    if (!mcugdx_prefs_write_int("manual", config.manual ? -1 : 0)) {
        mcugdx_loge(TAG, "Could not save manual");
    }
	mcugdx_mutex_unlock_l(&mutex, __FILE__, __LINE__);
}

config_t *config_lock(void) {
	mcugdx_mutex_lock_l(&mutex, __FILE__, __LINE__);
	return &config;
}

void config_unlock(void) {
	mcugdx_mutex_unlock_l(&mutex, __FILE__, __LINE__);
}
