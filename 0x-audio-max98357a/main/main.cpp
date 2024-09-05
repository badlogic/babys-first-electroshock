#include <stdint.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include <math.h>


#define I2S_WS GPIO_NUM_18
#define I2S_BCLK GPIO_NUM_17
#define I2S_DOUT GPIO_NUM_10

#define EXAMPLE_BUFF_SIZE 2048
#define SAMPLE_RATE 44100

static i2s_chan_handle_t tx_chan;// I2S tx channel handler

#define SAMPLE_RATE 44100 // Sample rate for the sine wave
#define FREQUENCY 440     // Frequency of the sine wave (440Hz)
#define AMPLITUDE 16000   // Amplitude of the sine wave
#define TWO_PI 6.283185307179586476925286766559


/*static void i2s_example_write_task(void *args) {

	int16_t *buffer = (int16_t *) malloc(EXAMPLE_BUFF_SIZE * sizeof(int16_t));

	size_t w_bytes = 0;
	uint32_t sample_rate = SAMPLE_RATE;
	uint32_t offset = 0;
	float phase = 0.0;
	float phase_increment = (TWO_PI * FREQUENCY) / sample_rate;

	while (1) {
		for (int i = 0; i < EXAMPLE_BUFF_SIZE; i++) {
			// Generate sine wave sample
			float sample = (sin(phase) + 1) / 2 * 0.4;
			phase += phase_increment;
			if (phase >= TWO_PI) {
				phase -= TWO_PI;
			}
			// Store the sample in the buffer
			buffer[i] = (int16_t)(sample * 32767); // Convert signed 16-bit to unsigned
		}

		// Write buffer to I2S
		if (i2s_channel_write(tx_chan, buffer, EXAMPLE_BUFF_SIZE * 2, &w_bytes, 1000) == ESP_OK) {
			printf("Write Task: i2s write %d bytes\n", w_bytes);
		} else {
			printf("Write Task: i2s write failed\n");
		}
	}

	ESP_ERROR_CHECK(i2s_channel_disable(tx_chan));
	vTaskDelete(NULL);
}*/


extern const uint8_t pcm_start[] asm("_binary_trimmed_o_pcm_start");
extern const uint8_t pcm_end[] asm("_binary_trimmed_o_pcm_end");

static void i2s_example_write_task(void *args) {

	uint16_t *buffer = (uint16_t *) malloc(EXAMPLE_BUFF_SIZE * 2);

	size_t w_bytes = 0;
	uint32_t offset = 0;

	while (1) {
        uint16_t min = 0xff;
        uint16_t max = 0;
		if (offset > (pcm_end - pcm_start)) {
			offset = 0;
		}

		for (int i = 0; i < EXAMPLE_BUFF_SIZE; i++) {
			offset++;
            uint8_t raw = pcm_start[offset];
			buffer[i] = raw << 6;
            if (buffer[i] < min) min = buffer[i];
            if (buffer[i] > max ) max = buffer[i];
            ;
		}

		if (i2s_channel_write(tx_chan, buffer, EXAMPLE_BUFF_SIZE * 2, &w_bytes, 1000) == ESP_OK) {
			printf(">min:%i,max:%i\n", min, max);
		} else {
			printf("Write Task: i2s write failed\n");
		}
	}
	ESP_ERROR_CHECK(i2s_channel_disable(tx_chan));

	vTaskDelete(NULL);
}


static void i2s_example_init_std_simplex(void) {
	i2s_chan_config_t tx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
	ESP_ERROR_CHECK(i2s_new_channel(&tx_chan_cfg, &tx_chan, NULL));


	i2s_std_config_t tx_std_cfg = {
			.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
			.slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT,
														I2S_SLOT_MODE_MONO),

			.gpio_cfg = {
					.mclk = I2S_GPIO_UNUSED,// some codecs may require mclk signal, this example doesn't need it
					.bclk = I2S_BCLK,
					.ws = I2S_WS,
					.dout = I2S_DOUT,
					.din = GPIO_NUM_NC,
					.invert_flags = {
							.mclk_inv = false,
							.bclk_inv = false,
							.ws_inv = false,
					},
			},
	};
	ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_chan, &tx_std_cfg));
}


extern "C" void app_main(void) {
	i2s_example_init_std_simplex();
	ESP_ERROR_CHECK(i2s_channel_enable(tx_chan));
	xTaskCreate(i2s_example_write_task, "i2s_example_write_task", 4096, NULL, 5, NULL);
}
