#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "math.h"
#include "mcugdx.h"
#include <string.h>
#define QOA_NO_STDIO
#define QOA_IMPLEMENTATION
#define QOA_MALLOC(size) heap_caps_calloc(1, size, MALLOC_CAP_SPIRAM)
#include "qoa.h"

#define SAMPLING_FREQUENCY 44100
#define BUFF_SIZE 2048
#define WS 1
#define BLCK 2
#define DOUT 3

extern const uint8_t music_start[] asm("_binary_music_wav_start");
extern const uint8_t music_end[]   asm("_binary_music_wav_end");

extern const uint8_t music_qoa_start[] asm("_binary_music_qoa_start");
extern const uint8_t music_qoa_end[]   asm("_binary_music_qoa_end");

i2s_chan_handle_t channel;

/*uint32_t get_wav_header_size(const uint8_t *wav_data) {
    uint32_t offset = 12; // Skip RIFF header (12 bytes)

    // Read through the chunks until we find the 'data' chunk
    while (memcmp(wav_data + offset, "data", 4) != 0) {
        // Get the chunk size
        uint32_t chunk_size = *(uint32_t *)(wav_data + offset + 4);
        offset += 8 + chunk_size; // Move past the chunk ('name' + size + data)
    }

    return offset + 8; // 'data' chunk starts at offset, add 8 bytes for 'data' and its size
}

static void i2s_example_write_task(void *args) {
    uint8_t *buffer = (uint8_t *) calloc(1, BUFF_SIZE);
    assert(buffer); // Check if buffer allocation is successful

    uint32_t pcm_offset = get_wav_header_size(music_start);
    uint32_t num_samples = (music_end - music_start - pcm_offset) / 2; // 16-bit samples
    int16_t *pcm = (int16_t *)(music_start + pcm_offset);

    printf("PCM offset: %lu\n", pcm_offset);
    printf("Samples: %lu\n", num_samples);

    size_t w_bytes = BUFF_SIZE;
    uint32_t sample_index = 0; // To track the current sample

    // Preload the initial buffer
    while (w_bytes == BUFF_SIZE) {
        ESP_ERROR_CHECK(i2s_channel_preload_data(channel, buffer, BUFF_SIZE, &w_bytes));
    }

    ESP_ERROR_CHECK(i2s_channel_enable(channel));

    while (1) {
        // If the end of PCM data is reached, reset the index to start over
        if (sample_index >= num_samples) {
            sample_index = 0; // Reset to start playback from the beginning
        }

        // Copy the PCM data to the buffer
        size_t bytes_to_copy = BUFF_SIZE;
        if (sample_index + BUFF_SIZE / 2 > num_samples) {
            bytes_to_copy = (num_samples - sample_index) * 2; // Remaining bytes
        }

        memcpy(buffer, &pcm[sample_index], bytes_to_copy);
        sample_index += bytes_to_copy / 2; // Move forward by the number of 16-bit samples

        // Write PCM data to the I2S channel
        ESP_ERROR_CHECK(i2s_channel_write(channel, buffer, bytes_to_copy, &w_bytes, 1000));

        // Small delay to allow task switching
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    free(buffer);
}*/

static void i2s_example_write_task(void *args) {
    uint8_t *buffer = (uint8_t *) calloc(1, BUFF_SIZE);
    assert(buffer); // Check if buffer allocation is successful

    // Set volume as a 16.16 fixed-point value (volume between 0 and 1)
    float volume_float = 0.3f; // Example volume level
    int32_t volume = (int32_t)(volume_float * (1 << 16)); // 16.16 fixed-point

    qoa_desc qoa;
    double decode_start = mcugdx_time();
    int16_t *samples = qoa_decode(music_qoa_start, music_qoa_end - music_qoa_start, &qoa);
    printf("channels: %i, sample rate: %i, samples: %i\n", qoa.channels, qoa.samplerate, qoa.samples);
    printf("decode: %f ms\n",  (mcugdx_time() - decode_start) * 1000);

    size_t w_bytes = BUFF_SIZE;
    uint32_t sample_index = 0; // To track the current sample

    // Preload the initial buffer
    while (w_bytes == BUFF_SIZE) {
        ESP_ERROR_CHECK(i2s_channel_preload_data(channel, buffer, BUFF_SIZE, &w_bytes));
    }

    ESP_ERROR_CHECK(i2s_channel_enable(channel));

    int printed = 0;
    while (1) {
        double prepare_start = mcugdx_time();

        // If the end of PCM data is reached, reset the index to start over
        if (sample_index >= qoa.samples) {
            sample_index = 0; // Reset to start playback from the beginning
        }

        // Copy and scale each sample manually
        int16_t *buffer_16 = (int16_t *)buffer; // Interpret buffer as int16_t for PCM data
        size_t samples_to_copy = BUFF_SIZE / 2; // BUFF_SIZE is in bytes, we want samples (16-bit)
        if (sample_index + samples_to_copy > qoa.samples) {
            samples_to_copy = qoa.samples - sample_index; // Adjust if we reach the end
        }

        for (size_t i = 0; i < samples_to_copy; i++) {
            int32_t scaled_sample = ((int32_t)samples[sample_index + i] * volume) >> 16; // Scale by volume
            buffer_16[i] = (int16_t)scaled_sample; // Store the scaled sample in the buffer
        }

        sample_index += samples_to_copy; // Move forward by the number of samples copied
        if (!printed) printf("prepare: %f ms\n", (mcugdx_time() - prepare_start) * 1000);

        // Write PCM data to the I2S channel
        double output_start = mcugdx_time();
        ESP_ERROR_CHECK(i2s_channel_write(channel, buffer, samples_to_copy * 2, &w_bytes, 1000));
        if (samples_to_copy * 2 != w_bytes) {
            printf("Not all samples written: %i -> %i\n", samples_to_copy * 2, w_bytes);
        }
        if (!printed) printf("output: %f ms\n", (mcugdx_time() - output_start) * 1000);

        // Small delay to allow task switching
        vTaskDelay(pdMS_TO_TICKS(1));

        printed = -1;
    }

    free(buffer);
}


static void init() {
	i2s_chan_config_t channel_config = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
	ESP_ERROR_CHECK(i2s_new_channel(&channel_config, &channel, NULL));

	i2s_std_config_t mode_config = {
			.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLING_FREQUENCY),
			.slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
			.gpio_cfg = {
					.mclk = I2S_GPIO_UNUSED,// some codecs may require mclk signal, this example doesn't need it
					.bclk = (gpio_num_t) BLCK,
					.ws = (gpio_num_t) WS,
					.dout = (gpio_num_t) DOUT,
					.din = I2S_GPIO_UNUSED,
					.invert_flags = {
							.mclk_inv = false,

							.bclk_inv = false,
							.ws_inv = false,
					},
			},
	};
	ESP_ERROR_CHECK(i2s_channel_init_std_mode(channel, &mode_config));
}

extern "C" void app_main() {
	printf("Audio test\n");

	init();
	xTaskCreate(i2s_example_write_task, "i2s_example_write_task", 4096, NULL, 5, NULL);
}