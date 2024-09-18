#include "neopixels.h"
#include "mem.h"
#include "log.h"

#include <driver/spi_master.h>
#include <driver/gpio.h>
#include "esp_heap_caps.h"
#include <string.h>

#define TAG "mcugdx_neopixels"

static mcugdx_neopixels_config_t config;
static mcugdx_neopixel_t *pixels;
static uint8_t *spi_buffer;
static size_t num_spi_bytes;
static spi_device_handle_t spi;
extern size_t internal_mem;

mcugdx_result_t mcugdx_neopixels_init(mcugdx_neopixels_config_t *user_config) {
	config = *user_config;
	num_spi_bytes = config.num_leds * 3 * 4;// 4 bytes spi bytes per pixel byte, 1 byte per 2 bits

	spi_bus_config_t buscfg = {
			.mosi_io_num = config.pin,
			.miso_io_num = -1,
			.sclk_io_num = -1,
			.quadhd_io_num = -1,
			.quadwp_io_num = -1,
			.data4_io_num = -1,
			.data5_io_num = -1,
			.data6_io_num = -1,
			.data7_io_num = -1,
			.max_transfer_sz = num_spi_bytes * 8,
			.flags = 0};

	if (spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO) != ESP_OK) {
		mcugdx_loge(TAG, "Could not initialize SPI bus");
		return MCUGDX_ERROR;
	}

	spi_device_interface_config_t devcfg = {
			.clock_speed_hz = 3200000,// 3.2MHz
			.mode = 0,
			.spics_io_num = -1,
			.queue_size = 1,
			.flags = SPI_DEVICE_NO_DUMMY
	};

	if (spi_bus_add_device(SPI2_HOST, &devcfg, &spi) != ESP_OK) {
		mcugdx_loge(TAG, "Could not add SPI device");
	}

	pixels = mcugdx_mem_alloc(sizeof(mcugdx_neopixel_t) * config.num_leds, MCUGDX_MEM_INTERNAL);
    memset(pixels, 0, sizeof(mcugdx_neopixel_t) * config.num_leds);
	spi_buffer = heap_caps_malloc(num_spi_bytes, MALLOC_CAP_DMA);
    memset(spi_buffer, 0, num_spi_bytes);

	mcugdx_log(TAG, "Initialized %li neopixels", config.num_leds);

	return MCUGDX_OK;
}

void mcugdx_neopixels_set(uint32_t index, uint8_t r, uint8_t g, uint8_t b) {
	if (index > config.num_leds - 1) {
		return;
	}

	mcugdx_neopixel_t *pixel = &pixels[index];
	pixel->r = r;
	pixel->g = g;
	pixel->b = b;
}

static void color_to_spi(uint8_t color, uint8_t *out) {
    for (int i = 0; i < 4; i++) {
        uint8_t bits = (color >> (6 - 2*i)) & 0x03;
        out[i] = (bits & 0x02 ? 0xE0 : 0x80) | (bits & 0x01 ? 0x0E : 0x08);
    }
}

void mcugdx_neopixels_show() {
    for (int i = 0, j = 0; i < config.num_leds; i++) {
        color_to_spi(pixels[i].g, &spi_buffer[j]);
        j += 4;
        color_to_spi(pixels[i].r, &spi_buffer[j]);
        j += 4;
        color_to_spi(pixels[i].b, &spi_buffer[j]);
        j += 4;
    }

    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = num_spi_bytes * 8;
    t.tx_buffer = spi_buffer;
    ret = spi_device_polling_transmit(spi, &t);
    assert(ret == ESP_OK);
}
