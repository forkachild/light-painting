project(common C)

add_subdirectory(buffer)

add_library(common INTERFACE)

target_include_directories(common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
