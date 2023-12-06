project(visualization C CXX ASM)
add_library(visualization)

target_sources(visualization
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/visualization.c
)

target_include_directories(visualization PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(visualization
    buffer
    pico_stdlib
)
