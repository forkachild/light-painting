#include <stdio.h>
#include <stdlib.h>

#include "driver.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include "pico/sync.h"
#include "swapchain.h"

#include "generated/inmp441_driver.pio.h"

#define SWAPCHAIN_LENGTH 3

struct INMP441PioDriver {
    PIO pio;
    uint pio_sm;
    uint pio_offset;
    uint dma_channel;
    uint sck_pin;
    uint ws_pin;
    uint data_pin;
    uint lr_config_pin;
    INMP441Swapchain swapchain;
    INMP441SwapchainNode *last_write_node;
};

static INMP441PioDriver *driver = NULL;

void dma_irq_handler() {
    // uint32_t save = save_and_disable_interrupts();
    swapchain_return_after_write(&driver->swapchain, driver->last_write_node);
    driver->last_write_node = swapchain_borrow_for_write(&driver->swapchain);

    // restore_interrupts(save);

    dma_hw->ints0 = 1u << driver->dma_channel;
    dma_channel_set_write_addr(driver->dma_channel,
                               swapchain_node_get_ptr(driver->last_write_node),
                               true);
}

void inmp441_driver_init(uint samples, uint sck_pin, uint ws_pin, uint data_pin,
                         uint lr_config_pin) {
    PIO pio;
    int pio_sm, dma_channel;
    uint pio_offset;
    INMP441Swapchain swapchain;
    dma_channel_config dma_config;

    if (driver)
        return;

    if (sck_pin >= NUM_BANK0_GPIOS || ws_pin >= NUM_BANK0_GPIOS ||
        data_pin >= NUM_BANK0_GPIOS || lr_config_pin >= NUM_BANK0_GPIOS)
        return;

    if (sck_pin + 1 != ws_pin)
        return;

    // Start with first PIO
    pio = pio0;

    // Check if the program can be loaded in the pio
    if (!pio_can_add_program(pio, &INMP441_program)) {
        // Try the other, PIO1
        pio = pio1;

        // Check again
        if (!pio_can_add_program(pio, &INMP441_program))
            // Guard if not
            return;
    }

    // Try to grab an unused State Machine
    if ((pio_sm = pio_claim_unused_sm(pio, false)) == -1)
        return;

    // Check if an unused dma channel is available
    if ((dma_channel = dma_claim_unused_channel(false)) == -1) {
        // Give up the State Machine claimed before returning
        pio_sm_unclaim(pio, pio_sm);

        // Guard if not
        return;
    }

    // Set LR selection to LOW to get words in L
    gpio_init(lr_config_pin);
    gpio_set_dir(lr_config_pin, true);
    gpio_put(lr_config_pin, false);

    // Load the PIO program in memory and initialize it
    pio_offset = pio_add_program(pio, &INMP441_program);
    INMP441_program_init(pio, pio_sm, pio_offset, sck_pin, ws_pin, data_pin);

    swapchain_init(&swapchain, samples, SWAPCHAIN_LENGTH);

    // Setup the DMA for data bursts
    dma_config = dma_channel_get_default_config(dma_channel);

    channel_config_set_read_increment(&dma_config, false);
    channel_config_set_write_increment(&dma_config, true);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_32);
    channel_config_set_dreq(&dma_config, pio_get_dreq(pio, pio_sm, false));
    channel_config_set_irq_quiet(&dma_config, false);

    INMP441SwapchainNode *write_node = swapchain_borrow_for_write(&swapchain);
    dma_channel_configure(dma_channel, &dma_config,
                          swapchain_node_get_ptr(write_node), &pio->rxf[pio_sm],
                          samples, false);

    dma_channel_set_irq0_enabled(dma_channel, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_irq_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    driver = malloc(sizeof(INMP441PioDriver));

    driver->pio = pio;
    driver->pio_sm = (uint)pio_sm;
    driver->pio_offset = pio_offset;
    driver->dma_channel = (uint)dma_channel;
    driver->sck_pin = sck_pin;
    driver->ws_pin = ws_pin;
    driver->data_pin = data_pin;
    driver->lr_config_pin = lr_config_pin;
    driver->swapchain = swapchain;
    driver->last_write_node = write_node;
}

void inmp441_driver_start_sampling() { dma_channel_start(driver->dma_channel); }

void inmp441_driver_stop_sampling() {
    // disable the channel on IRQ0
    dma_channel_set_irq0_enabled(driver->dma_channel, false);
    // abort the channel
    dma_channel_abort(driver->dma_channel);
    // clear the spurious IRQ (if there was one)
    dma_channel_acknowledge_irq0(driver->dma_channel);
    // re-enable the channel on IRQ0
    dma_channel_set_irq0_enabled(driver->dma_channel, true);
}

INMP441Swapchain *inmp441_driver_get_swapchain() { return &driver->swapchain; }

void inmp441_driver_deinit() {
    // Check if valid in memory
    if (!driver)
        return;

    swapchain_deinit(&driver->swapchain);

    if (dma_channel_is_busy(driver->dma_channel))
        dma_channel_wait_for_finish_blocking(driver->dma_channel);
    dma_channel_abort(driver->dma_channel);
    dma_channel_unclaim(driver->dma_channel);

    INMP441_program_deinit(driver->pio, driver->pio_sm);
    // This also unclaims the State Machine
    pio_remove_program(driver->pio, &INMP441_program, driver->pio_offset);

    gpio_deinit(driver->lr_config_pin);

    driver = NULL;
}
