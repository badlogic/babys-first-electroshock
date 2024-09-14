#include "image.h"

#define reverse_color(color) (((color) >> 8) | ((color) << 8))

#define QOI_IMPLEMENTATION
#include "qoi.h"

mcugdx_image_t *mcugdx_image_load(const char *path, mcugdx_read_file_func_t read_file, mcugdx_memory_type_t mem_type) {
	uint32_t size;
	uint8_t *raw_bytes = read_file(path, &size, mem_type);
	if (!raw_bytes) return NULL;

	qoi_desc desc;
	void *pixels = qoi_decode(raw_bytes, size, &desc, 3, mem_type);
	if (!pixels) {
		mcugdx_mem_free(raw_bytes);
		return NULL;
	}

	mcugdx_image_t *image = (mcugdx_image_t *) mcugdx_mem_alloc(sizeof(mcugdx_image_t), mem_type);
	if (!image) {
		mcugdx_mem_free(pixels);
		mcugdx_mem_free(raw_bytes);
		return NULL;
	}

	image->width = desc.width;
	image->height = desc.height;
	image->pixels = pixels;

	mcugdx_mem_free(raw_bytes);

	return image;
}

void mcugdx_image_unload(mcugdx_image_t *image) {
	mcugdx_mem_free(image->pixels);
	mcugdx_mem_free(image);
}
