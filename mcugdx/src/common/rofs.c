#include "rofs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "mem.h"

#define TAG "mcugdx_rofs"

#ifdef ESP_PLATFORM
#include "esp_partition.h"
#else
#include <limits.h>
#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif
#endif

typedef struct {
    char *name;
    uint32_t offset;
    uint32_t size;
} rofs_file_t;

typedef struct {
    uint32_t num_files;
    rofs_file_t *files;
    uint32_t data_offset;
} rofs_file_system_t;

rofs_file_system_t fs = {0};

#ifdef ESP_PLATFORM
const esp_partition_t *partition;

void read_line(const void *partition, size_t *offset, char *buffer, size_t max_len) {
    char ch;
    size_t i = 0;

    while (i < max_len - 1) {
        esp_partition_read(partition, (*offset)++, &ch, 1);
        if (ch == '\n') {
            break;
        }
        buffer[i++] = ch;
    }

    buffer[i] = '\0';
}

uint8_t *rofs_read_file(const char *path, uint32_t *size, mcugdx_memory_type_t mem_type) {
    for (int i = 0; i < fs.num_files; i++) {
        if (strcmp(fs.files[i].name, path) == 0) {
            uint32_t file_offset = fs.files[i].offset;
            uint32_t file_size = fs.files[i].size;
            uint8_t *copy = (uint8_t *)mcugdx_mem_alloc(file_size, mem_type);
            if (!copy) {
                mcugdx_loge(TAG, "Failed to allocate memory for file %s\n", path);
                return NULL;
            }
            if (esp_partition_read(partition, file_offset, copy, file_size) != ESP_OK) {
                free(copy);
                mcugdx_loge(TAG, "Failed to read file %s\n", path);
                return NULL;
            }

            *size = file_size;
            return copy;
        }
    }

    mcugdx_loge(TAG, "File not found: %s\n", path);
    return NULL;
}

#else
const uint8_t *partition;

void get_executable_dir(char *path, size_t size) {
#if defined(_WIN32)
    GetModuleFileName(NULL, path, size);
    for (int i = strlen(path) - 1; i >= 0; i--) {
        if (path[i] == '\\') {
            path[i] = '\0';
            break;
        }
    }
#elif defined(__linux__)
    ssize_t len = readlink("/proc/self/exe", path, size - 1);
    if (len != -1) {
        path[len] = '\0';
        for (int i = strlen(path) - 1; i >= 0; i--) {
            if (path[i] == '/') {
                path[i] = '\0';
                break;
            }
        }
    }
#elif defined(__APPLE__)
    uint32_t len = size;
    _NSGetExecutablePath(path, &len);
    for (int i = strlen(path) - 1; i >= 0; i--) {
        if (path[i] == '/') {
            path[i] = '\0';
            break;
        }
    }
#endif
}

uint8_t *read_partition_file(void) {
    char executable_dir[1024];
    get_executable_dir(executable_dir, sizeof(executable_dir));

    char rofs_bin_path[1060];
    snprintf(rofs_bin_path, sizeof(rofs_bin_path), "%s/rofs.bin", executable_dir);

    FILE *file = fopen(rofs_bin_path, "rb");
    if (!file) {
        mcugdx_loge(TAG, "Failed to open file: %s\n", rofs_bin_path);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    uint8_t *partition = mcugdx_mem_alloc(file_size, MCUGDX_MEM_INTERNAL);
    if (!partition) {
        mcugdx_loge(TAG, "Failed to allocate memory.\n");
        fclose(file);
        return NULL;
    }

    fread(partition, 1, file_size, file);
    fclose(file);

    return partition;
}

void read_line(const uint8_t *partition, size_t *offset, char *buffer, size_t max_len) {
    char ch;
    size_t i = 0;

    while (i < max_len - 1) {
        ch = partition[(*offset)++];
        if (ch == '\n') {
            break;
        }
        buffer[i++] = ch;
    }

    buffer[i] = '\0';
}

uint8_t *rofs_read_file(const char *path, uint32_t *size, mcugdx_memory_type_t mem_type) {
    (void)mem_type;

    for (uint32_t i = 0; i < fs.num_files; i++) {
        if (strcmp(fs.files[i].name, path) == 0) {
            uint32_t file_offset = fs.files[i].offset;
            uint32_t file_size = fs.files[i].size;
            const uint8_t *data = partition + file_offset;

            uint8_t *copy = (uint8_t *)mcugdx_mem_alloc(file_size, mem_type);
            if (!copy) {
                mcugdx_loge(TAG, "Failed to allocate memory for file\n");
                return NULL;
            }

            memcpy(copy, data, file_size);
            *size = file_size;
            return copy;
        }
    }

    mcugdx_loge(TAG, "File not found: %s\n", path);
    return NULL;
}
#endif

int rofs_init(void) {
#ifdef ESP_PLATFORM
    partition = esp_partition_find_first(
            ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "rofs");
    if (!partition) {
        mcugdx_loge(TAG, "Failed to find rofs partition");
        return 0;
    }
#else
    partition = read_partition_file();
    if (!partition) {
        mcugdx_loge(TAG, "Failed to load rofs.bin");
        return 0;
    }
#endif

    size_t offset = 0;
    char line[256];
    read_line(partition, &offset, line, sizeof(line));
    int num_files = atoi(line);
    fs.num_files = num_files;
    fs.files = mcugdx_mem_alloc(num_files * sizeof(rofs_file_t), MCUGDX_MEM_EXTERNAL);

    for (int i = 0; i < num_files; i++) {
        read_line(partition, &offset, line, sizeof(line));
        size_t file_name_len = strnlen(line, sizeof(line));
        fs.files[i].name = mcugdx_mem_alloc(file_name_len + 1, MCUGDX_MEM_EXTERNAL);
        memcpy(fs.files[i].name, line, file_name_len + 1);

        read_line(partition, &offset, line, sizeof(line));
        fs.files[i].offset = atoi(line);

        read_line(partition, &offset, line, sizeof(line));
        fs.files[i].size = atoi(line);
    }

    for (int i = 0; i < num_files; i++) {
        fs.files[i].offset += offset;
    }

    return 1;
}
