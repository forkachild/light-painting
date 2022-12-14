add_library(inmp441_pio)

pico_generate_pio_header(inmp441_pio ${CMAKE_CURRENT_LIST_DIR}/inmp441_driver.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/pio)

target_sources(inmp441_pio
    PRIVATE
        driver.c
        buffer.c
    INTERFACE
        driver.h
        buffer.h
)

target_link_libraries(inmp441_pio
    pico_stdlib
    pico_mem_ops
    hardware_pio
    hardware_dma
)