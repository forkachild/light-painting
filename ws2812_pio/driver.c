#include <stdio.h>
#include <stdlib.h>

#include "driver.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"

#include "generated/ws2812_driver_alt.pio.h"

struct WS2812PioDriver {
    // Number of LEDs
    uint count;

    // The PIO block
    PIO pio;

    // The State Machine of the PIO block
    uint pio_sm;

    // Offset inside PIO block codemem
    uint pio_offset;

    // DMA channel used to receive burst data
    int dma_channel;
};

void ws2812_pio_driver_init(WS2812PioDriver **pp_driver, uint count, uint pin,
                            bool dma) {
    PIO pio;
    int pio_sm, dma_channel;
    uint pio_offset;
    dma_channel_config dma_config;
    WS2812PioDriver *driver;

    // Check valid in memory
    if (*pp_driver)
        return;

    // Start with PIO0
    pio = pio0;

    // Check if the program can be loaded in the pio
    if (!pio_can_add_program(pio, &ws2812_program)) {
        // Try the next, PIO1
        pio = pio1;

        if (!pio_can_add_program(pio, &ws2812_program))
            // Guard if not
            return;
    }

    // Try to grab an unused State Machine
    if ((pio_sm = pio_claim_unused_sm(pio, false)) == -1)
        return;

    if (dma && (dma_channel = dma_claim_unused_channel(false)) == -1) {
        // Give up the sm claimed before returning
        pio_sm_unclaim(pio, pio_sm);

        // Guard if not
        return;
    }

    // Load the PIO program in memory and initialize it
    pio_offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, pio_sm, pio_offset, pin);

    driver = malloc(sizeof(WS2812PioDriver));

    driver->pio = pio;
    driver->pio_sm = (uint)pio_sm;
    driver->pio_offset = pio_offset;
    driver->count = count;

    if (dma) {
        // Setup the DMA for data bursts
        dma_config = dma_channel_get_default_config(dma_channel);
        channel_config_set_read_increment(&dma_config, true);
        channel_config_set_write_increment(&dma_config, false);
        channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_32);
        channel_config_set_dreq(&dma_config, pio_get_dreq(pio, pio_sm, true));
        channel_config_set_irq_quiet(&dma_config, true);
        dma_channel_configure(dma_channel, &dma_config, &pio->txf[pio_sm], NULL,
                              count, false);
        driver->dma_channel = dma_channel;
    } else {
        driver->dma_channel = -1;
    }

    // Ensures idempotence
    *pp_driver = driver;
}

uint ws2812_pio_driver_get_count(WS2812PioDriver *p_driver) {
    return p_driver->count;
}

void ws2812_pio_driver_submit_buffer_blocking(WS2812PioDriver *p_driver,
                                              const uint32_t *p_buffer) {
    WS2812PioDriver *driver;

    if (!(driver = p_driver))
        return;

    if (p_driver->dma_channel != -1) {
        dma_channel_set_read_addr(p_driver->dma_channel, p_buffer, true);
        dma_channel_wait_for_finish_blocking(p_driver->dma_channel);
    } else {
        pio_sm_clear_fifos(p_driver->pio, p_driver->pio_sm);
        for (uint i = 0; i < p_driver->count; i++) {
            pio_sm_put_blocking(p_driver->pio, p_driver->pio_sm, p_buffer[i]);
        }
    }
}

void ws2812_pio_driver_deinit(WS2812PioDriver **pp_driver) {
    WS2812PioDriver *driver;

    // Ensure driver is valid in memory
    if (!(driver = *pp_driver))
        return;

    // Let go of the DMA
    if (driver->dma_channel != -1) {
        dma_channel_abort(driver->dma_channel);
        dma_channel_unclaim(driver->dma_channel);
    }

    // PIO ciao
    ws2812_program_deinit(driver->pio, driver->pio_sm);
    // This also unclaims the State Machine
    pio_remove_program(driver->pio, &ws2812_program, driver->pio_offset);

    // Driver is free!
    free(driver);

    // As if nothing ever existed ;-)
    // Ensures idempotence
    *pp_driver = NULL;
}