add_library(ws2812_pio STATIC)
                
file(GLOB PIO_ASM_SRCS ${CMAKE_CURRENT_LIST_DIR}/*.pio)
foreach(PIO_ASM_SRC ${PIO_ASM_SRCS})
    pico_generate_pio_header(ws2812_pio
        ${PIO_ASM_SRC} 
        OUTPUT_DIR 
            ${CMAKE_CURRENT_LIST_DIR}/generated
    )
endforeach()

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