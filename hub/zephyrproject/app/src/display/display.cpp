#include <display.hpp>

#include <zephyr/kernel.h>

void Display::update()
{
    while(1)
    {
        // TODO: This should all be reactive to interrupts and such
        k_msleep(1000);
    }
}