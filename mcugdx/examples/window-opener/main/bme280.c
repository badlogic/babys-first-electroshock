#include <stdio.h>
#include "mcugdx.h"
#include "driver/i2c.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define I2C_MASTER_NUM I2C_NUM_0 // I2C port number for master dev
#define I2C_MASTER_FREQ_HZ 100000// I2C master clock frequency
#define BME280_SENSOR_ADDR 0x76  // BME280 sensor address

static const char *TAG = "bme280";

// BME280 registers
#define BME280_REGISTER_DIG_T1 0x88
#define BME280_REGISTER_DIG_T2 0x8A
#define BME280_REGISTER_DIG_T3 0x8C
#define BME280_REGISTER_DIG_P1 0x8E
#define BME280_REGISTER_DIG_P2 0x90
#define BME280_REGISTER_DIG_P3 0x92
#define BME280_REGISTER_DIG_P4 0x94
#define BME280_REGISTER_DIG_P5 0x96
#define BME280_REGISTER_DIG_P6 0x98
#define BME280_REGISTER_DIG_P7 0x9A
#define BME280_REGISTER_DIG_P8 0x9C
#define BME280_REGISTER_DIG_P9 0x9E
#define BME280_REGISTER_DIG_H1 0xA1
#define BME280_REGISTER_DIG_H2 0xE1
#define BME280_REGISTER_DIG_H3 0xE3
#define BME280_REGISTER_DIG_H4 0xE4
#define BME280_REGISTER_DIG_H5 0xE5
#define BME280_REGISTER_DIG_H6 0xE7
#define BME280_REGISTER_CHIPID 0xD0
#define BME280_REGISTER_VERSION 0xD1
#define BME280_REGISTER_SOFTRESET 0xE0
#define BME280_REGISTER_CONTROL_HUM 0xF2
#define BME280_REGISTER_CONTROL 0xF4
#define BME280_REGISTER_CONFIG 0xF5
#define BME280_REGISTER_PRESSUREDATA 0xF7
#define BME280_REGISTER_TEMPDATA 0xFA
#define BME280_REGISTER_HUMIDDATA 0xFD

static struct {
	uint16_t dig_T1;
	int16_t dig_T2;
	int16_t dig_T3;
	uint16_t dig_P1;
	int16_t dig_P2;
	int16_t dig_P3;
	int16_t dig_P4;
	int16_t dig_P5;
	int16_t dig_P6;
	int16_t dig_P7;
	int16_t dig_P8;
	int16_t dig_P9;
	uint8_t dig_H1;
	int16_t dig_H2;
	uint8_t dig_H3;
	int16_t dig_H4;
	int16_t dig_H5;
	int8_t dig_H6;
} bme280_calib;

static int32_t t_fine;
static float temperature;
static float pressure;
static float humidity;

static mcugdx_mutex_t mutex;

static esp_err_t i2c_master_init(int sclk, int sda) {
	i2c_config_t conf = {
			.mode = I2C_MODE_MASTER,
			.sda_io_num = sda,
			.scl_io_num = sclk,
			.sda_pullup_en = GPIO_PULLUP_ENABLE,
			.scl_pullup_en = GPIO_PULLUP_ENABLE,
			.master = {
					.clk_speed = I2C_MASTER_FREQ_HZ},
			.clk_flags = 0};

	esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
	if (err != ESP_OK) {
		return err;
	}
	return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

static esp_err_t bme280_read_registers(uint8_t reg, uint8_t *data, size_t len) {
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (BME280_SENSOR_ADDR << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, reg, true);
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (BME280_SENSOR_ADDR << 1) | I2C_MASTER_READ, true);
	if (len > 1) {
		i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);
	}
	i2c_master_read_byte(cmd, data + len - 1, I2C_MASTER_NACK);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
	return ret;
}

static esp_err_t bme280_write_register(uint8_t reg, uint8_t value) {
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (BME280_SENSOR_ADDR << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, reg, true);
	i2c_master_write_byte(cmd, value, true);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
	return ret;
}

static esp_err_t bme280_read_calibration_data() {
	esp_err_t ret;
	uint8_t data[26];

	ret = bme280_read_registers(BME280_REGISTER_DIG_T1, data, 26);
	if (ret != ESP_OK) return ret;

	bme280_calib.dig_T1 = (data[1] << 8) | data[0];
	bme280_calib.dig_T2 = (data[3] << 8) | data[2];
	bme280_calib.dig_T3 = (data[5] << 8) | data[4];
	bme280_calib.dig_P1 = (data[7] << 8) | data[6];
	bme280_calib.dig_P2 = (data[9] << 8) | data[8];
	bme280_calib.dig_P3 = (data[11] << 8) | data[10];
	bme280_calib.dig_P4 = (data[13] << 8) | data[12];
	bme280_calib.dig_P5 = (data[15] << 8) | data[14];
	bme280_calib.dig_P6 = (data[17] << 8) | data[16];
	bme280_calib.dig_P7 = (data[19] << 8) | data[18];
	bme280_calib.dig_P8 = (data[21] << 8) | data[20];
	bme280_calib.dig_P9 = (data[23] << 8) | data[22];
	bme280_calib.dig_H1 = data[25];

	ret = bme280_read_registers(BME280_REGISTER_DIG_H2, data, 7);
	if (ret != ESP_OK) return ret;

	bme280_calib.dig_H2 = (data[1] << 8) | data[0];
	bme280_calib.dig_H3 = data[2];
	bme280_calib.dig_H4 = (data[3] << 4) | (data[4] & 0x0F);
	bme280_calib.dig_H5 = (data[5] << 4) | (data[4] >> 4);
	bme280_calib.dig_H6 = data[6];

	return ESP_OK;
}

