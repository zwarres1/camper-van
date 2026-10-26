#include <display.hpp>

#include <events.hpp>
#include <display_hardware.hpp>

void Display::run()
{
    m_lvgl_manager.build_ui();
    auto disp = lv_display_get_default();

    while(1)
    {
        // TODO: This should all be reactive to interrupts and such
        auto sleep_ms = m_lvgl_manager.update();

        if (sleep_ms == 0) {
            /* If 0, LVGL has immediate work pending (e.g., mid-animation) */
            k_yield();
        } else {
            /* k_sem_take returns 0 if it was woken by the interrupt/network, 
             * or -EAGAIN if the LVGL animation timeout expired naturally */
            int sem_result = k_sem_take(&ui_wake_sem, K_MSEC(sleep_ms));
                
            /* If the hardware ISR gave the semaphore, read the touch data */
            if (sem_result == 0) {
                DisplayHardware::clear_isr();
                lv_indev_t * indev = lv_indev_get_next(NULL);
                if (indev) {
                    lv_indev_read(indev);
                }
            }
        }
    }
}