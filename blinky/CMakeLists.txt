add_executable(blinky
                program.c
                )

target_link_libraries(blinky
                pico_stdlib
                )

pico_add_extra_outputs(blinky)

pico_enable_stdio_usb(blinky 1)
pico_enable_stdio_uart(blinky 0)