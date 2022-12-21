add_library(inmp441_pio)

file(GLOB PIO_ASM_SRCS ${CMAKE_CURRENT_LIST_DIR}/*.pio)
foreach(PIO_ASM_SRC ${PIO_ASM_SRCS})
    pico_generate_pio_header(inmp441_pio
        ${PIO_ASM_SRC} 
        OUTPUT_DIR 
            ${CMAKE_CURRENT_LIST_DIR}/generated
    )
endforeach()

target_sources(inmp441_pio
    PRIVATE
        driver.c
    INTERFACE
        driver.h
        swapchain.h
)

target_link_libraries(inmp441_pio
    pico_stdlib
    pico_mem_ops
    hardware_pio
    hardware_dma
)