project(inmp441 C)
add_library(inmp441)

pico_generate_pio_header(inmp441 ${CMAKE_CURRENT_SOURCE_DIR}/inmp441.pio OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(inmp441
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/inmp441.c
)

target_include_directories(inmp441 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(inmp441 swapchain pico_stdlib pico_mem_ops hardware_pio hardware_dma)
