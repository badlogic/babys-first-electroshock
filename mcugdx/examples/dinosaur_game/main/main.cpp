#include "mcugdx.h"
#include <math.h>

#define TAG "Dino game"

typedef struct {
	mcugdx_image_t *img;
	int num_frames;
	float time_per_frame;
	bool loop;
	int frame;
	float passed_time;
} animation_t;

void animation_update(animation_t *anim, float delta_time) {
	anim->passed_time += delta_time;
	if (anim->passed_time > anim->time_per_frame) {
		anim->passed_time = 0;
		anim->frame++;
		if (anim->frame >= anim->num_frames) {
			if (anim->loop) {
				anim->frame = 0;
			} else {
				anim->frame--;
			}
		}
	}
}

void animation_draw(animation_t *anim, int x, int y, uint16_t color_key) {
	int frame_width = anim->img->width / anim->num_frames;
	int frame_height = anim->img->height;
	mcugdx_display_blit_region_keyed(anim->img, x, y, anim->frame * frame_width, 0, frame_width, frame_height, color_key);
}

typedef enum {
	DINO_RUN,
	DINO_JUMP
} dino_state_t;

typedef struct {
	dino_state_t state;
	animation_t *anim;
	float y;
	float vy;
} dino_t;

typedef enum {
	OBSTACLE_CACTUS_1,
	OBSTACLE_CACTUS_2,
	OBSTACLE_CACTUS_3,
	OBSTACLE_PTERO,
	OBSTACLE_NONE
} obstacle_type_t;

typedef struct {
	obstacle_type_t type;
	union {
		struct {
			int frame1;
			int frame2;
			int frame3;
		} cactus;
		struct {
			animation_t anim;
		} ptero;
	};
	float x;
	float y;
	float vx;
} obstacle_t;

mcugdx_image_t *dino_jump;
mcugdx_image_t *dino_run;
mcugdx_image_t *cactus;
mcugdx_image_t *cloud;
mcugdx_image_t *grass;
mcugdx_image_t *ground;
mcugdx_image_t *hill;
mcugdx_image_t *pterodactylus;
mcugdx_image_t *pterodactylus2;
mcugdx_image_t *sky;

animation_t dino_run_anim = {0};
animation_t dino_jump_anim = {0};
animation_t pterodactylus_anim = {0};
animation_t pterodactylus2_anim = {0};

// See https://docs.google.com/spreadsheets/d/1nlWkBR7OxU0TTQCja_hPZuGLlL0uLdvhydqvhc2n_ok/edit?usp=sharing
// Derrived from https://youtu.be/hG9SzQxaCm8?si=nk-uSBAlHBp1ftW7&t=781
float jump_height = 120.0f;
float jump_distance = 210.0f;
float velocity_x = 128;

float calculate_velocity_y() {
	return 2 * jump_height * velocity_x / (jump_distance / 2);
}

float calculate_gravity() {
	return -2 * jump_height * velocity_x * velocity_x / ((jump_distance / 2) * (jump_distance / 2));
}

dino_t dino;
#define NUM_OBSTACLES 16
obstacle_t obstacles[NUM_OBSTACLES];

mcugdx_button_handle_t escape_key;
mcugdx_button_handle_t jump_button;
mcugdx_button_handle_t speed_plus_button;
mcugdx_button_handle_t speed_minus_button;

#define PARALLAX_HILL_FACTOR 0.1f
#define PARALLAX_CLOUD_FACTOR 0.05f

float parallax_hill_x = 0;
float parallax_cloud_x = 0;
float ground_x = 0;

float randf() {
	return (float) rand() / (float) RAND_MAX;
}

int randi(int a, int b) {
	return a + rand() % (b - a + 1);
}

