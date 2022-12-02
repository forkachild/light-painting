add_library(ws2812_spi STATIC)

target_sources(ws2812_spi
    PRIVATE
        driver.c
    INTERFACE
        driver.h
)

target_link_libraries(ws2812_spi
    pico_stdlib
    pico_mem_ops
    hardware_spi
    hardware_dma
)