/**
 * @file driver.c
 * @author Suhel Chakraborty (chakraborty.suhel@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-11-28
 *
 * Highly optimized SPI DMA based data transmission layer for WS2812 RGB addressable LED
 * strips.
 *
 * WS2812 RGB addressable LED chain driver. The SPI peripheral is used for the transmission
 * with the optional benefit of DMA. At 3MHz, each "bit" takes 0.33us(330ns), and complying
 * with WS2812 documentation, the following is the encoding
 *
 * Transmit "0" = 0.33us HIGH followed by 0.99us LOW = Represented as "1000"
 * Transmit "1" = 0.66us HIGH followed by 0.66us LOW = Represented as "1100"
 *
 * which falls safely inside the timing tolerance
 * @copyright Copyright (c) 2022
 *
 */

#include "driver.h"
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include <memory.h>
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "hardware/dma.h"

#define SPI_FREQ 3333333

/**
 * @brief The number of transmission-bytes required to invoke a "reset" in WS2812.
 *
 * At 3MHz, each bit is transmitted at 1/3us, thus a byte at 8/3us.
 * Required pulse length is minimum 50us. Minimum bytes required is
 *
 * 50/(8/3) = 150/8 = 18.75 ~ 19 bytes
 *
 */
#define BYTES_TRANSMITTED_FOR_RESET 22

/**
 * @brief The number of data-bytes an RGB pixel
 *
 */
#define BYTES_PER_RGB_PIXEL 3

/**
 * @brief Number of bits required to encode one data bit.
 *
 * At 3MHz, one bit takes 1/3us.
 * Both T0H(3/3) + T0L(1/3) = T1H(2/3) + T1L(2/3) = 4*(1/3)us
 *
 */
#define BYTES_ENCODING_MULTIPLIER 4

#define ARGB_RED(rgb) ((rgb >> 16) & 0xFF)
#define ARGB_GREEN(rgb) ((rgb >> 8) & 0xFF)
#define ARGB_BLUE(rgb) (rgb & 0xFF)

struct WS2812Driver
{
    size_t ledCount;
    uint dmaChannel;
    uint16_t *pDataBuffer;
};

/**
 * @brief Dictionary entries to encode 4 bits of data at a time to
 *        transmit to
 *
 * See if we can use 4-bits in SPI0 Tx line to represent a bit signal
 *
 * Transmission
 * "0" ~~> 0.33us HIGH + 0.99us LOW => 1000
 * "1" ~~> 0.66us HIGH + 0.66us LOW => 1100
 *
 * Thus
 * 0b0 becomes 0b1000 or 0x8
 * 0b1 becomes 0b1100 or 0xC
 */
static uint16_t DICT[] = {
    // 0b0000
    0x8888U,
    // 0b0001
    0x888CU,
    // 0b0010
    0x88C8U,
    // 0b0011
    0x88CCU,
    // 0b0100
    0x8C00U,
    // 0b0101
    0x8C8CU,
    // 0b0110
    0x8CC8U,
    // 0b0111
    0x8CCCU,
    // 0b1000
    0xC888U,
    // 0b1001
    0xC88CU,
    // 0b1010
    0xC8C8U,
    // 0b1011
    0xC8CCU,
    // 0b1100
    0xCC88U,
    // 0b1101
    0xCC8CU,
    // 0b1110
    0xCCC8U,
    // 0b1111
    0xCCCCU,
};

void ws2812_driver_init(WS2812Driver **ppDriver, size_t ledCount)
{
    WS2812Driver *driver = malloc(sizeof(WS2812Driver));
    if (ledCount <= 0)
    {
        panic("Invalid led count\n");
    }

    driver->ledCount = ledCount;

    spi_init(spi_default, SPI_FREQ);
    spi_set_format(spi_default, 8, SPI_CPOL_0, SPI_CPOL_0, SPI_MSB_FIRST);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);

    size_t dataXferSize = ledCount * BYTES_PER_RGB_PIXEL * BYTES_ENCODING_MULTIPLIER;
    size_t totalXferSize = dataXferSize + BYTES_TRANSMITTED_FOR_RESET;
    driver->pDataBuffer = malloc(totalXferSize);
    driver->dmaChannel = dma_claim_unused_channel(true);
    memset((uint8_t *)(driver->pDataBuffer), 0, totalXferSize);

    dma_channel_config config = dma_channel_get_default_config(driver->dmaChannel);
    channel_config_set_transfer_data_size(&config, DMA_SIZE_8);
    channel_config_set_dreq(&config, spi_get_dreq(spi_default, true));
    channel_config_set_read_increment(&config, true);
    channel_config_set_write_increment(&config, false);

    dma_channel_configure(
        driver->dmaChannel,
        &config,
        &spi_get_hw(spi_default)->dr,
        NULL,
        totalXferSize,
        false);
    *ppDriver = driver;
}

void ws2812_driver_submit_argb_buffer_blocking(WS2812Driver *pDriver, const uint32_t *pArgbBuffer)
{
    for (int i = 0, k = 0; i < pDriver->ledCount; i++, k += 6)
    {
        uint32_t rgb = pArgbBuffer[i];
        uint8_t red = ARGB_RED(rgb);
        uint8_t green = ARGB_GREEN(rgb);
        uint8_t blue = ARGB_BLUE(rgb);

        pDriver->pDataBuffer[k] = DICT[(green >> 4) & 0xF];
        pDriver->pDataBuffer[k + 1] = DICT[green & 0xF];

        pDriver->pDataBuffer[k + 2] = DICT[(red >> 4) & 0xF];
        pDriver->pDataBuffer[k + 3] = DICT[red & 0xF];

        pDriver->pDataBuffer[k + 4] = DICT[(blue >> 4) & 0xF];
        pDriver->pDataBuffer[k + 5] = DICT[blue & 0xF];
    }

    // As the DMA read address is set to increment, it needs to be
    // set to the start of the buffer everytime a transfer is queued
    dma_channel_set_read_addr(pDriver->dmaChannel, pDriver->pDataBuffer, true);
    dma_channel_wait_for_finish_blocking(pDriver->dmaChannel);
}

void ws2812_driver_deinit(WS2812Driver **ppDriver)
{
    WS2812Driver *driver = *ppDriver;
    dma_channel_unclaim(driver->dmaChannel);
    spi_deinit(spi_default);
    free(driver->pDataBuffer);
    free(driver);
    *ppDriver = NULL;
}