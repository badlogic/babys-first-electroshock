#include <stdbool.h>
#include <stdint.h>
#include "mem.h"
#include "log.h"

#define TAG "mcugdx_prefs"

bool mcugdx_prefs_init(const char *collection) {
    mcugdx_loge(TAG, "not implemented");
    return false;
}


bool mcugdx_prefs_write_int(const char *name, int32_t value) {
    (void)name;
    (void)value;
    mcugdx_loge(TAG, "not implemented");
    return false;
}

bool mcugdx_prefs_write_string(const char *name, const char *value) {
    (void)name;
    (void)value;
    mcugdx_loge(TAG, "not implemented");
    return false;
}

bool mcugdx_prefs_read_int(const char *name, int32_t *value) {
    (void)name;
    (void)value;
    mcugdx_loge(TAG, "not implemented");
    return false;
}

char* mcugdx_prefs_read_string(const char *name) {
    (void)name;
    mcugdx_loge(TAG, "not implemented");
    return NULL;
}
