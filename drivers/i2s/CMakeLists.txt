add_library(i2s)

pico_generate_pio_header(i2s
    ${CMAKE_CURRENT_SOURCE_DIR}/i2s.pio
    OUTPUT_DIR
        ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(i2s
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/i2s.c)

target_include_directories(i2s PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(i2s swapchain pico_stdlib pico_mem_ops hardware_pio hardware_dma)
