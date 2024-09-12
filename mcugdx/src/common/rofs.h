#pragma once

#include "mem.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

int rofs_init(void);

uint8_t *rofs_read_file(const char *path, uint32_t *size, mcugdx_memory_type_t mem_type);

#ifdef __cplusplus
}
#endif
