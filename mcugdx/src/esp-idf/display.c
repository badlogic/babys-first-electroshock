// Adapted from:
//
// - https://github.com/nopnop2002/esp-idf-st7789
// - https://github.com/nopnop2002/esp-idf-ili9340/tree/master
//
// Both licensed under:
//
// MIT License
//
// Copyright (c) 2020 nopnop2002
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// - https://github.com/adafruit/Adafruit_ILI9341
//
// Licensed under MIT, but has no LICENSE file in repository, just these paragraphs
//
// Adafruit invests time and resources providing this open source code, please
// support Adafruit and open-source hardware by purchasing products from Adafruit!
//
// Written by Limor Fried/Ladyada for Adafruit Industries. MIT license, all text
// above must be included in any redistribution


#include "display.h"

#include <string.h>
#include <inttypes.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <driver/spi_master.h>
#include <driver/gpio.h>
#include "esp_heap_caps.h"
#include "log.h"

#define TAG "mcugdx_display"

#define BUFFER_SIZE 32768

#define MADCTL_MY 0x80 ///< Bottom to top
#define MADCTL_MX 0x40 ///< Right to left
#define MADCTL_MV 0x20 ///< Reverse Mode
#define MADCTL_ML 0x10 ///< LCD refresh Bottom to top
#define MADCTL_RGB 0x00///< Red-Green-Blue pixel order
#define MADCTL_BGR 0x08///< Blue-Green-Red pixel order
#define MADCTL_MH 0x04 ///< LCD refresh right to left
#define MADCTL 0x36

extern size_t internal_mem;

mcugdx_display_t display;
static mcugdx_display_driver_t driver;
static int dc;
static spi_device_handle_t spi_handle;
static uint8_t pixel_order = MADCTL_RGB;

void pin_mode(int pin, gpio_mode_t mode, int level) {
	gpio_reset_pin(pin);
	gpio_set_direction(pin, mode);
	gpio_set_level(pin, level);
}

void delay(int ms) {
	int _ms = ms + (portTICK_PERIOD_MS - 1);
	vTaskDelay(_ms / portTICK_PERIOD_MS);
}

void spi_write_bytes(spi_device_handle_t device, const uint8_t *data, size_t length) {
	spi_transaction_t SPITransaction;
	esp_err_t ret;

	if (length > 0) {
		memset(&SPITransaction, 0, sizeof(spi_transaction_t));
		SPITransaction.length = length * 8;
		SPITransaction.tx_buffer = data;
		ret = spi_device_polling_transmit(device, &SPITransaction);
		assert(ret == ESP_OK);
	}
}

void spi_write_command(spi_device_handle_t device, int dc, uint8_t cmd) {
	static uint8_t byte = 0;
	byte = cmd;
	gpio_set_level(dc, 0);
	spi_write_bytes(device, &byte, 1);
}

void spi_write_data_byte(spi_device_handle_t device, int dc, uint8_t data) {
	static uint8_t byte = 0;

	byte = data;
	gpio_set_level(dc, 1);
	spi_write_bytes(device, &byte, 1);
}


void spi_write_data_word(spi_device_handle_t device, int dc, uint16_t data) {
	static uint8_t bytes[2];
	bytes[0] = (data >> 8) & 0xFF;
	bytes[1] = data & 0xFF;
	gpio_set_level(dc, 1);
	spi_write_bytes(device, bytes, 2);
}

void spi_write_addr(spi_device_handle_t device, int dc, uint16_t addr1, uint16_t addr2) {
	static uint8_t bytes[4];
	bytes[0] = (addr1 >> 8) & 0xFF;
	bytes[1] = addr1 & 0xFF;
	bytes[2] = (addr2 >> 8) & 0xFF;
	bytes[3] = addr2 & 0xFF;
	gpio_set_level(dc, 1);
	spi_write_bytes(device, bytes, 4);
}

void init_st7789(spi_device_handle_t device, int dc) {
	spi_write_command(device, dc, 0x01);//Software Reset
	delay(150);

	spi_write_command(device, dc, 0x11);//Sleep Out
	delay(10);

	spi_write_command(device, dc, 0x3A);//Interface Pixel Format
	spi_write_data_byte(device, dc, 0x55);
	delay(10);

	spi_write_command(device, dc, 0x36);//Memory Data Access Control
	spi_write_data_byte(device, dc, pixel_order);

	spi_write_command(device, dc, 0x2A);//Column Address Set
	spi_write_data_byte(device, dc, 0x00);
	spi_write_data_byte(device, dc, 0x00);
	spi_write_data_byte(device, dc, 0x00);
	spi_write_data_byte(device, dc, 0xF0);

	spi_write_command(device, dc, 0x2B);//Row Address Set
	spi_write_data_byte(device, dc, 0x00);
	spi_write_data_byte(device, dc, 0x00);
	spi_write_data_byte(device, dc, 0x00);
	spi_write_data_byte(device, dc, 0xF0);

	spi_write_command(device, dc, 0x21);//Display Inversion On
	delay(10);

	spi_write_command(device, dc, 0x13);//Normal Display Mode On
	delay(10);

	spi_write_command(device, dc, 0x29);//Display ON
	delay(10);
}

