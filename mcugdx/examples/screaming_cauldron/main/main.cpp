#include "mcugdx.h"
#include <stdio.h>
#include <math.h>

#define TAG "Screaming cauldron"

#define NUM_LEDS 31
mcugdx_sound_id_t curr_sound = -1;

#define ULTRASONIC_INTERVAL 0.05
double last_ultrasonic_time;

float breath_speed = 0.2f;
float shimmer_speed = 0.5f;
float move_speed = 0.1f;
int base_green = 20;
int breath_intensity = 30;
int shimmer_intensity = 20;
int blue_hint = 0;

float scream_wave_speed = 5.0f;
float scream_move_speed = 2.0f;
int scream_min_red = 150;
int scream_max_red = 255;

float transition_duration = 0.5f;
float transition_progress = 0.0f;
bool transitioning_to_scream = false;

void idle_lights(float transition_factor) {
    float current_time = mcugdx_time();
    float breath = (sinf(current_time * breath_speed * 2 * M_PI) + 1.0f) * 0.5f;
    int breath_add = (int)(breath * breath_intensity);

    for (int i = 0; i < NUM_LEDS; i++) {
        float position = (float)i / NUM_LEDS;
        float shimmer = sinf(current_time * shimmer_speed * 2 * M_PI + position * 10 + current_time * move_speed);
        shimmer = (shimmer + 1.0f) * 0.5f;
        int shimmer_add = (int)(shimmer * shimmer_intensity);

        int green = base_green + breath_add + shimmer_add;
        if (green > 255) green = 255;

        int blue = (blue_hint > 0) ? (green * blue_hint / 255) : 0;
        green = (int)(green * (1 - transition_factor));
        blue = (int)(blue * (1 - transition_factor));
        green = green < base_green ? base_green : green;

        mcugdx_neopixels_set(i, 0, green, blue);
    }
}

void scream_lights(float transition_factor) {
    float current_time = mcugdx_time();
	if (transition_factor == 0) return;

    for (int i = 0; i < NUM_LEDS; i++) {
        float position = (float)i / NUM_LEDS;
        float wave = sinf(current_time * scream_wave_speed * 2 * M_PI + position * 10 + current_time * scream_move_speed);
        wave = (wave + 1.0f) * 0.5f;

        int red_value = (int)(wave * (scream_max_red - scream_min_red) + scream_min_red);
        if (red_value > 255) red_value = 255;

        red_value = (int)(red_value * transition_factor);
        int orange_value = red_value / 6;

        mcugdx_neopixels_set(i, red_value, orange_value, 0);
    }
}

void update_transition(bool is_screaming) {
    float current_time = mcugdx_time();
    static float last_update_time = 0;
    float delta_time = current_time - last_update_time;
    last_update_time = current_time;

    if (is_screaming != transitioning_to_scream) {
        transitioning_to_scream = is_screaming;
    }

    if (transitioning_to_scream) {
        transition_progress += delta_time / transition_duration;
        if (transition_progress > 1.0f) transition_progress = 1.0f;
    } else {
        transition_progress -= delta_time / transition_duration;
        if (transition_progress < 0.0f) transition_progress = 0.0f;
    }
}

void update_lights(bool is_screaming) {
    update_transition(is_screaming);
    idle_lights(transition_progress);
    scream_lights(transition_progress);
}

extern "C" void app_main() {
	mcugdx_rofs_init();

	mcugdx_audio_config_t audio_config = {
			.sample_rate = 44100,
			.channels = MCUGDX_MONO,
			.bclk = 12,
			.ws = 13,
			.dout = 11};
	mcugdx_audio_init(&audio_config);
	mcugdx_audio_set_master_volume(140);

	mcugdx_sound_t *sounds[] = {
			mcugdx_sound_load("scream1.qoa", mcugdx_rofs_read_file, MCUGDX_MEM_EXTERNAL),
			mcugdx_sound_load("scream2.qoa", mcugdx_rofs_read_file, MCUGDX_MEM_EXTERNAL),
			mcugdx_sound_load("scream3.qoa", mcugdx_rofs_read_file, MCUGDX_MEM_EXTERNAL)};
	size_t num_sounds = sizeof(sounds) / sizeof(sounds[0]);

	mcugdx_ultrasonic_config_t ultrasonic_config = {
			.trigger = 2,
			.echo = 3};
	mcugdx_ultrasonic_init(&ultrasonic_config);
	last_ultrasonic_time = mcugdx_time();

	mcugdx_neopixels_config_t neopixels_config = {
			.num_leds = NUM_LEDS,
			.pin = 5};
	if (!mcugdx_neopixels_init(&neopixels_config)) return;


    while (true) {
        uint32_t distance = 0;
        if (mcugdx_time() - last_ultrasonic_time > ULTRASONIC_INTERVAL && mcugdx_ultrasonic_measure(20, &distance)) {
            if (curr_sound == -1 && distance < 10) {
                mcugdx_log(TAG, "Hand detected, playing sound");
                size_t random_index = rand() % num_sounds;
                curr_sound = mcugdx_sound_play(sounds[random_index], 255, MCUGDX_SINGLE_SHOT);
            }
            last_ultrasonic_time = mcugdx_time();
        }
        curr_sound = mcugdx_sound_is_playing(curr_sound) ? curr_sound : -1;

        update_lights(curr_sound != -1);
        mcugdx_neopixels_show_max_milli_ampere(600);
    }
}
