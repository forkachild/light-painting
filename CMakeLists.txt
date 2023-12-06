cmake_minimum_required(VERSION 3.25)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(
    -Wall
    -Werror
    -Wextra
    -Wno-format
    -Wno-unused-function
    -Wno-maybe-uninitialized
)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
project(light-painting C CXX ASM)
pico_sdk_init()

add_subdirectory(canvas)
add_subdirectory(fft)
add_subdirectory(status)
add_subdirectory(swapchain)
add_subdirectory(drivers)

add_executable(light-painting)

target_sources(light-painting
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/main.c
)

target_link_libraries(light-painting
    inmp441
    ws2812
    canvas
    swapchain
    fft
    status
)

pico_add_extra_outputs(light-painting)
pico_enable_stdio_usb(light-painting ON)
pico_enable_stdio_uart(light-painting OFF)