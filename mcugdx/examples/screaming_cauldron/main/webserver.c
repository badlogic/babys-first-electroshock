#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "log.h"
#include "files.h"
#include "config.h"
#include "neopixels.h"
#include "cJSON.h"
#include "audio.h"

#define TAG "server"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static EventGroupHandle_t s_wifi_event_group;
static uint8_t *index_html;
static uint32_t index_html_size;

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
							   int32_t event_id, void *event_data) {
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
		esp_wifi_connect();
	} else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
		wifi_event_sta_disconnected_t *event = (wifi_event_sta_disconnected_t *) event_data;
		mcugdx_log(TAG, "WiFi disconnected. Reason: %d", event->reason);
		if (event->reason == WIFI_REASON_AUTH_FAIL) {
			mcugdx_log(TAG, "Authentication failed. Check your WiFi password.");
			xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
		} else {
			esp_wifi_connect();
			mcugdx_log(TAG, "Trying to reconnect...");
		}
	} else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
		mcugdx_log(TAG, "Got IP: http://" IPSTR, IP2STR(&event->ip_info.ip));
		xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	}
}

static esp_err_t root_handler(httpd_req_t *req) {
	httpd_resp_send(req, (const char *) index_html, index_html_size);
	return ESP_OK;
}

static esp_err_t config_get_handler(httpd_req_t *req) {
	config_t *config = config_lock();
	cJSON *json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "deviceName", config->device_name);
	cJSON_AddStringToObject(json, "ssid", config->ssid);
	cJSON_AddStringToObject(json, "password", config->password);
	cJSON_AddNumberToObject(json, "r", config->r);
	cJSON_AddNumberToObject(json, "g", config->g);
	cJSON_AddNumberToObject(json, "b", config->b);
	cJSON_AddNumberToObject(json, "brightness", config->brightness);
	cJSON_AddNumberToObject(json, "volume", config->volume);
	char *json_string = cJSON_Print(json);
	httpd_resp_send(req, json_string, strlen(json_string));
	cJSON_free(json_string);
	cJSON_free(json);
	config_unlock();
	return ESP_OK;
}

static esp_err_t config_save_handler(httpd_req_t *req) {
	esp_err_t ret = ESP_FAIL;

	char *buf = malloc(1024);

	int ret_size = httpd_req_recv(req, buf, 1024);
	if (ret_size <= 0) {
		mcugdx_loge(TAG, "Failed to receive config data");
		free(buf);
		httpd_resp_send_500(req);
		return ESP_FAIL;
	}

	buf[ret_size] = '\0';

	cJSON *root = cJSON_Parse(buf);
	if (root == NULL) {
		mcugdx_loge(TAG, "Failed to parse config JSON");
		free(buf);
		httpd_resp_send_500(req);
		return ESP_FAIL;
	}

	cJSON *device_name = cJSON_GetObjectItem(root, "deviceName");
	cJSON *ssid = cJSON_GetObjectItem(root, "ssid");
	cJSON *password = cJSON_GetObjectItem(root, "password");

	if (cJSON_IsString(device_name) && cJSON_IsString(ssid) && cJSON_IsString(password)) {
		config_t *config = config_lock();
		config->device_name = strdup(device_name->valuestring);
		config->ssid = strdup(ssid->valuestring);
		config->password = strdup(password->valuestring);
		config_unlock();
		config_save();

		httpd_resp_sendstr(req, "Configuration saved successfully");
		mcugdx_log(TAG, "Saved Wifi config, restarting");
		esp_restart();
	} else {
		mcugdx_loge(TAG, "Invalid config data");
		httpd_resp_send_500(req);
		ret = ESP_FAIL;
	}

	cJSON_Delete(root);
	free(buf);

	return ret;
}

static esp_err_t query_param_int(httpd_req_t *req, const char *key, int32_t *out) {
	esp_err_t ret = ESP_FAIL;

	size_t buf_len = httpd_req_get_url_query_len(req) + 1;
	if (buf_len > 1) {
		char *buf = malloc(buf_len);
		if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
			char param[32];
			if (httpd_query_key_value(buf, key, param, sizeof(param)) == ESP_OK) {
				int32_t value = atoi(param);
				*out = value;
				mcugdx_log(TAG, "Processed query parameter %s", key);
				ret = ESP_OK;
			} else {
				mcugdx_loge(TAG, "Invalid query string");
				httpd_resp_send_500(req);
			}
		} else {
			mcugdx_loge(TAG, "Failed to get query parameter %s", key);
			httpd_resp_send_500(req);
		}
		free(buf);
	} else {
		mcugdx_loge(TAG, "Query string too short");
		httpd_resp_send_500(req);
	}

	return ret;
}

static esp_err_t rgb_handler(httpd_req_t *req) {
	config_t *config = config_lock();
	esp_err_t err = query_param_int(req, "r", &config->r);
	if (err != ESP_OK) {
		config_unlock();
		return err;
	}
	err = query_param_int(req, "g", &config->g);
	if (err != ESP_OK) {
		config_unlock();
		return err;
	}
	err = query_param_int(req, "b", &config->b);
	if (err != ESP_OK) {
		config_unlock();
		return err;
	}
	err = query_param_int(req, "br", &config->brightness);
	if (err != ESP_OK) {
		config_unlock();
		return err;
	}
	httpd_resp_sendstr(req, "OK");
	config_unlock();
	config_save();
	return ESP_OK;
}

