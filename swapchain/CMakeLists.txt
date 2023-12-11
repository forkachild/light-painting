add_library(swapchain)

target_sources(swapchain
    PRIVATE
        swapchain.c)

target_include_directories(swapchain PUBLIC .)