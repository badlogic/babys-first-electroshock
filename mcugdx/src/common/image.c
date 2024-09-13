#include "image.h"
#define QOI_IMPLEMENTATION
#include "qoi.h"

#define reverse_color(color) (((color) >> 8) | ((color) << 8))

mcugdx_image_t *mcugdx_image_load(const char *path, mcugdx_read_file_func_t read_file, mcugdx_memory_type_t mem_type) {
    uint32_t size;
    uint8_t *raw_bytes = read_file(path, &size, mem_type);
    if (!raw_bytes) return NULL;

    qoi_desc desc;
    void *pixels = qoi_decode(raw_bytes, size, &desc, 3);
    if (!pixels) {
        mcugdx_mem_free(raw_bytes);
        return NULL;
    }

    mcugdx_image_t *image = (mcugdx_image_t *)mcugdx_mem_alloc(sizeof(mcugdx_image_t), mem_type);
    if (!image) {
        mcugdx_mem_free(pixels);
        mcugdx_mem_free(raw_bytes);
        return NULL;
    }

    image->width = desc.width;
    image->height = desc.height;

    image->pixels = (uint16_t *)mcugdx_mem_alloc(desc.width * desc.height * sizeof(uint16_t), mem_type);
    if (!image->pixels) {
        mcugdx_mem_free(image);
        mcugdx_mem_free(pixels);
        mcugdx_mem_free(raw_bytes);
        return NULL;
    }

    uint8_t *rgba_pixels = (uint8_t *)pixels;
    for (uint32_t i = 0; i < desc.width * desc.height; i++) {
        uint8_t r = rgba_pixels[i * 3 + 0] >> 3;
        uint8_t g = rgba_pixels[i * 3 + 1] >> 2;
        uint8_t b = rgba_pixels[i * 3 + 2] >> 3;
        image->pixels[i] = reverse_color((r << 11) | (g << 5) | b);
    }

    mcugdx_mem_free(pixels);
    mcugdx_mem_free(raw_bytes);

    return image;
}
