#include <display.hpp>

#include <events.hpp>

#include <zephyr/kernel.h>

void Display::run()
{
    m_lvgl_manager.build_ui();

    while(1)
    {
        // TODO: This should all be reactive to interrupts and such
        auto sleep_ms = m_lvgl_manager.update();

        if (sleep_ms == 0) {
            /* If 0, LVGL has immediate work pending (e.g., mid-animation) */
            k_yield();
        } else {
            /* Sleep the CPU. Wakes up early if the screen is touched! */
            k_sem_take(&ui_wake_sem, K_MSEC(sleep_ms));
        }
    }
}