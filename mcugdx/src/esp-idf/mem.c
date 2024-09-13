#include "mem.h"
#include "log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"

#define TAG "mcugdx_mem"

void *mcugdx_mem_alloc(size_t size, mcugdx_memory_type_t mem_type) {
    return heap_caps_malloc(size, mem_type == MCUGDX_MEM_EXTERNAL ? MALLOC_CAP_SPIRAM : MALLOC_CAP_INTERNAL);
}

void mcugdx_mem_free(void *ptr) {
    heap_caps_free(ptr);
}

void mcugdx_print_memory() {
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
        heap_caps_get_free_size(MALLOC_CAP_RETENTION)
    );
}