void init_ili9143(spi_device_handle_t device, int dc) {
	spi_write_command(device, dc, 0xC0);//Power Control 1
	spi_write_data_byte(device, dc, 0x23);

	spi_write_command(device, dc, 0xC1);//Power Control 2
	spi_write_data_byte(device, dc, 0x10);

	spi_write_command(device, dc, 0xC5);//VCOM Control 1
	spi_write_data_byte(device, dc, 0x3E);
	spi_write_data_byte(device, dc, 0x28);

	spi_write_command(device, dc, 0xC7);//VCOM Control 2
	spi_write_data_byte(device, dc, 0x86);

	spi_write_command(device, dc, 0x36);                //Memory Access Control
	spi_write_data_byte(device, dc, pixel_order);//Right top start, BGR color filter panel
	//spi_write_data_byte(device, dc, 0x00);//Right top start, RGB color filter panel

	spi_write_command(device, dc, 0x3A);  //Pixel Format Set
	spi_write_data_byte(device, dc, 0x55);//65K color: 16-bit/pixel

	spi_write_command(device, dc, 0x20);//Display Inversion OFF

	spi_write_command(device, dc, 0xB1);//Frame Rate Control
	spi_write_data_byte(device, dc, 0x00);
	spi_write_data_byte(device, dc, 0x18);

	spi_write_command(device, dc, 0xB6);//Display Function Control
	spi_write_data_byte(device, dc, 0x08);
	spi_write_data_byte(device, dc, 0xA2);// REV:1 GS:0 SS:0 SM:0
	spi_write_data_byte(device, dc, 0x27);
	spi_write_data_byte(device, dc, 0x00);

	spi_write_command(device, dc, 0x26);//Gamma Set
	spi_write_data_byte(device, dc, 0x01);

	spi_write_command(device, dc, 0xE0);//Positive Gamma Correction
	spi_write_data_byte(device, dc, 0x0F);
	spi_write_data_byte(device, dc, 0x31);
	spi_write_data_byte(device, dc, 0x2B);
	spi_write_data_byte(device, dc, 0x0C);
	spi_write_data_byte(device, dc, 0x0E);
	spi_write_data_byte(device, dc, 0x08);
	spi_write_data_byte(device, dc, 0x4E);
	spi_write_data_byte(device, dc, 0xF1);
	spi_write_data_byte(device, dc, 0x37);
	spi_write_data_byte(device, dc, 0x07);
	spi_write_data_byte(device, dc, 0x10);
	spi_write_data_byte(device, dc, 0x03);
	spi_write_data_byte(device, dc, 0x0E);
	spi_write_data_byte(device, dc, 0x09);
	spi_write_data_byte(device, dc, 0x00);

	spi_write_command(device, dc, 0xE1);//Negative Gamma Correction
	spi_write_data_byte(device, dc, 0x00);
	spi_write_data_byte(device, dc, 0x0E);
	spi_write_data_byte(device, dc, 0x14);
	spi_write_data_byte(device, dc, 0x03);
	spi_write_data_byte(device, dc, 0x11);
	spi_write_data_byte(device, dc, 0x07);
	spi_write_data_byte(device, dc, 0x31);
	spi_write_data_byte(device, dc, 0xC1);
	spi_write_data_byte(device, dc, 0x48);
	spi_write_data_byte(device, dc, 0x08);
	spi_write_data_byte(device, dc, 0x0F);
	spi_write_data_byte(device, dc, 0x0C);
	spi_write_data_byte(device, dc, 0x31);
	spi_write_data_byte(device, dc, 0x36);
	spi_write_data_byte(device, dc, 0x0F);

	spi_write_command(device, dc, 0x11);//Sleep Out
	delay(120);

	spi_write_command(device, dc, 0x29);//Display ON
}

