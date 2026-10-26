#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

#include <display_thread.hpp>
#include <controls_thread.hpp>
#include <network_thread.hpp>
#include <eve_hardware_struct.hpp>

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec eve_cs = GPIO_DT_SPEC_GET(DT_ALIAS(eve_cs), gpios);
static const struct gpio_dt_spec eve_pd = GPIO_DT_SPEC_GET(DT_ALIAS(eve_pd), gpios);
static const struct gpio_dt_spec eve_int = GPIO_DT_SPEC_GET(DT_ALIAS(eve_int), gpios);
/* EVE chips use SPI Mode 0 (CPOL=0, CPHA=0), MSB first */
const struct spi_dt_spec eve_spi = SPI_DT_SPEC_GET(
    DT_NODELABEL(eve_display), 
    SPI_WORD_SET(8) | SPI_TRANSFER_MSB
);

int main(void)
{
    if (!gpio_is_ready_dt(&led) || 
        !gpio_is_ready_dt(&eve_pd) || 
        !gpio_is_ready_dt(&eve_cs) || 
        !gpio_is_ready_dt(&eve_int) ||
        !spi_is_ready_dt(&eve_spi)) {
        return -1; /* Fail immediately if any bus is offline */
    }
    if (gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE) < 0) {
        return -1;
    }
    if (gpio_pin_configure_dt(&eve_pd, GPIO_OUTPUT_INACTIVE) < 0) {
        return -1;
    }
    if (gpio_pin_configure_dt(&eve_cs, GPIO_OUTPUT_INACTIVE) < 0) {
        return -1;
    }

    // eve_int is both an input and an interrupt
    if (gpio_pin_configure_dt(&eve_int, GPIO_INPUT) < 0) {
        return -1;
    }
    if (gpio_pin_interrupt_configure_dt(&eve_int, GPIO_INT_EDGE_TO_ACTIVE) < 0) {
        return -1;
    }

    EveHardware eve_hw = {
        .spi = &eve_spi,
        .cs = &eve_cs,
        .pd = &eve_pd,
        .int_pin = &eve_int 
    };

    //k_msleep(1000U);
    controls::controls_thread_init();
	network::network_thread_init();
	DisplayThread::display_thread_init(&eve_hw);

    /* Keep UI alive */
    while (1) {
        // TODO: put LED blink back in
        k_msleep(1000);
    }
	return 0;
}
