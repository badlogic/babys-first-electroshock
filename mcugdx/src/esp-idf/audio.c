#include "audio.h"
#include "mutex.h"
#include "log.h"
#include "mcugdx.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"
#include "driver/gpio.h"

#define TAG "mcugdx_audio"

#define BUFFER_SIZE_IN_FRAMES 2048
static uint32_t sample_rate = 0;
static uint32_t channels = 0;
static int32_t *buffer;
mcugdx_mutex_t audio_lock;
static i2s_chan_handle_t channel;

void mix_task(void *args) {
	size_t buffer_size_in_bytes = BUFFER_SIZE_IN_FRAMES * channels * sizeof(int16_t);

	// FIXME use i2s_channel_preload_data to fill the initial buffer
	i2s_channel_enable(channel);

	while (true) {
		mcugdx_audio_mix(buffer, BUFFER_SIZE_IN_FRAMES, channels);
		size_t written_bytes = 0;
		while (written_bytes != buffer_size_in_bytes) {
			size_t wb = 0;
			i2s_channel_write(channel, buffer, buffer_size_in_bytes, &wb, 1000);
			written_bytes += wb;
		}
		vTaskDelay(pdMS_TO_TICKS(1));
	}
}

bool mcugdx_audio_init(mcugdx_audio_config_t *config) {
	sample_rate = config->sample_rate;
	channels = config->channels;
	if (!mcugdx_mutex_init(&audio_lock)) {
		mcugdx_loge(TAG, "Could not create audio lock");
		return false;
	}

	i2s_chan_config_t channel_config = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
	if (i2s_new_channel(&channel_config, &channel, NULL) != ESP_OK) {
		mcugdx_loge(TAG, "Could not create audio channel");
	}

	i2s_std_config_t mode_config = (i2s_std_config_t){
			.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(config->sample_rate),
			.slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, config->channels == MCUGDX_MONO ? I2S_SLOT_MODE_MONO : I2S_SLOT_MODE_STEREO),
			.gpio_cfg = {
					.mclk = I2S_GPIO_UNUSED,
					.bclk = (gpio_num_t) config->bclk,
					.ws = (gpio_num_t) config->ws,
					.dout = (gpio_num_t) config->dout,
					.din = I2S_GPIO_UNUSED,
					.invert_flags = {
							.mclk_inv = false,
							.bclk_inv = false,
							.ws_inv = false,
					},
			},
	};
	if (i2s_channel_init_std_mode(channel, &mode_config) != ESP_OK) {
		mcugdx_loge(TAG, "Could not initialize i2s standard mode");
		i2s_del_channel(channel);
		mcugdx_mutex_destroy(&audio_lock);
		return false;
	}

	buffer = (int32_t *) mcugdx_mem_alloc(sizeof(int32_t) * BUFFER_SIZE_IN_FRAMES * config->channels, MCUGDX_MEM_INTERNAL);

	uint32_t coreId = xPortGetCoreID();
	mcugdx_log(TAG, "This code is running on core %d", coreId);

	BaseType_t xReturned = xTaskCreatePinnedToCore(
			mix_task,
			"mcugdx_audio_task",
			4096,
			NULL,
			5,
			NULL,
			1);

	if (xReturned == pdPASS) {
		mcugdx_log(TAG, "Audio mixing task created successfully");
	} else {
		mcugdx_loge(TAG, "Failed to create audio mixing task\n");
		i2s_del_channel(channel);
		mcugdx_mutex_destroy(&audio_lock);
		return false;
	}
	return true;
}

uint32_t mcugdx_audio_get_sample_rate(void) {
	return sample_rate;
}