void generate_obstacle(int index) {
	if (index == 0) {
		if (obstacles[NUM_OBSTACLES - 1].type == OBSTACLE_NONE)
			obstacles[index].x = 320;
		else {
			obstacles[index].x = obstacles[NUM_OBSTACLES - 1].x + 200 + randf() * 220;
		}
	} else {
		obstacles[index].x = obstacles[index - 1].x + 200 + randf() * 220;
	}
	obstacles[index].vx = velocity_x;

	int cactus_y = 240 - ground->height - cactus->height;

	float selector = randf();
	if (selector < 0.4) {
		obstacles[index].type = OBSTACLE_CACTUS_1;
		obstacles[index].cactus.frame1 = randi(0, 6);
		obstacles[index].y = cactus_y;
	} else if (selector >= 0.4 && selector < 0.6) {
		obstacles[index].type = OBSTACLE_CACTUS_2;
		obstacles[index].cactus.frame1 = randi(0, 6);
		obstacles[index].cactus.frame2 = randi(0, 6);
		obstacles[index].y = cactus_y;
	} else if (selector >= 0.6 && selector < 0.75) {
		obstacles[index].type = OBSTACLE_CACTUS_3;
		obstacles[index].cactus.frame1 = randi(0, 6);
		obstacles[index].cactus.frame2 = randi(0, 6);
		obstacles[index].cactus.frame3 = randi(0, 6);
		obstacles[index].y = cactus_y;
	} else {
		obstacles[index].type = OBSTACLE_PTERO;
		obstacles[index].y = 240 - ground->height - pterodactylus->height - randi(20, 60);
		animation_t *anim = randf() > 0.5 ? &pterodactylus2_anim : &pterodactylus_anim;
		anim->passed_time = 0;
		anim->frame = 0;
		obstacles[index].ptero.anim = *anim;
	}
}

void load() {
	dino_jump = mcugdx_image_load("dino-jump.qoi", mcugdx_rofs_read_file, MCUGDX_MEM_EXTERNAL);
	dino_run = mcugdx_image_load("dino-run.qoi", mcugdx_rofs_read_file, MCUGDX_MEM_EXTERNAL);
	cactus = mcugdx_image_load("cactus.qoi", mcugdx_rofs_read_file, MCUGDX_MEM_EXTERNAL);
	cloud = mcugdx_image_load("cloud.qoi", mcugdx_rofs_read_file, MCUGDX_MEM_EXTERNAL);
	grass = mcugdx_image_load("grass.qoi", mcugdx_rofs_read_file, MCUGDX_MEM_EXTERNAL);
	ground = mcugdx_image_load("ground.qoi", mcugdx_rofs_read_file, MCUGDX_MEM_EXTERNAL);
	hill = mcugdx_image_load("hill.qoi", mcugdx_rofs_read_file, MCUGDX_MEM_EXTERNAL);
	pterodactylus = mcugdx_image_load("pterodactylus.qoi", mcugdx_rofs_read_file, MCUGDX_MEM_EXTERNAL);
	pterodactylus2 = mcugdx_image_load("pterodactylus-2.qoi", mcugdx_rofs_read_file, MCUGDX_MEM_EXTERNAL);
	sky = mcugdx_image_load("sky.qoi", mcugdx_rofs_read_file, MCUGDX_MEM_EXTERNAL);

	dino_run_anim = (animation_t){
			.img = dino_run,
			.num_frames = 4,
			.time_per_frame = 0.1,
			.loop = true,
			.frame = 0,
			.passed_time = 0,
	};

	dino_jump_anim = (animation_t){
			.img = dino_jump,
			.num_frames = 2,
			.time_per_frame = 0.15,
			.loop = false,
			.frame = 0,
			.passed_time = 0,
	};

	pterodactylus_anim = (animation_t){
			.img = pterodactylus,
			.num_frames = 4,
			.time_per_frame = 0.15,
			.loop = true,
			.frame = 0,
			.passed_time = 0};

	pterodactylus2_anim = (animation_t){
			.img = pterodactylus2,
			.num_frames = 4,
			.time_per_frame = 0.15,
			.loop = true,
			.frame = 0,
			.passed_time = 0};

	dino = (dino_t){
			.state = DINO_RUN,
			.anim = &dino_run_anim,
			.y = 0};

	for (int i = 0; i < NUM_OBSTACLES; i++) {
		generate_obstacle(i);
	}
}

