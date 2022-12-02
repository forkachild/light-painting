#pragma once

#include <stdbool.h>
#include <stdint.h>

#define STATUS_DELAY_FAST_MS 300
#define STATUS_DELAY_MEDIUM_MS 700
#define STATUS_DELAY_SLOW_MS 1500

void status_init();
void status_show(bool value);
void status_blink_blocking(uint32_t times, uint32_t delay);
void status_panic_blocking(uint32_t delay);
void status_deinit();