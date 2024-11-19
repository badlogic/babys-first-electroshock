cmake_minimum_required(VERSION 3.16)

if(DEFINED ESP_PLATFORM)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
endif()

project(blink)

if(NOT DEFINED ESP_PLATFORM)
add_subdirectory(../../ ${CMAKE_BINARY_DIR}/mcugdx)
add_executable(blink "main/main.cpp")
target_link_libraries(blink PUBLIC mcugdx)
endif()