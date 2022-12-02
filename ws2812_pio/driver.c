#include "driver.h"

#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"

#include "generated/ws2812_driver.pio.h"

#define OUTPUT_GPIO 19

static const PIO pio = pio0;
static const uint sm = 0;

struct WS2812PioDriver
{
    uint count;
    uint pioOffset;
    uint dmaChannel;
};

void ws2812_pio_driver_init(WS2812PioDriver **ppDriver, uint count)
{
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_pio_program_init(pio, sm, offset, OUTPUT_GPIO);

    uint channel = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(channel);
    channel_config_set_dreq(&c, pio_get_dreq(pio, sm, true));
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    dma_channel_configure(
        channel,
        &c,
        &pio->txf[sm],
        NULL,
        count,
        false);

    WS2812PioDriver *driver = malloc(sizeof(WS2812PioDriver));

    driver->count = count;
    driver->pioOffset = offset;
    driver->dmaChannel = channel;

    *ppDriver = driver;
}

void ws2812_pio_driver_submit_buffer_blocking(WS2812PioDriver *pDriver, const uint32_t *pBuffer)
{
    dma_channel_set_read_addr(pDriver->dmaChannel, pBuffer, true);
    dma_channel_wait_for_finish_blocking(pDriver->dmaChannel);
    sleep_us(50);
}

void ws2812_pio_driver_deinit(WS2812PioDriver **ppDriver)
{
    WS2812PioDriver *driver = *ppDriver;

    // Let go of the DMA
    dma_channel_abort(driver->dmaChannel);
    dma_channel_unclaim(driver->dmaChannel);

    // PIO ciao
    ws2812_pio_program_deinit(pio, sm);
    pio_remove_program(pio, &ws2812_program, driver->pioOffset);

    // Driver is free!
    free(driver);

    // As if nothing ever existed ;-)
    *ppDriver = NULL;
}