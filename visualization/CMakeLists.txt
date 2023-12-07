project(visualization C)
add_library(visualization)

target_sources(visualization
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/visualization.c
)

target_include_directories(visualization PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(visualization
    PRIVATE
        fft
        audio
        ws2812
        inmp441
        canvas
        pico_stdlib
)
