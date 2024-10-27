#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char *device_name;
    char *ssid;
    char *password;
    int32_t r, g, b, brightness;
    int32_t volume;
} config_t;

extern config_t config;

void config_read();
void config_print();
void config_save();
config_t *config_lock();
void config_unlock();

#ifdef __cplusplus
}
#endif
