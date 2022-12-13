#include <stdlib.h>

#include "driver.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"

#include "generated/ws2812_driver.pio.h"

struct WS2812PioDriver {
    PIO pio;
    uint pio_sm;
    uint pio_offset;
    uint dma_channel;
};

void ws2812_pio_driver_init(WS2812PioDriver **ppDriver, uint pin, uint count) {
    PIO pio;
    int pio_sm, dma_channel;
    uint pio_offset;
    dma_channel_config dma_config;
    WS2812PioDriver *driver;

    pio = pio0;

    // Find an unused sm on PIO0
    if ((pio_sm = pio_claim_unused_sm(pio, false)) == -1) {

        // Search the next, PIO1
        pio = pio1;

        if ((pio_sm = pio_claim_unused_sm(pio, false)) == -1)

            // Guard if not
            return;
    }

    // Check if the program can be loaded in the pio
    if (!pio_can_add_program(pio, &ws2812_pio_program)) {
        // Give up the sm claimed before returning
        pio_sm_unclaim(pio, pio_sm);

        // Guard if not
        return;
    }

    // Check if an unused dma channel is available
    if ((dma_channel = dma_claim_unused_channel(false)) == -1) {
        // Give up the sm claimed before returning
        pio_sm_unclaim(pio, pio_sm);

        // Guard if not
        return;
    }

    // Load the PIO program in memory and initialize it
    pio_offset = pio_add_program(pio, &ws2812_pio_program);
    ws2812_pio_program_init(pio, pio_sm, pio_offset, pin);

    // Setup the DMA for data bursts
    dma_config = dma_channel_get_default_config(dma_channel);
    channel_config_set_dreq(&dma_config, pio_get_dreq(pio, pio_sm, true));
    channel_config_set_read_increment(&dma_config, true);
    channel_config_set_write_increment(&dma_config, false);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_32);
    channel_config_set_irq_quiet(&dma_config, true);
    dma_channel_configure(dma_channel, &dma_config, &pio->txf[pio_sm], NULL,
                          count, false);

    driver = malloc(sizeof(WS2812PioDriver));

    driver->pio = pio;
    driver->pio_sm = pio_sm;
    driver->pio_offset = pio_offset;
    driver->dma_channel = dma_channel;

    *ppDriver = driver;
}

void ws2812_pio_driver_submit_buffer_blocking(WS2812PioDriver *pDriver,
                                              const uint32_t *pBuffer) {
    dma_channel_set_read_addr(pDriver->dma_channel, pBuffer, true);
    dma_channel_wait_for_finish_blocking(pDriver->dma_channel);
    // sleep_us(50);
}

void ws2812_pio_driver_deinit(WS2812PioDriver **ppDriver) {
    WS2812PioDriver *driver = *ppDriver;

    // Let go of the DMA
    dma_channel_abort(driver->dma_channel);
    dma_channel_unclaim(driver->dma_channel);

    // PIO ciao
    ws2812_pio_program_deinit(driver->pio, driver->pio_sm);
    pio_remove_program(driver->pio, &ws2812_pio_program, driver->pio_offset);

    // Driver is free!
    free(driver);

    // As if nothing ever existed ;-)
    *ppDriver = NULL;
}