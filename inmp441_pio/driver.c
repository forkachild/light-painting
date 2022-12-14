#include <stdlib.h>

#include "buffer.h"
#include "driver.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"

#include "pio/inmp441_driver.pio.h"

struct INMP441PioDriver {
    PIO pio;
    uint pio_sm;
    uint pio_offset;
    uint dma_channel;
};

void inmp441_pio_driver_init(INMP441PioDriver **pp_driver, uint clk_pin_start,
                             uint data_pin, uint lr_pin) {
    PIO pio;
    int pio_sm, dma_channel;
    uint pio_offset;
    dma_channel_config dma_config;
    INMP441PioDriver *driver;

    if (*pp_driver)
        return;

    // Start with first PIO
    pio = pio0;

    // Find an unused State Machine on PIO0
    if ((pio_sm = pio_claim_unused_sm(pio, false)) == -1) {

        // Search the other PIO
        pio = pio1;

        if ((pio_sm = pio_claim_unused_sm(pio, false)) == -1)

            // Guard if not
            return;
    }

    // Check if the program can be loaded in the PIO
    if (!pio_can_add_program(pio, &inmp441_pio_program)) {
        // Give up the State Machine claimed before returning
        pio_sm_unclaim(pio, pio_sm);

        // Guard if not
        return;
    }

    // Check if an unused dma channel is available
    if ((dma_channel = dma_claim_unused_channel(false)) == -1) {
        // Give up the State Machine claimed before returning
        pio_sm_unclaim(pio, pio_sm);

        // Guard if not
        return;
    }

    // Set LR selection to LOW to get words in L
    gpio_init(lr_pin);
    gpio_set_dir(lr_pin, true);
    gpio_put(lr_pin, false);

    // Load the PIO program in memory and initialize it
    pio_offset = pio_add_program(pio, &inmp441_pio_program);
    inmp441_pio_program_init(pio, pio_sm, pio_offset, clk_pin_start, data_pin);

    // Setup the DMA for data bursts
    dma_config = dma_channel_get_default_config(dma_channel);
    channel_config_set_dreq(&dma_config, pio_get_dreq(pio, pio_sm, false));
    channel_config_set_read_increment(&dma_config, false);
    channel_config_set_write_increment(&dma_config, true);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_32);
    channel_config_set_irq_quiet(&dma_config, true);
    dma_channel_configure(dma_channel, &dma_config, NULL, &pio->rxf[pio_sm], 0,
                          false);

    driver = malloc(sizeof(INMP441PioDriver));

    driver->pio = pio;
    driver->pio_sm = pio_sm;
    driver->pio_offset = pio_offset;
    driver->dma_channel = dma_channel;

    *pp_driver = driver;
}

void inmp441_pio_driver_receive_blocking(INMP441PioDriver *p_driver,
                                         INMP441PioBuffer *p_buffer) {
    dma_channel_set_trans_count(p_driver->dma_channel,
                                inmp441_pio_buffer_get_trans_count(p_buffer),
                                false);
    dma_channel_set_write_addr(p_driver->dma_channel,
                               inmp441_pio_buffer_get_trans_ptr(p_buffer),
                               true);
    dma_channel_wait_for_finish_blocking(p_driver->dma_channel);
}

void inmp441_pio_driver_deinit(INMP441PioDriver **pp_driver) {
    INMP441PioDriver *driver;

    if (!(driver = *pp_driver))
        return;

    if (dma_channel_is_busy(driver->dma_channel))
        dma_channel_wait_for_finish_blocking(driver->dma_channel);
    dma_channel_unclaim(driver->dma_channel);

    inmp441_pio_program_deinit(driver->pio, driver->pio_sm);
    pio_remove_program(driver->pio, &inmp441_pio_program, driver->pio_offset);
    pio_sm_unclaim(driver->pio, driver->pio_sm);

    *pp_driver = NULL;
}
