#include "inmp441_pio.h"

#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include "pico/sync.h"
#include "pio/i2s_driver.pio.h"
#include "swapchain.h"
#include "utils.h"
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
    Swapchain swapchain;

    // Written node cached
    SwapchainNode *write_node;

    // Whether the driver is initialized
    bool is_init;

    // Whether transmission is ongoing
    bool is_transmitting;
} INMP441PIODriver;

static INMP441PIODriver driver = {
    .write_node = NULL,
    .is_init = false,
    .is_transmitting = false,
};

static void dma_irq_handler() {
    swapchain_return_after_write(&driver.swapchain, driver.write_node);
    driver.write_node = swapchain_try_borrow_for_write(&driver.swapchain);
    dma_channel_acknowledge_irq0(driver.dma_channel);
    dma_channel_set_write_addr(driver.dma_channel,
                               swapchain_node_get_buffer_ptr(driver.write_node),
                               true);
}

Result inmp441_init(uint samples, uint sck_pin, uint ws_pin, uint data_pin) {
    PIO pio;
    int pio_sm, dma_channel;
    uint pio_offset;
    Swapchain swapchain;
    dma_channel_config dma_config;

    if (driver.is_init)
        return RESULT_ALREADY_INIT;

    // SCK, WS & Data must be
    if (sck_pin >= NUM_BANK0_GPIOS || ws_pin >= NUM_BANK0_GPIOS ||
        data_pin >= NUM_BANK0_GPIOS)
        return RESULT_ARG_ERR;

    // SCK and WS **MUST** be consecutive pins
    if (sck_pin + 1 != ws_pin)
        return RESULT_ARG_ERR;

    // Start with first PIO
    if (!(pio = pio_find(&inmp441_program)))
        return RESULT_PIO_ERR;

    // Try to grab an unused State Machine
    if ((pio_sm = pio_claim_unused_sm(pio, false)) == -1)
        return RESULT_PIO_ERR;

    // Check if an unused dma channel is available
    if ((dma_channel = dma_claim_unused_channel(false)) == -1) {
        // Give up the State Machine claimed before returning
        pio_sm_unclaim(pio, pio_sm);

        // Guard if not
        return RESULT_DMA_ERR;
    }

    // Load the PIO program in memory and initialize it
    pio_offset = pio_add_program(pio, &inmp441_program);
    inmp441_program_init(pio, pio_sm, pio_offset, sck_pin, ws_pin, data_pin);

    swapchain_init(&swapchain, samples * sizeof(uint32_t), SWAPCHAIN_LENGTH);

    // Setup the DMA for data bursts
    dma_config = dma_channel_get_default_config(dma_channel);
    channel_config_set_read_increment(&dma_config, false);
    channel_config_set_write_increment(&dma_config, true);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_32);
    // channel_config_set_bswap(&dma_config, true);
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
    driver.swapchain = swapchain;
    driver.is_init = true;

    return RESULT_ALL_OK;
}

void inmp441_start_sampling() {
    if (driver.is_transmitting)
        return;

    driver.write_node = swapchain_try_borrow_for_write(&driver.swapchain);
    dma_channel_set_write_addr(driver.dma_channel,
                               swapchain_node_get_buffer_ptr(driver.write_node),
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

Swapchain *inmp441_get_swapchain() { return &driver.swapchain; }

Result inmp441_deinit() {
    // Check if valid in memory
    if (!driver.is_init)
        return RESULT_NOT_INIT;

    inmp441_stop_sampling();
    swapchain_deinit(&driver.swapchain);

    inmp441_program_deinit(driver.pio, driver.pio_sm);
    // This also unclaims the State Machine
    pio_remove_program(driver.pio, &inmp441_program, driver.pio_offset);

    return RESULT_ALL_OK;
}
