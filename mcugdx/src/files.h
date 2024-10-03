#pragma once

#include "mem.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t mcugdx_file_handle_t;

typedef bool (*mcugdx_fs_init_func_t)(void);
typedef bool (*mcugdx_fs_exists_func_t)(const char *path);
typedef mcugdx_file_handle_t (*mcugdx_fs_open_func_t)(const char *path);
typedef void (*mcugdx_fs_close_func_t)(mcugdx_file_handle_t);
typedef uint32_t (*mcugdx_fs_length_func_t)(mcugdx_file_handle_t handle);
typedef uint32_t (*mcugdx_fs_read_func_t)(mcugdx_file_handle_t handle, uint32_t offset, uint8_t *buffer, uint32_t buffer_len);
typedef uint8_t *(*mcugdx_fs_read_fully_func_t)(const char *path, uint32_t *size, mcugdx_memory_type_t mem_type);

typedef struct {
    mcugdx_fs_exists_func_t exists;
    mcugdx_fs_open_func_t open;
    mcugdx_fs_close_func_t close;
    mcugdx_fs_length_func_t length;
    mcugdx_fs_read_func_t read;
    mcugdx_fs_read_fully_func_t read_fully;
} mcugdx_file_system_t;

bool mcugdx_rofs_init(void);
extern mcugdx_file_system_t mcugdx_rofs;

#ifdef __cplusplus
}
#endif
