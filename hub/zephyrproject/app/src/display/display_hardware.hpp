#pragma once

#include <eve_hardware_struct.hpp>

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

namespace DisplayHardware
{
    void init(const EveHardware* eve_hw_ptr);
}