project(ws2812 C)
add_library(ws2812)

pico_generate_pio_header(ws2812 ${CMAKE_CURRENT_SOURCE_DIR}/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(ws2812
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/ws2812.c
)

target_include_directories(ws2812 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(ws2812 swapchain pico_stdlib pico_mem_ops hardware_pio hardware_dma)
