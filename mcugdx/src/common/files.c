#include "files.h"
#include "rofs.h"

bool mcugdx_rofs_init(void) {
	return rofs_init();
}

static void rofs_close(mcugdx_file_handle_t handle) {
	(void)handle;
	return;
}

mcugdx_file_system_t mcugdx_rofs = {
	.exists = rofs_exists,
	.length = rofs_length,
	.open = rofs_open,
	.close = rofs_close,
	.read = rofs_read,
	.read_fully = rofs_read_fully,
};
