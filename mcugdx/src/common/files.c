#include "files.h"
#include "rofs.h"

mcugdx_result_t mcugdx_rofs_init(void) {
	return (mcugdx_result_t) rofs_init();
}

mcugdx_result_t mcugdx_rofs_length(const char *path);

uint8_t *mcugdx_rofs_read_file(const char *path, uint32_t *size, mcugdx_memory_type_t mem_type) {
	return rofs_read_file(path, size, mem_type);
}
