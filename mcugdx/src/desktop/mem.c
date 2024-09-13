#include "mem.h"
#include "log.h"
#include <stdlib.h>

#define TAG "mcugdx_mem"

void *mcugdx_mem_alloc(size_t size, mcugdx_memory_type_t mem_type) {
    (void)mem_type;
    return malloc(size);
}

void mcugdx_mem_free(void *ptr) {
    free(ptr);
}

void mcugdx_print_memory(void) {
    mcugdx_log(TAG, "mcugdx_print_memory: not implemented on desktop\n");
}
