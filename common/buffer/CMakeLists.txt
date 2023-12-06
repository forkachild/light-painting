project(buffer C)
add_library(buffer)

target_sources(buffer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/buffer.c
)

target_include_directories(buffer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(buffer
    pico_stdlib
)
