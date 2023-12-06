project(status C CXX ASM)
add_library(status)

target_sources(status
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/status.c
)

target_include_directories(status PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(status
    pico_stdlib
)
