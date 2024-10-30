#include "mcugdx.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "config.h"
#include "webserver.h"

#define TAG "Screaming cauldron"

mcugdx_sound_id_t curr_sound = -1;
uint32_t sound_index = 0;

#define ULTRASONIC_INTERVAL 0.1
double last_ultrasonic_time;

#define NUM_LEDS 31

typedef struct {
	int32_t r, g, b;
} LED;

float led_offsets[NUM_LEDS];
LED led_buffer[NUM_LEDS];
bool current_is_screaming = false;
float transition_factor = 0.0f;
bool updating_audio = false;
bool restart = false;

void cauldron_set_color(int r, int g, int b) {
	config_t *config = config_lock();
	config->r = r;
	config->g = g;
	config->b = b;
	config_unlock();
	config_save();
}

void cauldron_set_volume(int volume) {
	config_t *config = config_lock();
	config->volume = volume;
	mcugdx_audio_set_master_volume(volume);
	config_unlock();
	config_save();
}

void initialize_led_offsets() {
	for (int i = 0; i < NUM_LEDS; i++) {
		led_offsets[i] = (float) rand() / RAND_MAX * 2 * M_PI;
	}
}

void clear_led_buffer() {
	memset(led_buffer, 0, sizeof(led_buffer));
}

void mix_color(int led_index, int r, int g, int b, float intensity) {
	led_buffer[led_index].r += r * intensity;
	led_buffer[led_index].g += g * intensity;
	led_buffer[led_index].b += b * intensity;
}

void apply_buffer() {
	config_t *config = config_lock();
	float brightness = config->brightness / 255.0f;
	config_unlock();
	for (int i = 0; i < NUM_LEDS; i++) {
		float r = (led_buffer[i].r > 255 ? 255 : led_buffer[i].r) * brightness;
		float g = (led_buffer[i].g > 255 ? 255 : led_buffer[i].g) * brightness;
		float b = (led_buffer[i].b > 255 ? 255 : led_buffer[i].b) * brightness;
		mcugdx_neopixels_set(i, (int) r, (int) g, (int) b);
	}
}

void idle_lights(float factor) {
	config_t *config = config_lock();
	int r = config->r, g = config->g, b = config->b;
	config_unlock();

	float current_time = mcugdx_time();
	float breath_speed = 1.5f;
	float min_brightness = 0.2f;
	float brightness_range = 1.0f - min_brightness;

	for (int i = 0; i < NUM_LEDS; i++) {
		float breath = (sinf(current_time * breath_speed + led_offsets[i]) + 1.0f) * 0.5f;
		float brightness = (min_brightness + breath * brightness_range) * factor;

		mix_color(i, r, g, b, brightness);
	}
}

void scream_lights(float factor) {
	float current_time = mcugdx_time();
	float scream_wave_speed = 5.0f;
	float scream_move_speed = 2.0f;
	int scream_min_red = 150;
	int scream_max_red = 255;

	for (int i = 0; i < NUM_LEDS; i++) {
		float position = (float) i / NUM_LEDS;
		float wave = sinf(current_time * scream_wave_speed * 2 * M_PI + position * 10 + current_time * scream_move_speed);
		wave = (wave + 1.0f) * 0.5f;

		int red_value = (int) (wave * (scream_max_red - scream_min_red) + scream_min_red);
		if (red_value > 255) red_value = 255;
		int orange_value = red_value / 6;

		mix_color(i, red_value, orange_value, 0, factor);
	}
}

void update_lights(bool is_screaming) {
	static float last_update_time = 0;
	float current_time = mcugdx_time();
	float delta_time = current_time - last_update_time;
	last_update_time = current_time;

	const float transition_to_scream_duration = 0.1f;  // 100ms
	const float transition_from_scream_duration = 1.0f;// 1000ms

	if (is_screaming != current_is_screaming) {
		current_is_screaming = is_screaming;
	}

	float transition_speed;
	if (current_is_screaming) {
		transition_speed = 1.0f / transition_to_scream_duration;
		transition_factor += delta_time * transition_speed;
		if (transition_factor > 1.0f) transition_factor = 1.0f;
	} else {
		transition_speed = 1.0f / transition_from_scream_duration;
		transition_factor -= delta_time * transition_speed;
		if (transition_factor < 0.0f) transition_factor = 0.0f;
	}

	clear_led_buffer();
	idle_lights(1.0f - transition_factor);
	scream_lights(transition_factor);
	apply_buffer();
}

