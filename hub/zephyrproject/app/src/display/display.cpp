#include <display.hpp>

#include <Gpu.h>

#include <zephyr/kernel.h>

Display::Display()
{
    m_lvgl_manager = LVGLManager();
}

void Display::run()
{
    while(1)
    {
        // TODO: This should all be reactive to interrupts and such
        lv_task_handler();

        uint32_t touch_xy = lv_draw_eve_memread32(lv_display_get_default(), REG_TOUCH_SCREEN_XY);
        if (touch_xy != 0x80008000 && touch_xy != 0xFFFFFFFF) {
            printk("HARDWARE TOUCH DETECTED! Raw XY: 0x%08X\n", touch_xy);
        }

        k_msleep(5);
    }
}