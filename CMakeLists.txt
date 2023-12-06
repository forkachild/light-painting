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
project(lightpainting C CXX ASM)
pico_sdk_init()

add_subdirectory(buffer)
add_subdirectory(canvas)
add_subdirectory(fft)
add_subdirectory(status)
add_subdirectory(swapchain)
add_subdirectory(visualization)
add_subdirectory(drivers)

add_executable(lightpainting)

target_sources(lightpainting
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/main.c
)

target_link_libraries(lightpainting
    inmp441
    ws2812
    canvas
    swapchain
    fft
    status
)

pico_add_extra_outputs(lightpainting)
pico_enable_stdio_usb(lightpainting ON)
pico_enable_stdio_uart(lightpainting OFF)