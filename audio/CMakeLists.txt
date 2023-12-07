cmake_minimum_required(VERSION 3.25)

project(audio C)
add_library(audio)

target_sources(audio
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/audio.c
)

target_include_directories(audio
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(audio
    PRIVATE
        fft
        pico_stdlib
)