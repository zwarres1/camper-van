#include <display_thread.hpp>

#include <display_hardware.hpp>
#include <display.hpp>

#include <zephyr/kernel.h>

K_THREAD_STACK_DEFINE(display_stack_area, 8192);
struct k_thread display_thread_data;


void DisplayThread::display_entry_point(void* p1, void* p2, void* p3)
{
    const EveHardware* eve_hw_ptr = static_cast<const EveHardware*>(p1);
    DisplayHardware::init(eve_hw_ptr);

    Display display = Display();
    display.run();
}

void DisplayThread::display_thread_init(EveHardware* eve_hw_ptr)
{
    k_thread_create(&display_thread_data, display_stack_area,
                    K_THREAD_STACK_SIZEOF(display_stack_area),
                    display_entry_point,
                    (void*)eve_hw_ptr, NULL, NULL,
                    7, 0, K_NO_WAIT);
}