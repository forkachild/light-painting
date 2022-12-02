add_library(status STATIC)

target_sources(status
    PRIVATE
        status.c
    INTERFACE
        status.h
)

target_link_libraries(status
    pico_stdlib
)