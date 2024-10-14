#pragma once

#include <stdint.h>

bool mcugdx_prefs_init(void);
bool mcugdx_prefs_write_int(const char *collection, const char *name, int32_t value);
bool mcugdx_prefs_write_string(const char *collection, const char *name, const char *value);
bool mcugdx_prefs_read_int(const char *collection, const char *name, int32_t *value);
char* mcugdx_prefs_read_string(const char *collection, const char *name);