void draw_background() {
	for (int x = 0; x < 2 * sky->width; x += sky->width) {
		mcugdx_display_blit(sky, x, 0);
	}

	for (int x = 0; x < 3 * cloud->width; x += cloud->width) {
		mcugdx_display_blit_keyed(cloud, parallax_cloud_x + x, 240 - ground->height - hill->height - cloud->height / 2, 0);
	}

	for (int x = 0; x < 3 * hill->width; x += hill->width) {
		mcugdx_display_blit_keyed(hill, parallax_hill_x + x, 240 - ground->height - hill->height, 0);
	}

	for (int x = 0; x < 6 * ground->width; x += ground->width) {
		mcugdx_display_blit(ground, ground_x + x, 240 - ground->height);
	}
}

void draw_objects() {
	animation_draw(dino.anim, 10, 240 - ground->height - dino.anim->img->height - dino.y, 0);

	int cactus_width = cactus->width / 7;
	for (int i = 0; i < NUM_OBSTACLES; i++) {
		obstacle_t *obst = &obstacles[i];

		if (obst->type == OBSTACLE_CACTUS_1) {
			mcugdx_display_blit_region_keyed(cactus, obst->x, obst->y, obst->cactus.frame1 * cactus_width, 0, cactus_width, cactus->height, 0);
		}

		if (obst->type == OBSTACLE_CACTUS_2) {
			mcugdx_display_blit_region_keyed(cactus, obst->x, obst->y, obst->cactus.frame1 * cactus_width, 0, cactus_width, cactus->height, 0);
			mcugdx_display_blit_region_keyed(cactus, obst->x + cactus_width, obst->y, obst->cactus.frame2 * cactus_width, 0, cactus_width, cactus->height, 0);
		}

		if (obst->type == OBSTACLE_CACTUS_3) {
			mcugdx_display_blit_region_keyed(cactus, obst->x, obst->y, obst->cactus.frame1 * cactus_width, 0, cactus_width, cactus->height, 0);
			mcugdx_display_blit_region_keyed(cactus, obst->x + cactus_width, obst->y, obst->cactus.frame2 * cactus_width, 0, cactus_width, cactus->height, 0);
			mcugdx_display_blit_region_keyed(cactus, obst->x + cactus_width * 2, obst->y, obst->cactus.frame3 * cactus_width, 0, cactus_width, cactus->height, 0);
		}

		if (obst->type == OBSTACLE_PTERO) {
			animation_draw(&obst->ptero.anim, obst->x, obst->y, 0);
		}
	}
}

