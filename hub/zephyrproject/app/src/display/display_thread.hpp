#pragma once

#include <eve_hardware_struct.hpp>

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

namespace DisplayThread
{

void display_entry_point(void* p1, void* p2, void* p3);

void display_thread_init(EveHardware* eve_hw_ptr);

}