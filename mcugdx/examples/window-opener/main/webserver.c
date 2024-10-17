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
#include "prefs.h"
#include "files.h"
#include "motor.h"
#include "bme280.h"
#include "config.h"
#include "cJSON.h"

#define TAG "server"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static EventGroupHandle_t s_wifi_event_group;
static uint8_t *index_html;
static uint32_t index_html_size;

void start_ap_mode(void) {
	wifi_mode_t mode;
	if (esp_wifi_get_mode(&mode) == ESP_ERR_WIFI_NOT_INIT) {
		ESP_ERROR_CHECK(esp_netif_init());
		ESP_ERROR_CHECK(esp_event_loop_create_default());
		wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
		ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	} else {
		ESP_ERROR_CHECK(esp_wifi_stop());
	}

	esp_netif_t *ap_netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
	if (ap_netif == NULL) {
		ap_netif = esp_netif_create_default_wifi_ap();
	}

	wifi_config_t wifi_config = {
			.ap = {
					.ssid = "window-opener",
					.ssid_len = strlen("window-opener"),
					.channel = 1,
					.password = "password",
					.max_connection = 4,
					.authmode = WIFI_AUTH_WPA2_PSK},
	};

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());
	ESP_ERROR_CHECK(esp_wifi_set_max_tx_power(84));

	esp_netif_ip_info_t ip_info;
	esp_netif_get_ip_info(ap_netif, &ip_info);

	mcugdx_log(TAG, "AP Mode started. SSID: %s, Password: %s", wifi_config.ap.ssid, wifi_config.ap.password);
	mcugdx_log(TAG, "AP IP address: " IPSTR, IP2STR(&ip_info.ip));
	mcugdx_log(TAG, "AP Netmask: " IPSTR, IP2STR(&ip_info.netmask));
	mcugdx_log(TAG, "AP Gateway: " IPSTR, IP2STR(&ip_info.gw));
	mcugdx_log(TAG, "Server should be accessible at http://" IPSTR, IP2STR(&ip_info.ip));
}

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
		mcugdx_log(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
		xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	}
}

static esp_err_t root_handler(httpd_req_t *req) {
	httpd_resp_send(req, (const char *) index_html, index_html_size);
	return ESP_OK;
}

static esp_err_t motor_toggle_handler(httpd_req_t *req) {
	motor_toggle();
	httpd_resp_send(req, NULL, 0);
	return ESP_OK;
}

static esp_err_t state_handler(httpd_req_t *req) {
	bool is_closed = motor_is_closed();
	motor_state_t state = motor_state();
	const char *state_str = NULL;
	switch (state) {
		case MOTOR_CLOSING:
			state_str = "closing";
			break;
		case MOTOR_OPENING:
			state_str = "opening";
			break;
		default:
			state_str = "idle";
	}

	cJSON *json = cJSON_CreateObject();
	cJSON_AddBoolToObject(json, "isClosed", is_closed);
	cJSON_AddStringToObject(json, "state", state_str);
	cJSON_AddNumberToObject(json, "temperature", bme280_temperature());
	cJSON_AddNumberToObject(json, "pressure", bme280_temperature());
	cJSON_AddNumberToObject(json, "humidity", bme280_humidity());
	config_t *config = config_lock();
	cJSON_AddBoolToObject(json, "manual", config->manual);
	config_unlock();
	char *json_string = cJSON_Print(json);
	httpd_resp_send(req, json_string, strlen(json_string));
	cJSON_free(json_string);
	cJSON_free(json);
	return ESP_OK;
}

static esp_err_t config_get_handler(httpd_req_t *req) {
	config_t *config = config_lock();
	cJSON *json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "deviceName", config->device_name);
	cJSON_AddStringToObject(json, "ssid", config->ssid);
	cJSON_AddStringToObject(json, "password", config->password);
	cJSON_AddNumberToObject(json, "minTemp", config->min_temp);
	cJSON_AddNumberToObject(json, "maxTemp", config->max_temp);
	cJSON_AddBoolToObject(json, "manual", config->manual);
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

static esp_err_t min_temp_handler(httpd_req_t *req) {
	esp_err_t ret = ESP_FAIL;

	size_t buf_len = httpd_req_get_url_query_len(req) + 1;
	if (buf_len > 1) {
		char *buf = malloc(buf_len);
		if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
			char param[32];
			if (httpd_query_key_value(buf, "value", param, sizeof(param)) == ESP_OK) {
				float new_min_temp = atof(param);
				config_t *config = config_lock();
				config->min_temp = new_min_temp;
				config_unlock();
				config_save();
				httpd_resp_sendstr(req, "Minimum temperature updated successfully");
				ret = ESP_OK;
			} else {
				mcugdx_loge(TAG, "Invalid query string");
				httpd_resp_send_500(req);
			}
		} else {
			mcugdx_loge(TAG, "Failed to get query string");
			httpd_resp_send_500(req);
		}
		free(buf);
	} else {
		mcugdx_loge(TAG, "Query string too short");
		httpd_resp_send_500(req);
	}

	return ret;
}

