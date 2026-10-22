#include <display_thread.hpp>
#include <display_hardware.hpp>
#include <display.hpp>

#include <zephyr/kernel.h>

K_THREAD_STACK_DEFINE(display_stack_area, 8192);
struct k_thread display_thread_data;


void DisplayThread::display_entry_point(void* p1, void* p2, void* p3)
{
    const struct gpio_dt_spec* eve_cs = static_cast<const struct gpio_dt_spec*>(p1);
    const struct gpio_dt_spec* eve_pd = static_cast<const struct gpio_dt_spec*>(p2);
    const struct spi_dt_spec* eve_spi = static_cast<const struct spi_dt_spec*>(p3);
    DisplayHardware::init(eve_cs, eve_pd, eve_spi);

    Display display = Display();
    display.update();
}

void DisplayThread::display_thread_init(const struct gpio_dt_spec* eve_cs,
                                  const struct gpio_dt_spec* eve_pd,
                                  const struct spi_dt_spec* eve_spi)
{
    k_thread_create(&display_thread_data, display_stack_area,
                    K_THREAD_STACK_SIZEOF(display_stack_area),
                    display_entry_point,
                    (void*)eve_cs, (void*)eve_pd, (void*)eve_spi,
                    7, 0, K_NO_WAIT);
}