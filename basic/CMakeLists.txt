add_executable(basic
                program.c
                driver.c
                canvas.c
                status.c
                )

target_link_libraries(basic
                pico_stdlib
                pico_mem_ops
                hardware_spi
                hardware_dma
                )

pico_add_extra_outputs(basic)

pico_enable_stdio_usb(basic 1)
pico_enable_stdio_uart(basic 0)