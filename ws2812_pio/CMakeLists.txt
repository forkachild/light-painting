add_library(ws2812_pio STATIC)
                
pico_generate_pio_header(ws2812_pio ${CMAKE_CURRENT_LIST_DIR}/ws2812_driver.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/pio)

target_sources(ws2812_pio
    PRIVATE
        driver.c
    INTERFACE
        driver.h
)

target_link_libraries(ws2812_pio
    pico_stdlib
    pico_mem_ops
    hardware_pio
    hardware_dma
)