#include <network_thread.hpp>

#include <zephyr/kernel.h>

K_THREAD_STACK_DEFINE(network_stack_area, 4096);
struct k_thread network_thread_data;

void network::network_entry_point(void* p1, void* p2, void* p3)
{
    while(1)
    {
        k_msleep(1000);
    }
}

void network::network_thread_init()
{
    k_thread_create(&network_thread_data, network_stack_area,
                    K_THREAD_STACK_SIZEOF(network_stack_area),
                    network_entry_point,
                    NULL, NULL, NULL,
                    6, 0, K_NO_WAIT);
}