#pragma once

#include "pico/types.h"

void status_init();
void status_show(bool value);
void status_blink_blocking(uint32_t times, uint32_t delay);
void status_panic_blocking(uint32_t delay);
void status_deinit();