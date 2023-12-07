project(canvas C CXX ASM)
add_library(canvas INTERFACE)

target_include_directories(canvas INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
