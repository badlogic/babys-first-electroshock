#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef enum {
    MCUGDX_MEM_INTERNAL,
    MCUGDX_MEM_EXTERNAL
} mcugdx_memory_type_t;

void *mcugdx_mem_alloc(size_t size, mcugdx_memory_type_t mem_type);

void mcugdx_mem_free(void *ptr);

void mcugdx_print_memory(void);

#ifdef __cplusplus
}
#endif
