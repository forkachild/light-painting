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

add_executable(LightPainting)

file(GLOB PIO_ASM_SRCS pio/*.pio)
foreach(PIO_ASM_SRC ${PIO_ASM_SRCS})
    pico_generate_pio_header(LightPainting
        ${PIO_ASM_SRC} 
        OUTPUT_DIR 
            include/pio
    )
endforeach()

target_sources(LightPainting
    PRIVATE
        program.c
        ws2812_pio.c
        inmp441_pio.c
        swapchain.c
        canvas.c
        fft.c
)

target_include_directories(LightPainting PUBLIC include)

target_link_libraries(LightPainting
    pico_stdlib
    pico_mem_ops
    hardware_pio
    hardware_dma
)

pico_add_extra_outputs(LightPainting)
pico_enable_stdio_usb(LightPainting 1)
pico_enable_stdio_uart(LightPainting 0)