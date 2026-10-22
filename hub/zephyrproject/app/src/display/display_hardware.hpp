#pragma once

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

namespace DisplayHardware
{
    void init(const gpio_dt_spec* eve_cs,
                   const gpio_dt_spec* eve_pd,
                   const spi_dt_spec* eve_spi);
}