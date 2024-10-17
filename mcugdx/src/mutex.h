#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#include <windows.h>
typedef CRITICAL_SECTION mcugdx_mutex_t;
#elif defined(__APPLE__) || defined(__linux__)
#include <stddef.h>
#include <pthread.h>
typedef pthread_mutex_t mcugdx_mutex_t;
#elif defined(ESP_PLATFORM)
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
typedef SemaphoreHandle_t mcugdx_mutex_t;
#else
#error "Unsupported platform"
#endif

bool mcugdx_mutex_init(mcugdx_mutex_t *mutex);
void mcugdx_mutex_lock(mcugdx_mutex_t *mutex);
void mcugdx_mutex_lock_l(mcugdx_mutex_t *mutex, const char* file, int line);
void mcugdx_mutex_unlock(mcugdx_mutex_t *mutex);
void mcugdx_mutex_unlock_l(mcugdx_mutex_t *mutex, const char *file, int line);
void mcugdx_mutex_destroy(mcugdx_mutex_t *mutex);

#ifdef __cplusplus
}
#endif
