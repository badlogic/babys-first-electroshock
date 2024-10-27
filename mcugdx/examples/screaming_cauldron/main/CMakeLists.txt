idf_component_register(SRC_DIRS "." INCLUDE_DIRS "." REQUIRES driver esp_http_server esp_wifi json mcugdx)

mcugdx_create_rofs_partition(rofs "${CMAKE_CURRENT_SOURCE_DIR}/../data/")