#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "mem.h"
#include "log.h"

#ifdef ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#endif

#define TAG "mcugdx_mem"

// Use 64-bit for size and type on 64-bit platforms, 32-bit on 32-bit platforms
#if UINTPTR_MAX == 0xffffffff
typedef uint32_t mem_info_t;
#define ALIGNMENT 4
#else
typedef uint64_t mem_info_t;
#define ALIGNMENT 8
#endif

#define MEM_TYPE_MASK ((mem_info_t) 1 << (sizeof(mem_info_t) * 8 - 1))
#define SIZE_MASK (~MEM_TYPE_MASK)

size_t internal_mem = 0;
size_t external_mem = 0;

void *mcugdx_mem_alloc(size_t size, mcugdx_memory_type_t mem_type) {
	size_t total_size = size + sizeof(mem_info_t);

#ifdef ESP_PLATFORM
	mem_info_t *info = (mem_info_t *) heap_caps_malloc(total_size, mem_type == MCUGDX_MEM_EXTERNAL ? MALLOC_CAP_SPIRAM : MALLOC_CAP_INTERNAL);
#else
	mem_info_t *info = (mem_info_t *) malloc(total_size);
#endif
	if (info == NULL) {
		return NULL;// Out of memory
	}

	*info = (total_size & SIZE_MASK) | (mem_type == MCUGDX_MEM_INTERNAL ? 0 : MEM_TYPE_MASK);

	if (mem_type == MCUGDX_MEM_INTERNAL) {
		internal_mem += total_size;
	} else {
		external_mem += total_size;
	}

	return (void *) (info + 1);// Return pointer to the aligned data area
}

void mcugdx_mem_free(void *ptr) {
	if (ptr == NULL) {
		return;
	}

	mem_info_t *info = ((mem_info_t *) ptr) - 1;
	size_t total_size = *info & SIZE_MASK;
	mcugdx_memory_type_t mem_type = (*info & MEM_TYPE_MASK) ? MCUGDX_MEM_EXTERNAL : MCUGDX_MEM_INTERNAL;

	if (mem_type == MCUGDX_MEM_INTERNAL) {
		internal_mem -= total_size;
	} else {
		external_mem -= total_size;
	}

#ifdef ESP_PLATFORM
	heap_caps_free(info);
#else
	free(info);
#endif
}

size_t mcugdx_mem_internal_usage(void) {
	return internal_mem;
}

size_t mcugdx_mem_external_usage(void) {
	return external_mem;
}

void mcugdx_mem_print(void) {
	mcugdx_log(TAG, "internal: %i, external %i\n", internal_mem, external_mem);
#ifdef ESP_PLATFORM
	mcugdx_log(
			TAG,
			"Free Heap: %u bytes\n"
			"  MALLOC_CAP_8BIT      %7zu bytes\n"
			"  MALLOC_CAP_DMA       %7zu bytes\n"
			"  MALLOC_CAP_SPIRAM    %7zu bytes\n"
			"  MALLOC_CAP_INTERNAL  %7zu bytes\n"
			"  MALLOC_CAP_DEFAULT   %7zu bytes\n"
			"  MALLOC_CAP_IRAM_8BIT %7zu bytes\n"
			"  MALLOC_CAP_RETENTION %7zu bytes\n",
			xPortGetFreeHeapSize(),
			heap_caps_get_free_size(MALLOC_CAP_8BIT),
			heap_caps_get_free_size(MALLOC_CAP_DMA),
			heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
			heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
			heap_caps_get_free_size(MALLOC_CAP_DEFAULT),
			heap_caps_get_free_size(MALLOC_CAP_IRAM_8BIT),
			heap_caps_get_free_size(MALLOC_CAP_RETENTION));
#endif
}
