add_library(neopixel)

pico_generate_pio_header(neopixel ${CMAKE_CURRENT_SOURCE_DIR}/neopixel.pio OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(neopixel
    PRIVATE
        neopixel.c)

target_include_directories(neopixel PUBLIC .)

target_link_libraries(neopixel swapchain pico_stdlib pico_mem_ops hardware_pio hardware_dma)
