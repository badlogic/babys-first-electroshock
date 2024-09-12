get_cmake_property(cache_vars CACHE_VARIABLES)
foreach (cache_var ${cache_vars})
    message(STATUS "${cache_var} = ${${cache_var}}")
endforeach()

if(DEFINED ESP_PLATFORM)
    idf_component_register(
        SRC_DIRS "src/esp-idf"
        INCLUDE_DIRS "src"
        PRIV_REQUIRES driver esp_timer
    )
else()
cmake_minimum_required(VERSION 3.16)
project(mcugdx)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build-desktop)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build-desktop)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build-desktop)

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

add_library(mcugdx "src/desktop/mcugdx.c" "src/desktop/display.c")
target_include_directories(mcugdx PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(mcugdx PUBLIC minifb)

endif()