static esp_err_t max_temp_handler(httpd_req_t *req) {
	esp_err_t ret = ESP_FAIL;

	size_t buf_len = httpd_req_get_url_query_len(req) + 1;
	if (buf_len > 1) {
		char *buf = malloc(buf_len);
		if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
			char param[32];
			if (httpd_query_key_value(buf, "value", param, sizeof(param)) == ESP_OK) {
				float new_max_temp = atof(param);
				config_t *config = config_lock();
				config->max_temp = new_max_temp;
				config_unlock();
				config_save();
				httpd_resp_sendstr(req, "Minimum temperature updated successfully");
				ret = ESP_OK;
			} else {
				mcugdx_loge(TAG, "Invalid query string");
				httpd_resp_send_500(req);
			}
		} else {
			mcugdx_loge(TAG, "Failed to get query string");
			httpd_resp_send_500(req);
		}
		free(buf);
	} else {
		mcugdx_loge(TAG, "Query string too short");
		httpd_resp_send_500(req);
	}

	return ret;
}

static esp_err_t manual_handler(httpd_req_t *req) {
	esp_err_t ret = ESP_FAIL;

	size_t buf_len = httpd_req_get_url_query_len(req) + 1;
	if (buf_len > 1) {
		char *buf = malloc(buf_len);
		if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
			char param[32];
			if (httpd_query_key_value(buf, "value", param, sizeof(param)) == ESP_OK) {
				bool manual = atof(param) != 0;
				config_t *config = config_lock();
				config->manual = manual;
				config_unlock();
				config_save();
				httpd_resp_sendstr(req, "Manual updated successfully");
				ret = ESP_OK;
			} else {
				mcugdx_loge(TAG, "Invalid query string");
				httpd_resp_send_500(req);
			}
		} else {
			mcugdx_loge(TAG, "Failed to get query string");
			httpd_resp_send_500(req);
		}
		free(buf);
	} else {
		mcugdx_loge(TAG, "Query string too short");
		httpd_resp_send_500(req);
	}

	return ret;
}

void webserver_init(void) {
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	mcugdx_rofs_init();
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
												   pdMS_TO_TICKS(10000));

			if (bits & WIFI_CONNECTED_BIT) {
				mcugdx_log(TAG, "Connected to WiFi");
			} else {
				mcugdx_log(TAG, "Failed to connect to WiFi, starting AP mode");
				ESP_ERROR_CHECK(esp_wifi_stop());
				start_ap_mode();
			}
		} else {
			mcugdx_log(TAG, "No WiFi credentials set, starting AP mode");
			start_ap_mode();
		}
		config_unlock();
	}

	httpd_handle_t server = NULL;
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();

	ret = httpd_start(&server, &config);
	if (ret == ESP_OK) {
		httpd_uri_t root = {
				.uri = "/",
				.method = HTTP_GET,
				.handler = root_handler,
				.user_ctx = NULL};
		httpd_register_uri_handler(server, &root);

		httpd_uri_t toggle = {
				.uri = "/toggle",
				.method = HTTP_POST,
				.handler = motor_toggle_handler,
				.user_ctx = NULL};
		httpd_register_uri_handler(server, &toggle);

		httpd_uri_t state = {
				.uri = "/state",
				.method = HTTP_GET,
				.handler = state_handler,
				.user_ctx = NULL};
		httpd_register_uri_handler(server, &state);

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

		httpd_uri_t min_temp = {
				.uri = "/min_temp",
				.method = HTTP_POST,
				.handler = min_temp_handler,
				.user_ctx = NULL};
		httpd_register_uri_handler(server, &min_temp);

		httpd_uri_t max_temp = {
				.uri = "/max_temp",
				.method = HTTP_POST,
				.handler = max_temp_handler,
				.user_ctx = NULL};
		httpd_register_uri_handler(server, &max_temp);

		httpd_uri_t manual = {
				.uri = "/manual",
				.method = HTTP_POST,
				.handler = manual_handler,
				.user_ctx = NULL};
		httpd_register_uri_handler(server, &manual);


		mcugdx_log(TAG, "Web server started successfully");
	} else {
		mcugdx_loge(TAG, "Failed to start web server");
	}
}
