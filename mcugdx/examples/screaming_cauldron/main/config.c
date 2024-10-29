#include "config.h"
#include "mcugdx.h"
#include "string.h"
#include "password.h"

#define TAG "config"
#define NAMESPACE "cauldron"

config_t config = {0};

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
		mcugdx_loge(TAG, "Couldn't read device_name, using default screaming-cauldron");
		config.device_name = "screaming-cauldron";
	}

	config.ssid = mcugdx_prefs_read_string("ssid");
	if (!config.ssid) {
		config.ssid = strdup(DEFAULT_SSID);
		mcugdx_loge(TAG, "Couldn't read ssid, using default %s", config.ssid);
	}

	config.password = mcugdx_prefs_read_string("password");
	if (!config.password) {
		config.password = strdup(DEFAULT_PASSWORD);
		mcugdx_loge(TAG, "Couldn't read password, using default %s", config.password);
	}

	if (!mcugdx_prefs_read_int("r", &config.r)) {
		mcugdx_loge(TAG, "Couldn't read r, using default 0");
		config.r = 0;
	}
	if (!mcugdx_prefs_read_int("g", &config.g)) {
		mcugdx_loge(TAG, "Couldn't read g, using default 255");
		config.g = 255;
	}
	if (!mcugdx_prefs_read_int("b", &config.b)) {
		mcugdx_loge(TAG, "Couldn't read g, using default 255");
		config.g = 255;
	}
	if (!mcugdx_prefs_read_int("br", &config.brightness)) {
		mcugdx_loge(TAG, "Couldn't read brightness, using default 255");
		config.brightness = 255;
	}
	if (!mcugdx_prefs_read_int("volume", &config.volume)) {
		mcugdx_loge(TAG, "Couldn't read volume, using default 127");
		config.volume = 255;
	}
	mcugdx_mutex_unlock_l(&mutex, __FILE__, __LINE__);
}

void config_print(void) {
	mcugdx_mutex_lock_l(&mutex, __FILE__, __LINE__);
	mcugdx_log(TAG, "name: %s, ssid: %s, password: %s, r: %li, g: %li, b: %li, br: %li, volume: %li", config.device_name ? config.device_name : "null", config.ssid ? config.ssid : "null", config.password ? config.password : "null", config.r, config.g, config.b, config.brightness, config.volume);
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
	if (!mcugdx_prefs_write_int("r", config.r)) {
		mcugdx_loge(TAG, "Could not save r");
	}
	if (!mcugdx_prefs_write_int("g", config.g)) {
		mcugdx_loge(TAG, "Could not save g");
	}
	if (!mcugdx_prefs_write_int("b", config.b)) {
		mcugdx_loge(TAG, "Could not save b");
	}
	if (!mcugdx_prefs_write_int("br", config.brightness)) {
		mcugdx_loge(TAG, "Could not save brightness");
	}
	if (!mcugdx_prefs_write_int("volume", config.volume)) {
		mcugdx_loge(TAG, "Could not save volume");
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
