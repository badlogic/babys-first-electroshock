cmake_minimum_required(VERSION 3.16)

if(DEFINED ESP_PLATFORM)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
endif()

project(audio)

if(NOT DEFINED ESP_PLATFORM)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build-desktop)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build-desktop)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build-desktop)
add_subdirectory(../../ ${CMAKE_BINARY_DIR}/mcugdx)

add_executable(display "main/main.cpp")
target_link_libraries(display PUBLIC mcugdx)

endif()