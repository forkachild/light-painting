cmake_minimum_required(VERSION 3.25)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/source)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(GENERATED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/generated)

add_compile_options(
    -Wall
    -Werror
    -Wextra
    -Wno-format
    -Wno-unused-function
    -Wno-maybe-uninitialized
)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
project(LightPainting C CXX)
pico_sdk_init()

add_executable(LightPainting)

pico_generate_pio_header(LightPainting ${SOURCE_DIR}/drivers/i2s_mono.pio OUTPUT_DIR ${GENERATED_DIR}/include/drivers)
pico_generate_pio_header(LightPainting ${SOURCE_DIR}/drivers/ws2812.pio OUTPUT_DIR ${GENERATED_DIR}/include/drivers)

target_sources(LightPainting
    PRIVATE
        ${SOURCE_DIR}/main.c
        ${SOURCE_DIR}/drivers/inmp441_pio.c
        ${SOURCE_DIR}/drivers/ws2812_pio.c
        ${SOURCE_DIR}/components/buffer.c
        ${SOURCE_DIR}/components/fft.c
        ${SOURCE_DIR}/components/canvas.c
        ${SOURCE_DIR}/components/visualization.c
        ${SOURCE_DIR}/components/status.c
        ${SOURCE_DIR}/components/swapchain.c)

target_include_directories(LightPainting PUBLIC ${INCLUDE_DIR} ${GENERATED_DIR}/include)

target_link_libraries(LightPainting
    pico_stdlib
    pico_mem_ops
    hardware_pio
    hardware_dma
)

pico_add_extra_outputs(LightPainting)
pico_enable_stdio_usb(LightPainting ON)
pico_enable_stdio_uart(LightPainting OFF)