add_library(swapchain)

target_sources(swapchain
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/swapchain.c)

target_include_directories(swapchain
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})