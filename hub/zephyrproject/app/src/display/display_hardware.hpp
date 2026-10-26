#pragma once

#include <eve_hardware_struct.hpp>

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

/* 
* Namespace that contains the actual hardware initialization and handling for the display
*/
namespace DisplayHardware
{
    void init(const EveHardware* eve_hw_ptr);

    void clear_isr();
}