#pragma once

#include "result.h"
#include "mem.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t *(*mcugdx_read_file_func_t)(const char *path, uint32_t *size, mcugdx_memory_type_t mem_type);

mcugdx_result_t mcugdx_rofs_init(void);
uint8_t *mcugdx_rofs_read_file(const char *path, uint32_t *size, mcugdx_memory_type_t mem_type);

#ifdef __cplusplus
}
#endif
