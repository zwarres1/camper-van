#pragma once

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

namespace DisplayThread
{

void display_entry_point(void* p1, void* p2, void* p3);

void display_thread_init(const struct gpio_dt_spec* eve_cs,
                        const struct gpio_dt_spec* eve_pd,
                        const struct spi_dt_spec* eve_spi);

}