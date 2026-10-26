#pragma once

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

struct EveHardware {
    const struct spi_dt_spec* spi;
    const struct gpio_dt_spec* cs;
    const struct gpio_dt_spec* pd;
    const struct gpio_dt_spec* int_pin;
};