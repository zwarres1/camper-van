#include <controls_thread.hpp>

#include <zephyr/kernel.h>

K_THREAD_STACK_DEFINE(controls_stack_area, 4096);
struct k_thread controls_thread_data;

void controls::controls_entry_point(void* p1, void* p2, void* p3)
{
    while(1)
    {
        k_msleep(1000);
    }
}

void controls::controls_thread_init()
{
    k_thread_create(&controls_thread_data, controls_stack_area,
                    K_THREAD_STACK_SIZEOF(controls_stack_area),
                    controls_entry_point,
                    NULL, NULL, NULL,
                    5, 0, K_NO_WAIT);
}