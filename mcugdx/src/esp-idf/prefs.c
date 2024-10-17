#include "nvs_flash.h"
#include "nvs.h"
#include <string.h>
#include <stdbool.h>
#include "mem.h"
#include "log.h"

#define MAX_KEY_LENGTH 15
#define TAG "mcugdx_prefs"

static nvs_handle_t prefs_handle;

bool mcugdx_prefs_init(const char *namespace) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ret = nvs_flash_erase();
        if (ret != ESP_OK) {
            mcugdx_loge(TAG, "Failed to erase NVS: %s", esp_err_to_name(ret));
            return false;
        }
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        mcugdx_loge(TAG, "Failed to init NVS: %s", esp_err_to_name(ret));
        return false;
    }

    ret = nvs_open(namespace, NVS_READWRITE, &prefs_handle);
    if (ret != ESP_OK) {
        mcugdx_loge(TAG, "Failed to open NVS handle: %s", esp_err_to_name(ret));
        return false;
    }
    return true;
}

bool mcugdx_prefs_write_int(const char *name, int32_t value) {
    esp_err_t ret = nvs_set_i32(prefs_handle, name, value);
    if (ret != ESP_OK) {
        mcugdx_loge(TAG, "Failed to set int value: %s", esp_err_to_name(ret));
        return false;
    }

    ret = nvs_commit(prefs_handle);
    if (ret != ESP_OK) {
        mcugdx_loge(TAG, "Failed to commit changes: %s", esp_err_to_name(ret));
        return false;
    }
    return true;
}

bool mcugdx_prefs_write_string(const char *name, const char *value) {
    esp_err_t ret = nvs_set_str(prefs_handle, name, value);
    if (ret != ESP_OK) {
        mcugdx_loge(TAG, "Failed to set string value: %s", esp_err_to_name(ret));
        return false;
    }

    ret = nvs_commit(prefs_handle);
    if (ret != ESP_OK) {
        mcugdx_loge(TAG, "Failed to commit changes: %s", esp_err_to_name(ret));
        return false;
    }
    return true;
}

bool mcugdx_prefs_read_int(const char *name, int32_t *value) {
    esp_err_t ret = nvs_get_i32(prefs_handle, name, value);
    if (ret != ESP_OK) {
        mcugdx_loge(TAG, "Failed to read int value: %s", esp_err_to_name(ret));
        return false;
    }
    return true;
}

char* mcugdx_prefs_read_string(const char *name) {
    size_t required_size;
    esp_err_t ret = nvs_get_str(prefs_handle, name, NULL, &required_size);
    if (ret != ESP_OK) {
        mcugdx_loge(TAG, "Failed to get string size: %s", esp_err_to_name(ret));
        return NULL;
    }

    char* value = (char*)mcugdx_mem_alloc(required_size, MCUGDX_MEM_INTERNAL);
    if (value == NULL) {
        mcugdx_loge(TAG, "Failed to allocate memory for string");
        return NULL;
    }

    ret = nvs_get_str(prefs_handle, name, value, &required_size);
    if (ret != ESP_OK) {
        mcugdx_loge(TAG, "Failed to read string value: %s", esp_err_to_name(ret));
        mcugdx_mem_free(value);
        return NULL;
    }

    return value;
}