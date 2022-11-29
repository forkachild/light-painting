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
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "hardware/dma.h"

#define SPI_FREQ 3000000
#define BYTES_PER_RGB_PIXEL 3

/**
 * @brief Number of bits required to encode one data bit.
 *
 * At 3MHz, one bit takes 1/3us.
 * Both T0H(3/3) + T0L(1/3) = T1H(2/3) + T1L(2/3) = 4*(1/3)us
 *
 */
#define ENCODING_MULTIPLIER 4

/**
 * @brief Byte padding to be filled to represent "reset" pulse.
 *
 * At 3MHz, each bit is transmitted at 1/3us, thus a byte at 8/3us.
 * Required pulse length is minimum 50us. Minimum bytes required is
 *
 * 50/(8/3) = 150/8 = 18.75 ~ 19 bytes
 *
 */
#define BYTES_FOR_RESET 19

struct WS2812Driver
{
    size_t ledCount;
    uint dmaChannel;
    uint16_t *dataBuffer;
};

/**
 * @brief Dictionary entries to encode 4 bits of data at a time
 *
 */
static uint16_t DICT[] = {
    // 0b0000
    0b1000100010001000,
    // 0b0001
    0b1000100010001100,
    // 0b0010
    0b1000100011001000,
    // 0b0011
    0b1000100011001100,
    // 0b0100
    0b1000110010001000,
    // 0b0101
    0b1000110010001100,
    // 0b0110
    0b1000110011001000,
    // 0b0111
    0b1000110011001100,
    // 0b1000
    0b1100100010001000,
    // 0b1001
    0b1100100010001100,
    // 0b1010
    0b1100100011001000,
    // 0b1011
    0b1100100011001100,
    // 0b1100
    0b1100110010001000,
    // 0b1101
    0b1100110010001100,
    // 0b1110
    0b1100110011001000,
    // 0b1111
    0b1100110011001100,
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
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI));
    bi_decl(bi_1pin_with_name(PICO_DEFAULT_SPI_CSN_PIN, "SPI CS"));

    size_t dataTransferSize = (ledCount * BYTES_PER_RGB_PIXEL * ENCODING_MULTIPLIER) + BYTES_FOR_RESET;
    driver->dataBuffer = (uint16_t *)malloc(dataTransferSize);
    driver->dmaChannel = dma_claim_unused_channel(true);

    dma_channel_config config = dma_channel_get_default_config(driver->dmaChannel);
    channel_config_set_transfer_data_size(&config, DMA_SIZE_8);
    channel_config_set_dreq(&config, spi_get_dreq(spi_default, true));
    dma_channel_configure(
        driver->dmaChannel,
        &config,
        &spi_get_hw(spi_default)->dr,
        driver->dataBuffer,
        dataTransferSize,
        false);
    *ppDriver = driver;
}

void ws2812_driver_submit_argb_buffer_blocking(WS2812Driver *pDriver, const uint32_t *pArgbBuffer)
{
    for (int i = 0, k = 0; i < pDriver->ledCount; i++)
    {
        uint32_t rgb = pArgbBuffer[i];
        uint8_t red = (uint8_t)(rgb >> 16);
        uint8_t green = (uint8_t)(rgb >> 8);
        uint8_t blue = (uint8_t)rgb;

        pDriver->dataBuffer[k++] = DICT[green >> 4];
        pDriver->dataBuffer[k++] = DICT[green & 0x0F];

        pDriver->dataBuffer[k++] = DICT[red >> 4];
        pDriver->dataBuffer[k++] = DICT[red & 0x0F];

        pDriver->dataBuffer[k++] = DICT[blue >> 4];
        pDriver->dataBuffer[k++] = DICT[blue & 0x0F];
    }

    dma_start_channel_mask((1 << pDriver->dmaChannel));
    dma_channel_wait_for_finish_blocking(pDriver->dmaChannel);
}

void ws2812_driver_deinit(WS2812Driver **ppDriver)
{
    WS2812Driver *driver = *ppDriver;
    spi_deinit(spi_default);
    gpio_deinit(PICO_DEFAULT_SPI_CSN_PIN);
    dma_channel_unclaim(driver->dmaChannel);
    free(driver->dataBuffer);
    free(driver);
    *ppDriver = NULL;
}