mcugdx_result_t mcugdx_display_init(mcugdx_display_config_t *display_cfg) {
	// Setup SPI2 bus
	spi_bus_config_t bus_config = {
			.mosi_io_num = display_cfg->mosi,
			.miso_io_num = -1,
			.sclk_io_num = display_cfg->sck,
			.quadhd_io_num = -1,
			.quadwp_io_num = -1,
			.data4_io_num = -1,
			.data5_io_num = -1,
			.data6_io_num = -1,
			.data7_io_num = -1,
			.max_transfer_sz = BUFFER_SIZE * 8,
			.flags = 0};

	esp_err_t ret = spi_bus_initialize(SPI2_HOST, &bus_config, SPI_DMA_CH_AUTO);
	if (ret != ESP_OK) {
		return MCUGDX_ERROR;
	}
	mcugdx_log(TAG, "Initialized SPI bus");

	// Setup up display device, starting with CS and DC pins followed by adding the SPI device
	if (display_cfg->cs >= 0) pin_mode(display_cfg->cs, GPIO_MODE_OUTPUT, 0);
	pin_mode(display_cfg->dc, GPIO_MODE_OUTPUT, 0);

	if (display_cfg->reset >= 0) {
		mcugdx_log(TAG, "Resetting display via pin %i", display_cfg->reset);
		pin_mode(display_cfg->reset, GPIO_MODE_OUTPUT, 1);
		delay(100);
		gpio_set_level(display_cfg->reset, 1);
		delay(100);
		gpio_set_level(display_cfg->reset, 0);
		delay(100);
		gpio_set_level(display_cfg->reset, 1);
		delay(100);
	}

	spi_device_interface_config_t device_config;
	memset(&device_config, 0, sizeof(device_config));
	switch (display_cfg->driver) {
		case MCUGDX_ST7789:
			device_config.clock_speed_hz = SPI_MASTER_FREQ_80M;
			pixel_order = MADCTL_RGB;
			break;
		case MCUGDX_ILI9341:
			device_config.clock_speed_hz = SPI_MASTER_FREQ_40M;
			pixel_order = MADCTL_BGR;
			break;
		default:
			mcugdx_loge(TAG, "Unknown display driver %i\n", display_cfg->driver);
			return MCUGDX_ERROR;
	}
	device_config.queue_size = 7;
	device_config.mode = 3;
	device_config.flags = SPI_DEVICE_NO_DUMMY;
	device_config.spics_io_num = display_cfg->cs >= 0 ? display_cfg->cs : -1;

	ret = spi_bus_add_device(SPI2_HOST, &device_config, &spi_handle);
	mcugdx_log(TAG, "Added SPI display device");

	// Setup internal display struct
	driver = display_cfg->driver;
	display.native_width = display.width = display_cfg->native_width;
	display.native_height = display.height = display_cfg->native_height;
	display.orientation = MCUGDX_PORTRAIT;
	dc = display_cfg->dc;
	mcugdx_mem_print();
	mcugdx_log(TAG, "Largest DMA block %li", heap_caps_get_largest_free_block(MALLOC_CAP_DMA));
	size_t num_bytes = display.width * display.height * sizeof(uint16_t);
	mcugdx_log(TAG, "Trying to allocate %li frame buffer bytes", num_bytes);
	display.frame_buffer = heap_caps_calloc(num_bytes, 1, MALLOC_CAP_DMA);
	mcugdx_log(TAG, "Frame buffer at %p\n", display.frame_buffer);
	internal_mem += display.width * display.height * sizeof(uint16_t);

	// Send init commands to display
	switch (driver) {
		case MCUGDX_ST7789:
			init_st7789(spi_handle, dc);
			break;
		case MCUGDX_ILI9341:
			init_ili9143(spi_handle, dc);
			break;
		default:
			mcugdx_loge(TAG, "Unknown display driver %i\n", driver);
			return MCUGDX_ERROR;
	}

	// Set orientation to portrait by default
	mcugdx_display_set_orientation(MCUGDX_PORTRAIT);
	return MCUGDX_OK;
}

void mcugdx_display_set_orientation(mcugdx_display_orientation_t orientation) {
	uint8_t madctl = orientation = orientation % 4;

	switch (orientation) {
		case MCUGDX_PORTRAIT:
			madctl = (MADCTL_MY | pixel_order);
			display.width = display.native_width;
			display.height = display.native_height;
			break;
		case MCUGDX_LANDSCAPE:
			madctl = (MADCTL_MY | MADCTL_MV | pixel_order);
			display.width = display.native_height;
			display.height = display.native_width;
			break;
		default:
			mcugdx_loge(TAG, "Unsupported display orientation %i\n", orientation);
	}
	spi_write_command(spi_handle, dc, MADCTL);
	spi_write_data_byte(spi_handle, dc, madctl);

	display.orientation = orientation;
}

void mcugdx_display_show() {
	spi_write_command(spi_handle, dc, 0x2A);
	spi_write_addr(spi_handle, dc, 0, display.width - 1);
	spi_write_command(spi_handle, dc, 0x2B);
	spi_write_addr(spi_handle, dc, 0, display.height - 1);
	spi_write_command(spi_handle, dc, 0x2C);

	uint32_t size = display.width * display.height * 2;
	uint8_t *frame_buffer = (uint8_t *) display.frame_buffer;
	gpio_set_level(dc, 1);
	while (size > 0) {
		uint16_t bs = (size > BUFFER_SIZE) ? BUFFER_SIZE : size;
		spi_write_bytes(spi_handle, frame_buffer, bs);
		size -= bs;
		frame_buffer += bs;
	}
}
