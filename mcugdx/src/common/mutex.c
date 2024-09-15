#include "mutex.h"

mcugdx_result_t mcugdx_mutex_init(mcugdx_mutex_t *mutex) {
#ifdef _WIN32
	InitializeCriticalSection(mutex);
	return MCUGDX_OK;
#elif defined(__APPLE__) || defined(__linux__)
	return pthread_mutex_init(mutex, NULL) ? MCUGDX_ERROR : MCUGDX_OK;
#elif defined(ESP_PLATFORM)
	*mutex = xSemaphoreCreateMutex();
	return (*mutex != NULL) ? MCUGDX_OK : MCUGDX_ERROR;
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

void mcugdx_mutex_destroy(mcugdx_mutex_t *mutex) {
#ifdef _WIN32
	DeleteCriticalSection(mutex);
#elif defined(__APPLE__) || defined(__linux__)
	pthread_mutex_destroy(mutex);
#elif defined(ESP_PLATFORM)
	vSemaphoreDelete(*mutex);
#endif
}
