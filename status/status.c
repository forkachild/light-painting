#include "status.h"
#include "pico/stdlib.h"

void status_init()
{
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
}

void status_show(bool value)
{
    gpio_put(PICO_DEFAULT_LED_PIN, value);
}

void status_blink_blocking(uint32_t times, uint32_t delay)
{
    for (int i = 0; i < times; i++)
    {
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        sleep_ms(delay);
        gpio_put(PICO_DEFAULT_LED_PIN, false);
        sleep_ms(delay);
    }
}

void status_panic_blocking(uint32_t delay)
{
    while (true)
    {
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        sleep_ms(delay);
        gpio_put(PICO_DEFAULT_LED_PIN, false);
        sleep_ms(delay);
    }
}

void status_deinit()
{
    gpio_deinit(PICO_DEFAULT_LED_PIN);
}