cmake_minimum_required(VERSION 3.25)

add_library(canvas)

target_sources(canvas
    PRIVATE
        canvas.c)

target_include_directories(canvas PUBLIC .)

target_link_libraries(canvas swapchain pico_stdlib)