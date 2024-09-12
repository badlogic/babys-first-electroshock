idf_component_register(SRC_DIRS "." INCLUDE_DIRS "." REQUIRES driver mcugdx)

mcugdx_create_rofs_partition(rofs "${CMAKE_CURRENT_SOURCE_DIR}/../data/")