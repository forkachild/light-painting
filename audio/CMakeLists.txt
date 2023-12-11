add_library(audio)

target_sources(audio
    PRIVATE
        audio.c
        )

target_include_directories(audio PUBLIC .)

target_link_libraries(audio fft pico_stdlib)
