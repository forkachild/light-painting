#include <stdlib.h>

#include "driver.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"

#include "generated/inmp441_driver.pio.h"

#ifdef RX_FIFO_JOIN_TX
#define RX_FIFO_SIZE 8
#else
#define RX_FIFO_SIZE 4
#endif

struct INMP441PioDriver {
    PIO pio;
    uint pio_sm;
    uint pio_offset;
    uint dma_channel;
};

struct INMP441PioDriverBuffer {
    uint count;
    uint32_t *p_padded_buffer;
};

static inline uint32_t *
inpm441_pio_driver_buffer_get_ptr(INMP441PioDriverBuffer *p_buffer_ptr) {
    return p_buffer_ptr->p_padded_buffer;
}

void inmp441_pio_driver_init(INMP441PioDriver **pp_driver, uint ctrl_pin_start,
                             uint data_pin) {
    PIO pio;
    int pio_sm, dma_channel;
    uint pio_offset;
    dma_channel_config dma_config;
    INMP441PioDriver *driver;

    if (*pp_driver)
        return;

    pio = pio0;
    if ((pio_sm = pio_claim_unused_sm(pio, false)) == -1) {
        pio = pio1;
        if ((pio_sm = pio_claim_unused_sm(pio, false)) == -1)
            return;
    }

    if (!pio_can_add_program(pio, &inmp441_pio_program)) {
        pio_sm_unclaim(pio, pio_sm);
        return;
    }

    if ((dma_channel = dma_claim_unused_channel(false)) == -1) {
        pio_sm_unclaim(pio, pio_sm);
        return;
    }

    pio_offset = pio_add_program(pio, &inmp441_pio_program);
    inmp441_pio_program_init(pio, pio_sm, pio_offset, ctrl_pin_start, data_pin);

    dma_config = dma_channel_get_default_config(dma_channel);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_32);
    channel_config_set_read_increment(&dma_config, false);
    channel_config_set_write_increment(&dma_config, true);
    channel_config_set_dreq(&dma_config, pio_get_dreq(pio, pio_sm, false));
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
                                         INMP441PioDriverBuffer *p_buffer,
                                         uint count) {
    dma_channel_set_write_addr(p_driver->dma_channel, p_buffer, false);
    dma_channel_set_trans_count(p_driver->dma_channel, count, true);
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

void inpm441_pio_driver_buffer_init(INMP441PioDriverBuffer **pp_buffer,
                                    uint count) {
    INMP441PioDriverBuffer *buffer = malloc(sizeof(INMP441PioDriverBuffer));
    buffer->count = count;
    buffer->p_padded_buffer = malloc((count + RX_FIFO_SIZE) * sizeof(uint32_t));
    *pp_buffer = buffer;
}

void inmp441_pio_driver_buffer_deinit(INMP441PioDriverBuffer **pp_buffer) {
    INMP441PioDriverBuffer *buffer;

    if (!(buffer = *pp_buffer))
        return;
}