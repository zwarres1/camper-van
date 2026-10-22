#pragma once

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

namespace DisplayHardware
{
    void init(const struct gpio_dt_spec* eve_cs,
                   const struct gpio_dt_spec* eve_pd,
                   const struct spi_dt_spec* eve_spi);
}