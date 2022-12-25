#pragma once

#include "pico/types.h"
#include "swapchain.h"

void inmp441_init(uint samples, uint sck_pin, uint ws_pin, uint data_pin,
                         uint lr_config_pin);

void inmp441_start_sampling();

void inmp441_stop_sampling();

Swapchain *inmp441_get_swapchain();

void inmp441_deinit();