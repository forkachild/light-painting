cmake_minimum_required(VERSION 3.25)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(light-painting C CXX ASM)
add_compile_options(
    -Wextra
    -Wall
    -Werror
    -Wno-format
    -Wno-unused-function
    -Wno-maybe-uninitialized
    -Wpointer-arith
    -Wcast-align)
add_link_options(
    -Wextra
    -Wall
    -Werror
    -flto)
add_definitions(
    -DAUDIO_ENVELOPE)

pico_sdk_init()

add_executable(light-painting)

add_subdirectory(util)
add_subdirectory(fft)
add_subdirectory(swapchain)
add_subdirectory(drivers)
add_subdirectory(audio)

target_sources(light-painting
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/main.c)
target_link_libraries(light-painting
    PRIVATE
        i2s
        neopixel
        util
        audio
        swapchain
        pico_stdlib)

pico_add_extra_outputs(light-painting)
pico_enable_stdio_usb(light-painting ON)
pico_enable_stdio_uart(light-painting OFF)