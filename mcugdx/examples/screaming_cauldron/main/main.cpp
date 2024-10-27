#include "mcugdx.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef ESP_PLATFORM
#include "driver/adc.h"
#endif

#define TAG "Screaming cauldron"

mcugdx_sound_id_t curr_sound = -1;
uint32_t sound_index = 0;

#define ULTRASONIC_INTERVAL 0.1
double last_ultrasonic_time;

#define NUM_LEDS 31

typedef struct {
	int32_t r, g, b;
} LED;

LED idle_led = {0, 255, 0};
float led_offsets[NUM_LEDS];
LED led_buffer[NUM_LEDS];
bool current_is_screaming = false;
float transition_factor = 0.0f;

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
	for (int i = 0; i < NUM_LEDS; i++) {
		int r = led_buffer[i].r > 255 ? 255 : led_buffer[i].r;
		int g = led_buffer[i].g > 255 ? 255 : led_buffer[i].g;
		int b = led_buffer[i].b > 255 ? 255 : led_buffer[i].b;
		mcugdx_neopixels_set(i, r, g, b);
	}
}

void idle_lights(float factor) {
	float current_time = mcugdx_time();
	float breath_speed = 1.5f;
	float min_brightness = 0.2f;
	float brightness_range = 1.0f - min_brightness;

	for (int i = 0; i < NUM_LEDS; i++) {
		float breath = (sinf(current_time * breath_speed + led_offsets[i]) + 1.0f) * 0.5f;
		float brightness = (min_brightness + breath * brightness_range) * factor;

		mix_color(i, idle_led.r, idle_led.g, idle_led.b, brightness);
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

	mcugdx_audio_config_t audio_config = {
			.sample_rate = 22050,
			.channels = MCUGDX_MONO,
			.bclk = 12,
			.ws = 13,
			.dout = 11};
	mcugdx_audio_init(&audio_config);
	mcugdx_audio_set_master_volume(255);
	mcugdx_sound_type_t mode = MCUGDX_STREAMED;
	mcugdx_sound_t *sounds[] = {
			mcugdx_sound_load("scream11.qoa", &mcugdx_rofs, mode, MCUGDX_MEM_EXTERNAL),
			mcugdx_sound_load("scream12.qoa", &mcugdx_rofs, mode, MCUGDX_MEM_EXTERNAL),
			mcugdx_sound_load("scream13.qoa", &mcugdx_rofs, mode, MCUGDX_MEM_EXTERNAL),
			mcugdx_sound_load("scream1.qoa", &mcugdx_rofs, mode, MCUGDX_MEM_EXTERNAL),
			mcugdx_sound_load("scream2.qoa", &mcugdx_rofs, mode, MCUGDX_MEM_EXTERNAL),
			mcugdx_sound_load("scream3.qoa", &mcugdx_rofs, mode, MCUGDX_MEM_EXTERNAL),
			mcugdx_sound_load("scream4.qoa", &mcugdx_rofs, mode, MCUGDX_MEM_EXTERNAL),
			mcugdx_sound_load("scream5.qoa", &mcugdx_rofs, mode, MCUGDX_MEM_EXTERNAL),
			mcugdx_sound_load("scream6.qoa", &mcugdx_rofs, mode, MCUGDX_MEM_EXTERNAL),
			mcugdx_sound_load("scream7.qoa", &mcugdx_rofs, mode, MCUGDX_MEM_EXTERNAL),
			mcugdx_sound_load("scream8.qoa", &mcugdx_rofs, mode, MCUGDX_MEM_EXTERNAL),
			mcugdx_sound_load("scream9.qoa", &mcugdx_rofs, mode, MCUGDX_MEM_EXTERNAL),
			mcugdx_sound_load("scream10.qoa", &mcugdx_rofs, mode, MCUGDX_MEM_EXTERNAL),
	};
	size_t num_sounds = sizeof(sounds) / sizeof(sounds[0]);
	mcugdx_mem_print();

	mcugdx_ultrasonic_config_t ultrasonic_config = {
			// PERF BOARD
			//.trigger = 2,
			//.echo = 3};
			.trigger = 1,
			.echo = 2,
			.interval = ULTRASONIC_INTERVAL};
	mcugdx_ultrasonic_init(&ultrasonic_config);

	mcugdx_neopixels_config_t neopixels_config = {
			.num_leds = NUM_LEDS,
			// PERF BOARD
			// .pin = 5};
			.pin = 3};
	if (!mcugdx_neopixels_init(&neopixels_config)) return;
	initialize_led_offsets();


	mcugdx_prefs_init("cauldron");
	if (!mcugdx_prefs_read_int("r", &idle_led.r)) {
		mcugdx_prefs_write_int("r", 0);
		mcugdx_prefs_write_int("g", 0);
		mcugdx_prefs_write_int("b", 0);
	}
	mcugdx_prefs_read_int("g", &idle_led.g);
	mcugdx_prefs_read_int("b", &idle_led.b);
	mcugdx_button_create(5, 25, MCUGDX_KEY_SPACE);
#ifdef ESP_PLATFORM
	adc1_config_width(ADC_WIDTH_BIT_12);
	adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_11);
#endif

	bool is_config_mode = false;

	while (true) {
		if (!is_config_mode) {
			uint32_t distance;
			if (mcugdx_ultrasonic_measure(20, &distance)) {
				if (curr_sound == -1 && distance < 10) {
					mcugdx_log(TAG, "Hand detected, playing sound");
					curr_sound = mcugdx_sound_play(sounds[sound_index++], 128, 0, MCUGDX_SINGLE_SHOT);
					if (sound_index >= num_sounds) sound_index = 0;
				}
			}
			curr_sound = mcugdx_sound_is_playing(curr_sound) ? curr_sound : -1;

			update_lights(curr_sound != -1);
			mcugdx_neopixels_show_max_milli_ampere(600);

			mcugdx_button_event_t event;
			while (mcugdx_button_get_event(&event)) {
				if (event.type == MCUGDX_BUTTON_PRESSED) {
					mcugdx_log(TAG, "Config button pressed");
					is_config_mode = true;
				}
			}

		} else {
			mcugdx_button_event_t event;
			while (mcugdx_button_get_event(&event)) {
				if (event.type == MCUGDX_BUTTON_PRESSED) {
					mcugdx_log(TAG, "Config button pressed");
					is_config_mode = false;
					mcugdx_prefs_write_int("r", idle_led.r);
					mcugdx_prefs_write_int("g", idle_led.g);
					mcugdx_prefs_write_int("b", idle_led.b);
				}
			}

#ifdef ESP_PLATFORM
			adc_to_color(adc1_get_raw(ADC1_CHANNEL_3), idle_led.r, idle_led.g, idle_led.b);
#endif
			set_lights(idle_led.r, idle_led.g, idle_led.b);
			mcugdx_neopixels_show_max_milli_ampere(600);
		}

		mcugdx_sleep(10);
	}
}
