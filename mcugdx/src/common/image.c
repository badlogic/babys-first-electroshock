#include "image.h"
#define QOI_IMPLEMENTATION
#include "qoi.h"

#define reverse_color(color) (((color) >> 8) | ((color) << 8))

mcugdx_image_t *mcugdx_image_load(const char *path, mcugdx_read_file_func_t read_file, mcugdx_memory_type_t mem_type) {
    uint32_t size;
    uint8_t *raw_bytes = read_file(path, &size, mem_type);
    if (!raw_bytes) return NULL;

    // Decode the QOI image
    qoi_desc desc;
    void *pixels = qoi_decode(raw_bytes, size, &desc, 3);
    if (!pixels) {
        mcugdx_mem_free(raw_bytes);  // Free raw bytes if decoding fails
        return NULL;
    }

    // Allocate memory for mcugdx_image_t
    mcugdx_image_t *image = (mcugdx_image_t *)mcugdx_mem_alloc(sizeof(mcugdx_image_t), mem_type);
    if (!image) {
        mcugdx_mem_free(pixels);     // Free pixels if memory allocation fails
        mcugdx_mem_free(raw_bytes);
        return NULL;
    }

    image->width = desc.width;
    image->height = desc.height;

    // Allocate memory for the pixel buffer
    image->pixels = (uint16_t *)mcugdx_mem_alloc(desc.width * desc.height * sizeof(uint16_t), mem_type);
    if (!image->pixels) {
        mcugdx_mem_free(image);
        mcugdx_mem_free(pixels);
        mcugdx_mem_free(raw_bytes);
        return NULL;
    }

    // Convert RGBA to 16-bit format and store in image->pixels
    uint8_t *rgba_pixels = (uint8_t *)pixels;
    for (uint32_t i = 0; i < desc.width * desc.height; i++) {
        uint8_t r = rgba_pixels[i * 3 + 0] >> 3; // 5-bit red
        uint8_t g = rgba_pixels[i * 3 + 1] >> 2; // 6-bit green
        uint8_t b = rgba_pixels[i * 3 + 2] >> 3; // 5-bit blue
        image->pixels[i] = reverse_color((r << 11) | (g << 5) | b);
    }

    // Free the original raw bytes and QOI decoded buffer
    mcugdx_mem_free(pixels);
    mcugdx_mem_free(raw_bytes);

    return image;
}
