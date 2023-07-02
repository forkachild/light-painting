#include "drivers/inmp441_pio.h"

#include "components/buffer.h"
#include "drivers/i2s_mono.pio.h"
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
    AsyncBuffer buffer;

    // Written node cached
    AsyncBufferNode *write_node;

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
    async_buffer_producer_submit(&driver.buffer, driver.write_node);
    driver.write_node = async_buffer_producer_obtain(&driver.buffer);
    dma_channel_acknowledge_irq0(driver.dma_channel);
    dma_channel_set_write_addr(driver.dma_channel,
                               async_buffer_node_data_ptr(driver.write_node),
                               true);
}

Result inmp441_init(uint samples, uint sck_pin, uint ws_pin, uint data_pin) {
    PIO pio;
    int pio_sm, dma_channel;
    uint pio_offset;
    AsyncBuffer buffer;
    dma_channel_config dma_config;

    if (driver.is_init)
        return RESULT_ALREADY_INIT;

    // SCK, WS & Data must be
    if (sck_pin >= NUM_BANK0_GPIOS || ws_pin >= NUM_BANK0_GPIOS ||
        data_pin >= NUM_BANK0_GPIOS)
        return RESULT_ARG_ERR;

    // SCK and WS **MUST** be consecutive pins in that order
    if (sck_pin + 1 != ws_pin)
        return RESULT_ARG_ERR;

    // Start with PIO0
    pio = pio0;

    // Check if the program can be loaded in the pio
    if (!pio_can_add_program(pio, &i2s_mono_program)) {
        // Try the next, PIO1
        pio = pio1;

        if (!pio_can_add_program(pio, &i2s_mono_program)) {
            // Guard if not
            return RESULT_PIO_ERR;
        }
    }

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
    pio_offset = pio_add_program(pio, &i2s_mono_program);
    i2s_mono_program_init(pio, pio_sm, pio_offset, sck_pin, ws_pin, data_pin);

    async_buffer_init(&buffer, samples * sizeof(uint32_t));

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
    driver.buffer = buffer;
    driver.is_init = true;

    return RESULT_OK;
}

void inmp441_start_sampling() {
    if (driver.is_transmitting)
        return;

    driver.write_node = async_buffer_producer_obtain(&driver.buffer);
    dma_channel_set_write_addr(driver.dma_channel,
                               async_buffer_node_data_ptr(driver.write_node),
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
    async_buffer_producer_submit(&driver.buffer, driver.write_node);

    driver.is_transmitting = false;
}

AsyncBuffer *inmp441_get_async_buffer() { return &driver.buffer; }

Result inmp441_deinit() {
    // Check if valid in memory
    if (!driver.is_init)
        return RESULT_NOT_INIT;

    inmp441_stop_sampling();
    async_buffer_deinit(&driver.buffer);

    i2s_mono_program_deinit(driver.pio, driver.pio_sm);
    // This also unclaims the State Machine
    pio_remove_program(driver.pio, &i2s_mono_program, driver.pio_offset);

    return RESULT_OK;
}
