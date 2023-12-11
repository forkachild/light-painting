project(fft C)
add_library(fft)

target_sources(fft
    PRIVATE
        fft.c)

target_include_directories(fft PUBLIC .)

target_link_libraries(fft pico_stdlib)