bool bme280_init(int sclk, int sda) {
	esp_err_t ret;

	// Initialize mutex
	mcugdx_mutex_init(&mutex);

	// Initialize I2C
	ret = i2c_master_init(sclk, sda);
	if (ret != ESP_OK) {
		mcugdx_loge(TAG, "Failed to initialize I2C");
		return false;
	}

	// Check BME280 chip ID
	uint8_t chip_id;
	ret = bme280_read_registers(BME280_REGISTER_CHIPID, &chip_id, 1);
	if (ret != ESP_OK || chip_id != 0x60) {
		mcugdx_loge(TAG, "Failed to read BME280 chip ID or invalid ID");
		return false;
	}

	// Read calibration data
	ret = bme280_read_calibration_data();
	if (ret != ESP_OK) {
		mcugdx_loge(TAG, "Failed to read calibration data");
		return false;
	}

	// Set BME280 to normal mode with oversampling
	bme280_write_register(BME280_REGISTER_CONTROL_HUM, 0x01);// Humidity oversampling x1
	bme280_write_register(BME280_REGISTER_CONTROL, 0x27);    // Normal mode, temp and pressure oversampling x1

	mcugdx_log(TAG, "BME280 initialized successfully");
	return true;
}

bool bme280_update() {
	mcugdx_mutex_lock_l(&mutex, __FILE__, __LINE__);
	uint8_t data[8];
	esp_err_t ret = bme280_read_registers(BME280_REGISTER_PRESSUREDATA, data, 8);
	if (ret != ESP_OK) {
		mcugdx_loge(TAG, "Failed to read sensor data");
		mcugdx_mutex_unlock_l(&mutex, __FILE__, __LINE__);
		return false;
	}
	int32_t adc_T = (data[3] << 12) | (data[4] << 4) | (data[5] >> 4);
	int32_t adc_P = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);
	int32_t adc_H = (data[6] << 8) | data[7];

	// Temperature calculation
	int32_t var1 = ((((adc_T >> 3) - ((int32_t) bme280_calib.dig_T1 << 1))) * ((int32_t) bme280_calib.dig_T2)) >> 11;
	int32_t var2 = (((((adc_T >> 4) - ((int32_t) bme280_calib.dig_T1)) * ((adc_T >> 4) - ((int32_t) bme280_calib.dig_T1))) >> 12) * ((int32_t) bme280_calib.dig_T3)) >> 14;
	t_fine = var1 + var2;
	temperature = (t_fine * 5 + 128) >> 8;
	temperature /= 100.0f;

	// Pressure calculation
	int64_t var1_p = ((int64_t) t_fine) - 128000;
	int64_t var2_p = var1_p * var1_p * (int64_t) bme280_calib.dig_P6;
	var2_p = var2_p + ((var1_p * (int64_t) bme280_calib.dig_P5) << 17);
	var2_p = var2_p + (((int64_t) bme280_calib.dig_P4) << 35);
	var1_p = ((var1_p * var1_p * (int64_t) bme280_calib.dig_P3) >> 8) + ((var1_p * (int64_t) bme280_calib.dig_P2) << 12);
	var1_p = (((((int64_t) 1) << 47) + var1_p)) * ((int64_t) bme280_calib.dig_P1) >> 33;
	if (var1_p == 0) {
		pressure = 0;
	} else {
		int64_t p = 1048576 - adc_P;
		p = (((p << 31) - var2_p) * 3125) / var1_p;
		var1_p = (((int64_t) bme280_calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
		var2_p = (((int64_t) bme280_calib.dig_P8) * p) >> 19;
		p = ((p + var1_p + var2_p) >> 8) + (((int64_t) bme280_calib.dig_P7) << 4);
		pressure = (float) p / 256.0f;
	}

	// Humidity calculation
	int32_t v_x1_u32r = (t_fine - ((int32_t) 76800));
	v_x1_u32r = (((((adc_H << 14) - (((int32_t) bme280_calib.dig_H4) << 20) - (((int32_t) bme280_calib.dig_H5) * v_x1_u32r)) + ((int32_t) 16384)) >> 15) * (((((((v_x1_u32r * ((int32_t) bme280_calib.dig_H6)) >> 10) * (((v_x1_u32r * ((int32_t) bme280_calib.dig_H3)) >> 11) + ((int32_t) 32768))) >> 10) + ((int32_t) 2097152)) * ((int32_t) bme280_calib.dig_H2) + 8192) >> 14));
	v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t) bme280_calib.dig_H1)) >> 4));
	v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
	v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
	humidity = (float) (v_x1_u32r >> 12) / 1024.0f;

	mcugdx_mutex_unlock_l(&mutex, __FILE__, __LINE__);
	return true;
}

float bme280_temperature() {
	mcugdx_mutex_lock_l(&mutex, __FILE__, __LINE__);
	float value = temperature;
	mcugdx_mutex_unlock_l(&mutex, __FILE__, __LINE__);
	return value;
}

float bme280_pressure() {
	mcugdx_mutex_lock_l(&mutex, __FILE__, __LINE__);
	float value = pressure;
	mcugdx_mutex_unlock_l(&mutex, __FILE__, __LINE__);
	return value;
}

float bme280_humidity() {
	mcugdx_mutex_lock_l(&mutex, __FILE__, __LINE__);
	float value = humidity;
	mcugdx_mutex_unlock_l(&mutex, __FILE__, __LINE__);
	return value;
}