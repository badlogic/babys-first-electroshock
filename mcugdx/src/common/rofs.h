#pragma once

#include "mem.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t rofs_file_handle_t;

bool rofs_init(void);

bool rofs_exists(const char *path);

rofs_file_handle_t rofs_open(const char *path);

uint32_t rofs_length(rofs_file_handle_t handle);

uint32_t rofs_read(rofs_file_handle_t handle, uint32_t file_offset, uint8_t *buffer, uint32_t buffer_len);

uint8_t *rofs_read_fully(const char *path, uint32_t *size, mcugdx_memory_type_t mem_type);

#ifdef __cplusplus
}
#endif
