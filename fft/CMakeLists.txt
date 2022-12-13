add_library(fft)

target_sources(fft
    PRIVATE
        fft.c
    INTERFACE
        fft.h
)

target_link_libraries(fft
    pico_stdlib
)
