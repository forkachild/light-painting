add_library(canvas STATIC)

target_sources(canvas
    PRIVATE
        canvas.c
    INTERFACE
        canvas.h
)

target_link_libraries(canvas
    pico_stdlib
)