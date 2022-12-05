cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(LightPainting C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_compile_options(
    -Wall
    -Wno-format
    -Wno-unused-function
    -Wno-maybe-uninitialized
)

add_subdirectory(status)
add_subdirectory(canvas)
add_subdirectory(ws2812_spi)
add_subdirectory(ws2812_pio)

add_executable(LightPainting)

target_sources(LightPainting
    PRIVATE
        program.c
)

target_link_libraries(LightPainting
    pico_stdlib
    ws2812_pio
    canvas
    status
)

pico_add_extra_outputs(LightPainting)
pico_enable_stdio_usb(LightPainting 1)
pico_enable_stdio_uart(LightPainting 0)