bool update_state(float delta_time) {
	if (mcugdx_button_is_pressed(escape_key)) {
		return false;
	}

	parallax_cloud_x -= PARALLAX_CLOUD_FACTOR * velocity_x * delta_time;
	if (-parallax_cloud_x > cloud->width) parallax_cloud_x = 0;
	parallax_hill_x -= PARALLAX_HILL_FACTOR * velocity_x * delta_time;
	if (-parallax_hill_x > hill->width) parallax_hill_x = 0;
	ground_x -= velocity_x * delta_time;
	if (-ground_x > ground->width) ground_x = 0;

	static bool space_held = false;
	static float gravity_factor = 1;
	mcugdx_button_event_t event;
	while (mcugdx_button_get_event(&event)) {
		if (event.button == jump_button) {
			if (event.type == MCUGDX_BUTTON_PRESSED && dino.state != DINO_JUMP) {
				dino.state = DINO_JUMP;
				dino.anim = &dino_jump_anim;
				dino.anim->passed_time = 0;
				dino.anim->frame = 0;
				dino.y = 0;
				dino.vy = calculate_velocity_y();
				space_held = true;
				gravity_factor = 1;
			} else if (event.type == MCUGDX_BUTTON_RELEASED) {
				space_held = false;
			}
		}

		if (event.button == speed_plus_button && event.type == MCUGDX_BUTTON_RELEASED) {
			velocity_x += 16;
			mcugdx_log(TAG, "velocity_x: %f", velocity_x);
		}
		if (event.button == speed_minus_button && event.type == MCUGDX_BUTTON_RELEASED) {
			velocity_x -= 16;
			mcugdx_log(TAG, "velocity_x: %f", velocity_x);
		}
	}

	if (dino.state == DINO_JUMP) {
		dino.vy = dino.vy + gravity_factor * calculate_gravity() * delta_time;
		dino.y = dino.y + dino.vy * delta_time;

		if (!space_held && dino.vy > 0 && dino.y > jump_height / 3) {
			gravity_factor = jump_height / (dino.y * 0.75);
		}

		if (dino.y < 0) {
			dino.state = DINO_RUN;
			dino.anim = &dino_run_anim;
			dino.anim->passed_time = 0;
			dino.anim->frame = 0;
			dino.y = 0;
			dino.vy = 0;
		}
	}
	animation_update(dino.anim, delta_time);

	for (int i = 0; i < NUM_OBSTACLES; i++) {
		obstacle_t *obst = &obstacles[i];
		obst->x -= obst->vx * delta_time;

		switch (obst->type) {
			case OBSTACLE_CACTUS_1:
			case OBSTACLE_CACTUS_2:
			case OBSTACLE_CACTUS_3:
				if (obst->x < -(float) cactus->width * 3 / 7) {
					generate_obstacle(i);
				}
				break;
			case OBSTACLE_PTERO:
				if (obst->x < 320 && obst->vx == velocity_x) {
					obst->vx = velocity_x * (1 + randf() / 2);
				}
				animation_update(&obst->ptero.anim, delta_time);
				if (obst->x < -(float) pterodactylus->width / 4) {
					generate_obstacle(i);
				}
				break;
			default:
				break;
		}
	}

	return true;
}

extern "C" void app_main() {
	mcugdx_init();
	mcugdx_rofs_init();

	mcugdx_audio_config_t audio_config = {
			.sample_rate = 44100,
			.channels = MCUGDX_STEREO,
			.bclk = 47,
			.ws = 21,
			.dout = 38};
	mcugdx_audio_init(&audio_config);
	mcugdx_audio_set_master_volume(64);

	mcugdx_display_config_t display_config = {
			.driver = MCUGDX_ST7789,
			.native_width = 240,
			.native_height = 320,
			.mosi = 3,
			.sck = 4,
			.dc = 2,
			.cs = 1,
			.reset = -1};
	mcugdx_display_init(&display_config);
	mcugdx_display_set_orientation(MCUGDX_LANDSCAPE);

	load();
	mcugdx_mem_print();

	escape_key = mcugdx_button_create(2, 25, MCUGDX_KEY_ESCAPE);
	jump_button = mcugdx_button_create(3, 25, MCUGDX_KEY_SPACE);
	speed_plus_button = mcugdx_button_create(4, 25, MCUGDX_KEY_J);
	speed_minus_button = mcugdx_button_create(5, 25, MCUGDX_KEY_K);

	int frame = 0;
	float last_time = mcugdx_time();
	float delta_time = 0;
	while (true) {
		float now = mcugdx_time();
		delta_time = now - last_time;
		delta_time = 0.016;

		if (!update_state(delta_time)) {
			return;
		}

		draw_background();
		draw_objects();

		mcugdx_display_show();

		frame++;
		if (frame % 60 * 5 == 0) {
			double time = mcugdx_time();
			double total = time - now;
			mcugdx_log(TAG, "total: %.3f ms", total * 1000);
		}
	}
}
