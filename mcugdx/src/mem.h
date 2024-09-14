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

void mcugdx_mem_print(void);

size_t mcugdx_mem_internal_usage(void);

size_t mcugdx_mem_get_external_usage(void);

#ifdef __cplusplus
}
#endif
