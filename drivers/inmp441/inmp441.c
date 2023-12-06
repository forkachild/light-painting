#include "swapchain.h"
#include "inmp441.pio.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include "pico/sync.h"
#include <stdio.h>
#include <stdlib.h>

#define SWAPCHAIN_LENGTH 3

typedef struct {
    // Selected PIO bank
    PIO pio;

    // Selected State Machine in the PIO bank
    uint pio_sm;

    // Offset inside PIO instruction bank
    uint pio_offset;

    // DMA channel used to burst buffers to PIO
    uint dma_channel;

    // GPIO connected to the SCK(Serial ClocK) pin
    uint sck_pin;

    // GPIO connected to the WS(Word Select) pin
    uint ws_pin;

    // GPIO connected to the SD(Serial Data) pin
    uint data_pin;

    // Swapchain used to circle the buffers
    swapchain_context_t *swapchain;

    // Whether the driver is initialized
    bool is_init;

    // Whether transmission is ongoing
    bool is_transmitting;
} INMP441PIODriver;

static INMP441PIODriver driver = {
    .is_init = false,
    .is_transmitting = false,
};

static void dma_irq_handler() {
    swapchain_flip_left(driver.swapchain);
    dma_channel_acknowledge_irq0(driver.dma_channel);
    dma_channel_set_write_addr(driver.dma_channel,
                               swapchain_get_left_buffer(driver.swapchain),
                               true);
}

int inmp441_init(uint samples, uint sck_pin, uint ws_pin, uint data_pin) {
    PIO pio;
    int pio_sm, dma_channel;
    uint pio_offset;
    dma_channel_config dma_config;

    if (driver.is_init)
        return -1;

    // SCK, WS & Data must be
    if (sck_pin >= NUM_BANK0_GPIOS || ws_pin >= NUM_BANK0_GPIOS ||
        data_pin >= NUM_BANK0_GPIOS)
        return -1;

    // SCK and WS **MUST** be consecutive pins in that order
    if (sck_pin + 1 != ws_pin)
        return -1;

    // Start with PIO0
    pio = pio0;

    // Check if the program can be loaded in the pio
    if (!pio_can_add_program(pio, &inmp441_program)) {
        // Try the next, PIO1
        pio = pio1;

        if (!pio_can_add_program(pio, &inmp441_program)) {
            // Guard if not
            return -1;
        }
    }

    // Try to grab an unused State Machine
    if ((pio_sm = pio_claim_unused_sm(pio, false)) == -1)
        return -1;

    // Check if an unused dma channel is available
    if ((dma_channel = dma_claim_unused_channel(false)) == -1) {
        // Give up the State Machine claimed before returning
        pio_sm_unclaim(pio, pio_sm);

        // Guard if not
        return -1;
    }

    // Load the PIO program in memory and initialize it
    pio_offset = pio_add_program(pio, &inmp441_program);
    inmp441_program_init(pio, pio_sm, pio_offset, sck_pin, ws_pin, data_pin);

    swapchain_init(&driver.swapchain, samples * sizeof(uint32_t));

    // Setup the DMA for data bursts
    dma_config = dma_channel_get_default_config(dma_channel);
    channel_config_set_read_increment(&dma_config, false);
    channel_config_set_write_increment(&dma_config, true);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_32);
    channel_config_set_dreq(&dma_config, pio_get_dreq(pio, pio_sm, false));
    channel_config_set_irq_quiet(&dma_config, false);
    dma_channel_configure(dma_channel, &dma_config, NULL, &pio->rxf[pio_sm],
                          samples, false);

    // Setup interrupts
    dma_channel_set_irq0_enabled(dma_channel, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_irq_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    driver.pio = pio;
    driver.pio_sm = (uint)pio_sm;
    driver.pio_offset = pio_offset;
    driver.dma_channel = (uint)dma_channel;
    driver.sck_pin = sck_pin;
    driver.ws_pin = ws_pin;
    driver.data_pin = data_pin;
    driver.is_init = true;

    return 0;
}

void inmp441_start_sampling() {
    if (driver.is_transmitting)
        return;

    dma_channel_set_write_addr(driver.dma_channel,
                               swapchain_get_left_buffer(driver.swapchain),
                               true);

    driver.is_transmitting = true;
}

void inmp441_stop_sampling() {
    if (!driver.is_transmitting)
        return;

    dma_channel_set_irq0_enabled(driver.dma_channel, false);
    dma_channel_abort(driver.dma_channel);
    dma_channel_acknowledge_irq0(driver.dma_channel);
    dma_channel_set_irq0_enabled(driver.dma_channel, true);

    driver.is_transmitting = false;
}

void *inmp441_get_async_buffer() { return swapchain_get_right_buffer(driver.swapchain); }

void inmp441_deinit() {
    // Check if valid in memory
    if (!driver.is_init)
    return;

    inmp441_stop_sampling();
    swapchain_deinit(&driver.swapchain);

    inmp441_program_deinit(driver.pio, driver.pio_sm);
    // This also unclaims the State Machine
    pio_remove_program(driver.pio, &inmp441_program, driver.pio_offset);
}
