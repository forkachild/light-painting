#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "status.h"

#define STATUS_DELAY_FAST_MS 300
#define STATUS_DELAY_MEDIUM_MS 700
#define STATUS_DELAY_SLOW_MS 1500

static inline void status_init() {
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
}

static inline void status_show(bool value) {
    gpio_put(PICO_DEFAULT_LED_PIN, value);
}

static inline void status_blink_blocking(uint32_t times, uint32_t delay) {
    for (int i = 0; i < times; i++) {
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        sleep_ms(delay);
        gpio_put(PICO_DEFAULT_LED_PIN, false);
        sleep_ms(delay);
    }
}

static inline void status_panic_blocking(uint32_t delay) {
    while (true) {
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        sleep_ms(delay);
        gpio_put(PICO_DEFAULT_LED_PIN, false);
        sleep_ms(delay);
    }
}

static inline void status_deinit() { gpio_deinit(PICO_DEFAULT_LED_PIN); }