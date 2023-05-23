cmake_minimum_required(VERSION 3.25)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(
    -Wall
    -Werror
    -Wno-format
    -Wno-unused-function
    -Wno-maybe-uninitialized
)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(LightPainting)

pico_sdk_init()

add_executable(LightPainting)

file(GLOB PIO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/source/pio/*.pio)
file(GLOB C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/source/*.c)

foreach(PIO_FILE ${PIO_FILES})
    pico_generate_pio_header(LightPainting ${PIO_FILE} OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/pio)
endforeach()

target_sources(LightPainting PRIVATE ${C_FILES})
target_include_directories(LightPainting PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(LightPainting
    pico_stdlib
    pico_mem_ops
    hardware_pio
    hardware_dma
)

pico_add_extra_outputs(LightPainting)
pico_enable_stdio_usb(LightPainting ON)
pico_enable_stdio_uart(LightPainting OFF)