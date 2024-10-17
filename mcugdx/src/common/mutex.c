#include "mutex.h"
#include "log.h"
#include <stdint.h>

#define TAG "mutext"

bool mcugdx_mutex_init(mcugdx_mutex_t *mutex) {
#ifdef _WIN32
	InitializeCriticalSection(mutex);
	return true;
#elif defined(__APPLE__) || defined(__linux__)
	return pthread_mutex_init(mutex, 0) ? false : true;
#elif defined(ESP_PLATFORM)
	*mutex = xSemaphoreCreateMutex();
	return (*mutex != NULL) ? true : false;
#endif
}

void mcugdx_mutex_lock(mcugdx_mutex_t *mutex) {
#ifdef _WIN32
	EnterCriticalSection(mutex);
#elif defined(__APPLE__) || defined(__linux__)
	pthread_mutex_lock(mutex);
#elif defined(ESP_PLATFORM)
	xSemaphoreTake(*mutex, portMAX_DELAY);
#endif
}

void mcugdx_mutex_unlock(mcugdx_mutex_t *mutex) {
#ifdef _WIN32
	LeaveCriticalSection(mutex);
#elif defined(__APPLE__) || defined(__linux__)
	pthread_mutex_unlock(mutex);
#elif defined(ESP_PLATFORM)
	xSemaphoreGive(*mutex);
#endif
}

void mcugdx_mutex_lock_l(mcugdx_mutex_t *mutex, const char* file, int line) {
	// mcugdx_log(TAG, "Locking %p, %s:%i", mutex, file, line);
#ifdef _WIN32
	EnterCriticalSection(mutex);
#elif defined(__APPLE__) || defined(__linux__)
	pthread_mutex_lock(mutex);
#elif defined(ESP_PLATFORM)
	xSemaphoreTake(*mutex, portMAX_DELAY);
#endif
}

void mcugdx_mutex_unlock_l(mcugdx_mutex_t *mutex, const char *file, int line) {
	// mcugdx_log(TAG, "Unlocking %p, %s:%i", mutex, file, line);
#ifdef _WIN32
	LeaveCriticalSection(mutex);
#elif defined(__APPLE__) || defined(__linux__)
	pthread_mutex_unlock(mutex);
#elif defined(ESP_PLATFORM)
	xSemaphoreGive(*mutex);
#endif
}

void mcugdx_mutex_destroy(mcugdx_mutex_t *mutex) {
#ifdef _WIN32
	DeleteCriticalSection(mutex);
#elif defined(__APPLE__) || defined(__linux__)
	pthread_mutex_destroy(mutex);
#elif defined(ESP_PLATFORM)
	vSemaphoreDelete(*mutex);
#endif
}