void set_lights(int32_t r, int32_t g, int32_t b) {
	for (int i = 0; i < NUM_LEDS; i++) {
		mcugdx_neopixels_set(i, r, g, b);
	}
}

void adc_to_color(int value, int32_t &r, int32_t &g, int32_t &b) {
	if (value < 0) value = 0;
	if (value > 4095) value = 4095;

	float wavelength = 380.0f + (740.0f - 380.0f) * value / 4095.0f;
	float r_f = 0, g_f = 0, b_f = 0;

	if (wavelength >= 380 && wavelength < 440) {
		r_f = -(wavelength - 440) / (440 - 380);
		b_f = 1.0;
	} else if (wavelength >= 440 && wavelength < 490) {
		g_f = (wavelength - 440) / (490 - 440);
		b_f = 1.0;
	} else if (wavelength >= 490 && wavelength < 510) {
		g_f = 1.0;
		b_f = -(wavelength - 510) / (510 - 490);
	} else if (wavelength >= 510 && wavelength < 580) {
		r_f = (wavelength - 510) / (580 - 510);
		g_f = 1.0;
	} else if (wavelength >= 580 && wavelength < 645) {
		r_f = 1.0;
		g_f = -(wavelength - 645) / (645 - 580);
	} else if (wavelength >= 645 && wavelength <= 740) {
		r_f = 1.0;
	}

	float factor = 1.0;
	if (wavelength >= 380 && wavelength < 420) {
		factor = 0.3 + 0.7 * (wavelength - 380) / (420 - 380);
	} else if (wavelength >= 645 && wavelength <= 740) {
		factor = 0.3 + 0.7 * (740 - wavelength) / (740 - 645);
	}

	r = (int) (r_f * factor * 255);
	g = (int) (g_f * factor * 255);
	b = (int) (b_f * factor * 255);
}

extern "C" void app_main() {
	mcugdx_init();
	mcugdx_rofs_init();
	config_read();

	mcugdx_audio_config_t audio_config = {
			.sample_rate = 22050,
			.channels = MCUGDX_MONO,
			.bclk = 12,
			.ws = 13,
			.dout = 11};
	mcugdx_audio_init(&audio_config);
	mcugdx_audio_set_master_volume(config.volume);

	mcugdx_sound_t *sounds[100];
	size_t num_sounds = 0;
	for (int i = 0; i < mcugdx_rofs.num_files(); i++) {
		const char *file_name = mcugdx_rofs.file_name(i);
		if (strlen(file_name) >= 4 && strcmp(file_name + strlen(file_name) - 4, ".qoa") == 0) {
			sounds[num_sounds++] = mcugdx_sound_load(file_name, &mcugdx_rofs, MCUGDX_STREAMED, MCUGDX_MEM_EXTERNAL);
			mcugdx_log(TAG, "Loaded sound %s", file_name);
		}
	}
	mcugdx_log(TAG, "Loaded %li sounds", num_sounds);
	mcugdx_mem_print();

	mcugdx_ultrasonic_config_t ultrasonic_config = {
			.trigger = 1,
			.echo = 2,
			.interval = ULTRASONIC_INTERVAL};
	mcugdx_ultrasonic_init(&ultrasonic_config);

	mcugdx_neopixels_config_t neopixels_config = {
			.num_leds = NUM_LEDS,
			.pin = 3};
	if (!mcugdx_neopixels_init(&neopixels_config)) return;
	initialize_led_offsets();

#ifdef ESP_PLATFORM
	webserver_init();
#endif

	while (true) {
		if (updating_audio) {
			int i = 0;
			while (true) {
				set_lights(0, i++ % 2 == 0 ? 255 : 0, 0);
				mcugdx_neopixels_show_max_milli_ampere(500);
				mcugdx_sleep(500);

				if (restart) {
					mcugdx_sleep(500);
					mcugdx_quit();
				}
			}
		}

		uint32_t distance;
		if (curr_sound == -1) {
			if (mcugdx_ultrasonic_measure(20, &distance)) {
				if (distance < 10) {
					mcugdx_log(TAG, "Hand detected, playing sound");
					curr_sound = mcugdx_sound_play(sounds[sound_index++], 255, 0, MCUGDX_SINGLE_SHOT);
					if (sound_index >= num_sounds) sound_index = 0;
				}
			}
		}
		curr_sound = mcugdx_sound_is_playing(curr_sound) ? curr_sound : -1;

		update_lights(curr_sound != -1);
		mcugdx_neopixels_show_max_milli_ampere(600);
		mcugdx_sleep(10);
	}
}
