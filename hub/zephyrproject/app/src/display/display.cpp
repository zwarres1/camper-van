#include <display.hpp>

#include <zephyr/kernel.h>

void Display::run()
{
    m_lvgl_manager.build_ui();

    while(1)
    {
        // TODO: This should all be reactive to interrupts and such
        lv_task_handler();

        k_msleep(5);
    }
}