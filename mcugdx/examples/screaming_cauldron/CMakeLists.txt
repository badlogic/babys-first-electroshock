cmake_minimum_required(VERSION 3.16)

if(DEFINED ESP_PLATFORM)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
endif()

project(screaming_cauldron)

if(NOT DEFINED ESP_PLATFORM)
add_subdirectory(../../ ${CMAKE_BINARY_DIR}/mcugdx)
add_executable(audio "main/main.cpp")
target_link_libraries(audio PUBLIC mcugdx)
mcugdx_create_rofs_partition(rofs "${CMAKE_CURRENT_SOURCE_DIR}/data/")
endif()