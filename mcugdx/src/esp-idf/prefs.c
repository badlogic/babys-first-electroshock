#include "nvs_flash.h"
#include "nvs.h"
#include <string.h>
#include <stdbool.h>
#include "mem.h"
#include "log.h"

#define MAX_KEY_LENGTH 15
#define TAG "MCUGDX_PREFS"

static nvs_handle_t prefs_handle;

bool mcugdx_prefs_init(void) {
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

    ret = nvs_open("storage", NVS_READWRITE, &prefs_handle);
    if (ret != ESP_OK) {
        mcugdx_loge(TAG, "Failed to open NVS handle: %s", esp_err_to_name(ret));
        return false;
    }
    return true;
}

bool mcugdx_prefs_write_int(const char *collection, const char *name, int32_t value) {
    char key[MAX_KEY_LENGTH + 1];
    snprintf(key, sizeof(key), "%s.%s", collection, name);

    esp_err_t ret = nvs_set_i32(prefs_handle, key, value);
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

bool mcugdx_prefs_write_string(const char *collection, const char *name, const char *value) {
    char key[MAX_KEY_LENGTH + 1];
    snprintf(key, sizeof(key), "%s.%s", collection, name);

    esp_err_t ret = nvs_set_str(prefs_handle, key, value);
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

bool mcugdx_prefs_read_int(const char *collection, const char *name, int32_t *value) {
    char key[MAX_KEY_LENGTH + 1];
    snprintf(key, sizeof(key), "%s.%s", collection, name);

    esp_err_t ret = nvs_get_i32(prefs_handle, key, value);
    if (ret != ESP_OK) {
        mcugdx_loge(TAG, "Failed to read int value: %s", esp_err_to_name(ret));
        return false;
    }
    return true;
}

char* mcugdx_prefs_read_string(const char *collection, const char *name) {
    char key[MAX_KEY_LENGTH + 1];
    snprintf(key, sizeof(key), "%s.%s", collection, name);

    size_t required_size;
    esp_err_t ret = nvs_get_str(prefs_handle, key, NULL, &required_size);
    if (ret != ESP_OK) {
        mcugdx_loge(TAG, "Failed to get string size: %s", esp_err_to_name(ret));
        return NULL;
    }

    char* value = (char*)mcugdx_mem_alloc(required_size, MCUGDX_MEM_INTERNAL);
    if (value == NULL) {
        mcugdx_loge(TAG, "Failed to allocate memory for string");
        return NULL;
    }

    ret = nvs_get_str(prefs_handle, key, value, &required_size);
    if (ret != ESP_OK) {
        mcugdx_loge(TAG, "Failed to read string value: %s", esp_err_to_name(ret));
        mcugdx_mem_free(value);
        return NULL;
    }

    return value;
}