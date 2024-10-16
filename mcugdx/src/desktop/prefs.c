#include <stdbool.h>
#include <stdint.h>
#include "mem.h"
#include "log.h"

#define TAG "mcugdx_prefs"

bool mcugdx_prefs_init(void) {
    mcugdx_loge(TAG, "not implemented");
    return false;
}


bool mcugdx_prefs_write_int(const char *collection, const char *name, int32_t value) {
    (void)collection;
    (void)name;
    (void)value;
    mcugdx_loge(TAG, "not implemented");
    return false;
}

bool mcugdx_prefs_write_string(const char *collection, const char *name, const char *value) {
    (void)collection;
    (void)name;
    (void)value;
    mcugdx_loge(TAG, "not implemented");
    return false;
}

bool mcugdx_prefs_read_int(const char *collection, const char *name, int32_t *value) {
    (void)collection;
    (void)name;
    (void)value;
    mcugdx_loge(TAG, "not implemented");
    return false;
}

char* mcugdx_prefs_read_string(const char *collection, const char *name) {
    (void)collection;
    (void)name;
    mcugdx_loge(TAG, "not implemented");
    return NULL;
}
