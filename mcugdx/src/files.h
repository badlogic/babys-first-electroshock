#pragma once

#include "result.h"
#include "mem.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t mcugdx_file_handle_t;

typedef uint8_t *(*mcugdx_read_file_func_t)(const char *path, uint32_t *size, mcugdx_memory_type_t mem_type);

mcugdx_result_t mcugdx_rofs_init(void);
mcugdx_result_t mcugdx_rofs_exists(const char *path);
mcugdx_file_handle_t mcugdx_rofs_open(const char *path);
uint32_t mcugdx_rofs_length(mcugdx_file_handle_t handle);
uint32_t mcugdx_rofs_read(mcugdx_file_handle_t handle, uint32_t offset, char *buffer, uint32_t buffer_len);
uint8_t *mcugdx_rofs_read_file(const char *path, uint32_t *size, mcugdx_memory_type_t mem_type);

#ifdef __cplusplus
}
#endif
