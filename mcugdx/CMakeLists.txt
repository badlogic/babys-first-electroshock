if(DEFINED ESP_PLATFORM)
    idf_component_register(
        SRC_DIRS "src/common" "src/esp-idf"
        INCLUDE_DIRS "src"
        PRIV_REQUIRES driver esp_timer esp_partition
    )
else()
cmake_minimum_required(VERSION 3.16)
project(mcugdx)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

include(FetchContent)
FetchContent_Declare(minifb GIT_REPOSITORY https://github.com/emoon/minifb)
set(MINIFB_BUILD_EXAMPLES CACHE INTERNAL FALSE)
FetchContent_MakeAvailable(minifb)

if(CMAKE_BUILD_TYPE MATCHES Release)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result)
    if(result)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

add_compile_options(-DDESKTOP_BUILD -Wall -Wextra -Wpedantic -Werror -Wno-implicit-fallthrough)

# Library
file(GLOB COMMON_SOURCES "src/common/*.c")
file(GLOB DESKTOP_SOURCES "src/desktop/*.c")
add_library(mcugdx  ${COMMON_SOURCES} ${DESKTOP_SOURCES})
target_include_directories(mcugdx PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(mcugdx PUBLIC minifb)

if(APPLE)
    find_library(AUDIOTOOLBOX AudioToolbox)
    if(NOT AUDIOTOOLBOX)
        message(FATAL_ERROR "AudioToolbox framework not found")
    endif()
    target_link_libraries(mcugdx PUBLIC ${AUDIOTOOLBOX})
endif()

# Tools

# CMake extensions
include("${CMAKE_CURRENT_SOURCE_DIR}/project_include.cmake")

endif()