static esp_err_t volume_handler(httpd_req_t *req) {
	config_t *config = config_lock();
	esp_err_t err = query_param_int(req, "value", &config->volume);
	mcugdx_audio_set_master_volume(config->volume);
	config_unlock();
	config_save();
	if (err != ESP_OK) return err;
	httpd_resp_sendstr(req, "OK");
	return ESP_OK;
}

esp_err_t audio_handler(httpd_req_t *req) {
    const esp_partition_t *partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "rofs");
    if (partition == NULL) {
        return ESP_FAIL;
    }

    uint8_t buffer[512];
    int received;

    esp_partition_erase_range(partition, 0, partition->size);
    size_t offset = 0;

    while ((received = httpd_req_recv(req, (char *)buffer, sizeof(buffer))) > 0) {
        if (esp_partition_write(partition, offset, buffer, received) != ESP_OK) {
			mcugdx_loge(TAG, "Failed to write audio data");
            return ESP_FAIL;
        }
        offset += received;
		mcugdx_log(TAG, "Wrote %li bytes to rofs", offset);
    }

	mcugdx_log(TAG, "Saved Wifi config, restarting");
	esp_restart();

    return (received == 0) ? ESP_OK : ESP_FAIL;
}

void webserver_init_internal(void *) {
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	index_html = mcugdx_rofs.read_fully("index.html", &index_html_size, MCUGDX_MEM_EXTERNAL);
	if (!index_html) {
		mcugdx_loge(TAG, "Could not load index.html from rofs");
	}

	s_wifi_event_group = xEventGroupCreate();
	if (s_wifi_event_group == NULL) {
		mcugdx_loge(TAG, "Failed to create event group");
		return;
	}

	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
														ESP_EVENT_ANY_ID,
														&wifi_event_handler,
														NULL,
														NULL));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
														IP_EVENT_STA_GOT_IP,
														&wifi_event_handler,
														NULL,
														NULL));

	bool failed = false;

	{
		config_t *config = config_lock();
		if (config->ssid != NULL && strlen(config->ssid) > 0 && config->password != NULL && strlen(config->password) > 0) {
			esp_netif_t *itf = esp_netif_create_default_wifi_sta();
			esp_netif_set_hostname(itf, config->device_name);

			wifi_config_t wifi_config = {
					.sta = {
							.threshold.authmode = WIFI_AUTH_WPA2_PSK,
					},
			};
			strlcpy((char *) wifi_config.sta.ssid, config->ssid, sizeof(wifi_config.sta.ssid));
			strlcpy((char *) wifi_config.sta.password, config->password, sizeof(wifi_config.sta.password));

			ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
			ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
			ESP_ERROR_CHECK(esp_wifi_start());

			mcugdx_log(TAG, "Connecting to WiFi...");
			EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
												   WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
												   pdFALSE,
												   pdFALSE,
												   pdMS_TO_TICKS(3000));

			if (bits & WIFI_CONNECTED_BIT) {
				mcugdx_log(TAG, "Connected to WiFi");
				mcugdx_neopixels_set(0, 0, 63, 0);
				mcugdx_neopixels_show();
			} else {
				mcugdx_log(TAG, "Failed to connect to WiFi");
				ESP_ERROR_CHECK(esp_wifi_stop());
				failed = true;
			}
		} else {
			mcugdx_log(TAG, "No WiFi credentials set");
			ESP_ERROR_CHECK(esp_wifi_stop());
			failed = true;
		}
		config_unlock();
	}

	if (failed) {
		vTaskDelete(NULL);
		return;
	}

	httpd_handle_t server = NULL;
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	config.core_id = 1;

	ret = httpd_start(&server, &config);
	if (ret == ESP_OK) {
		httpd_uri_t root = {
				.uri = "/",
				.method = HTTP_GET,
				.handler = root_handler,
				.user_ctx = NULL};
		httpd_register_uri_handler(server, &root);

		httpd_uri_t config_get = {
				.uri = "/config",
				.method = HTTP_GET,
				.handler = config_get_handler,
				.user_ctx = NULL};
		httpd_register_uri_handler(server, &config_get);

		httpd_uri_t config_save = {
				.uri = "/config",
				.method = HTTP_POST,
				.handler = config_save_handler,
				.user_ctx = NULL};
		httpd_register_uri_handler(server, &config_save);

		httpd_uri_t rgb = {
				.uri = "/rgb",
				.method = HTTP_POST,
				.handler = rgb_handler,
				.user_ctx = NULL};
		httpd_register_uri_handler(server, &rgb);

		httpd_uri_t volume = {
				.uri = "/volume",
				.method = HTTP_POST,
				.handler = volume_handler,
				.user_ctx = NULL};
		httpd_register_uri_handler(server, &volume);

		httpd_uri_t audio = {
				.uri = "/audio",
				.method = HTTP_POST,
				.handler = audio_handler,
				.user_ctx = NULL};
		httpd_register_uri_handler(server, &audio);

		mcugdx_log(TAG, "Web server started successfully");
	} else {
		mcugdx_loge(TAG, "Failed to start web server");
	}

	vTaskDelete(NULL);
}

void webserver_init() {
	xTaskCreatePinnedToCore(&webserver_init_internal, "webserver_task", 8192, NULL, 5, NULL, 1);// Run on core 1
}
