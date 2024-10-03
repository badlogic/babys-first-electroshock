#pragma once

#include "mem.h"
#include <stdint.h>
#include "files.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint32_t width, height;
	uint16_t *pixels;
	mcugdx_memory_type_t mem_type;
} mcugdx_image_t;

mcugdx_image_t *mcugdx_image_load(const char *path, mcugdx_file_system_t *fs, mcugdx_memory_type_t mem_type);

void mcugdx_image_unload(mcugdx_image_t *image);

#ifdef __cplusplus
}
#endif
