#include <stdlib.h>

#include "rgb_status.h"

#include "hardware/pio.h"
#include "ws2812_pio/driver.h"

#define STATUS_LED_PIN 16

struct RGBStatus {
    WS2812PioDriver *p_driver;
};

void rgb_status_init(RGBStatus **pp_status) {
    WS2812PioDriver *driver = NULL;
    RGBStatus *status = NULL;

    if (*pp_status)
        return;

    ws2812_pio_driver_init(&driver, STATUS_LED_PIN, 1);

    status = malloc(sizeof(RGBStatus));
    status->p_driver = driver;

    *pp_status = status;
}

void rgb_status_set_color_blocking(RGBStatus *p_status, uint32_t color) {
    ws2812_pio_driver_submit_buffer_blocking(p_status->p_driver, &color);
}

void rgb_status_set_color_rgb_blocking(RGBStatus *p_status, uint8_t red,
                                       uint8_t green, uint8_t blue) {
    uint32_t color = (green << 24) | (red << 16) | (blue << 8);
    rgb_status_set_color_blocking(p_status, color);
}

void rgb_status_deinit(RGBStatus **pp_status) {
    RGBStatus *status;

    if (!(status = *pp_status))
        return;

    ws2812_pio_driver_deinit(&status->p_driver);
    free(status);

    *pp_status = NULL;
}