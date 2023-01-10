#pragma once

#include "hardware/pio.h"
#include "types.h"

static inline PIO pio_find(const pio_program_t *program) {
    // Start with PIO0
    PIO pio = pio0;

    // Check if the program can be loaded in the pio
    if (!pio_can_add_program(pio, program)) {
        // Try the next, PIO1
        pio = pio1;

        if (!pio_can_add_program(pio, program))
            // Guard if not
            return NULL;
    }

    return pio;
}