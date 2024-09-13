#pragma once

#include "result.h"
#include "mem.h"
#include <stdint.h>
#include "files.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t width, height;
    uint16_t *pixels;
} mcugdx_image_t;

mcugdx_image_t *mcugdx_image_load(const char *path, mcugdx_read_file_func_t read_file, mcugdx_memory_type_t mem_type);

#ifdef __cplusplus
}
#endif
