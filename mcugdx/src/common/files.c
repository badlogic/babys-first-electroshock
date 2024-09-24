#include "files.h"
#include "rofs.h"

mcugdx_result_t mcugdx_rofs_init(void) {
	return (mcugdx_result_t) rofs_init();
}

mcugdx_result_t mcugdx_rofs_exists(const char *path) {
	return rofs_exists(path);
}
mcugdx_file_handle_t mcugdx_rofs_open(const char *path) {
	return rofs_open(path);
}

uint32_t mcugdx_rofs_length(mcugdx_file_handle_t handle) {
	return rofs_length(handle);
}

uint32_t mcugdx_rofs_read(mcugdx_file_handle_t handle, uint32_t offset, char *buffer, uint32_t buffer_len) {
	return rofs_read(handle, offset, buffer, buffer_len);
}

uint8_t *mcugdx_rofs_read_file(const char *path, uint32_t *size, mcugdx_memory_type_t mem_type) {
	return rofs_read_file(path, size, mem_type);
}
