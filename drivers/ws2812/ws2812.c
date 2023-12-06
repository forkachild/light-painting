#include "ws2812.h"
#include "buffer.h"
#include "ws2812.pio.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    // Number of LEDs
    uint count;

    // The PIO block
    PIO pio;

    // The State Machine of the PIO block
    uint pio_sm;

    // Offset inside PIO block codemem
    uint pio_offset;

    // DMA channel used to receive burst data
    uint dma_channel;

    // The swapchain to use
    AsyncBuffer buffer;

    // The node of the swapchain read last
    AsyncBufferNode *read_node;

    // Whether the driver is initialized
    bool is_init;

    // Whether the driver is transmitting
    bool is_transmitting;
} WS2812PIODriver;

static WS2812PIODriver driver = {
    .read_node = NULL,
    .is_init = false,
    .is_transmitting = false,
};

static void dma_irq_handler() {
    async_buffer_consumer_submit(&driver.buffer, driver.read_node);
    driver.read_node = async_buffer_consumer_obtain(&driver.buffer);
    dma_channel_acknowledge_irq1(driver.dma_channel);
    pio_sm_exec(driver.pio, driver.pio_sm,
                pio_encode_jmp(driver.pio_offset + ws2812_offset_sync));
    dma_channel_set_read_addr(
        driver.dma_channel, async_buffer_node_data_ptr(driver.read_node), true);
}

Result ws2812_init(uint count, uint pin) {
    PIO pio;
    int pio_sm, dma_channel;
    uint pio_offset;
    dma_channel_config dma_config;

    if (driver.is_init)
        return RESULT_ALREADY_INIT;

    // Start with PIO0
    pio = pio0;

    // Check if the program can be loaded in the pio
    if (!pio_can_add_program(pio, &ws2812_program)) {
        // Try the next, PIO1
        pio = pio1;

        if (!pio_can_add_program(pio, &ws2812_program)) {
            // Guard if not
            return RESULT_PIO_ERR;
        }
    }

    // Try to grab an unused State Machine
    if ((pio_sm = pio_claim_unused_sm(pio, false)) == -1) {
        return RESULT_PIO_ERR;
    }

    if ((dma_channel = dma_claim_unused_channel(false)) == -1) {
        pio_sm_unclaim(pio, pio_sm);
        return RESULT_DMA_ERR;
    }

    // Load the PIO program in memory and initialize it
    pio_offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, pio_sm, pio_offset, pin);

    // Setup the DMA for data bursts
    dma_config = dma_channel_get_default_config(dma_channel);
    channel_config_set_read_increment(&dma_config, true);
    channel_config_set_write_increment(&dma_config, false);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_32);
    channel_config_set_dreq(&dma_config, pio_get_dreq(pio, pio_sm, true));
    channel_config_set_irq_quiet(&dma_config, false);
    dma_channel_configure(dma_channel, &dma_config, &pio->txf[pio_sm], NULL,
                          count, false);

    dma_channel_set_irq1_enabled(dma_channel, true);
    irq_set_exclusive_handler(DMA_IRQ_1, dma_irq_handler);
    irq_set_enabled(DMA_IRQ_1, true);

    async_buffer_init(&driver.buffer, count * sizeof(uint32_t));

    driver.pio = pio;
    driver.pio_sm = (uint)pio_sm;
    driver.pio_offset = pio_offset;
    driver.count = count;
    driver.dma_channel = (uint)dma_channel;
    driver.is_init = true;

    return RESULT_OK;
}

bool ws2812_is_init() { return driver.is_init; }

void ws2812_start_transmission() {
    if (driver.is_transmitting)
        return;

    driver.read_node = async_buffer_consumer_obtain(&driver.buffer);
    dma_channel_set_read_addr(
        driver.dma_channel, async_buffer_node_data_ptr(driver.read_node), true);
    driver.is_transmitting = true;
}

void ws2812_stop_transmission() {
    if (!driver.is_transmitting)
        return;

    dma_channel_set_irq1_enabled(driver.dma_channel, false);
    dma_channel_abort(driver.dma_channel);
    dma_channel_acknowledge_irq1(driver.dma_channel);
    dma_channel_set_irq1_enabled(driver.dma_channel, true);
    async_buffer_consumer_submit(&driver.buffer, driver.read_node);

    driver.is_transmitting = false;
}

AsyncBuffer *ws2812_get_async_buffer() { return &driver.buffer; }

Result ws2812_deinit() {
    if (!driver.is_init)
        return RESULT_NOT_INIT;

    ws2812_stop_transmission();

    async_buffer_deinit(&driver.buffer);

    // PIO ciao
    ws2812_program_deinit(driver.pio, driver.pio_sm);
    // This also unclaims the State Machine
    pio_remove_program(driver.pio, &ws2812_program, driver.pio_offset);

    driver.is_init = false;

    return RESULT_OK;
}