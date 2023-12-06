project(canvas C CXX ASM)
add_library(canvas)

target_sources(canvas
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/canvas.c
)

target_include_directories(canvas PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(canvas
    pico_stdlib
)
