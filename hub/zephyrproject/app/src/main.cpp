#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

#include <display_thread.hpp>
#include <controls_thread.hpp>
#include <network_thread.hpp>

// TODO: move this to its own file
#include <lvgl.h>
#include <Gpu.h>

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec eve_cs = GPIO_DT_SPEC_GET(DT_ALIAS(eve_cs), gpios);
static const struct gpio_dt_spec eve_pd = GPIO_DT_SPEC_GET(DT_ALIAS(eve_pd), gpios);
/* EVE chips use SPI Mode 0 (CPOL=0, CPHA=0), MSB first */
const struct spi_dt_spec eve_spi = {
    .bus = DEVICE_DT_GET(DT_BUS(DT_NODELABEL(eve_display))),
    .config = {
        .frequency = DT_PROP(DT_NODELABEL(eve_display), spi_max_frequency),
        .operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB,
        .slave = DT_REG_ADDR(DT_NODELABEL(eve_display))
        /* .cs is intentionally omitted because eve lvgl toggles eve_cs manually */
    }
};

int main(void)
{
    if (!gpio_is_ready_dt(&led) || 
        !gpio_is_ready_dt(&eve_pd) || 
        !gpio_is_ready_dt(&eve_cs) || 
        !spi_is_ready_dt(&eve_spi)) {
        return -1; /* Fail immediately if any bus is offline */
    }
    if (gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE) < 0) {
        return -1;
    }
    /* 3. Configure the Riverdi EVE Power Down (PD) pin */
    if (gpio_pin_configure_dt(&eve_pd, GPIO_OUTPUT_ACTIVE) < 0) {
        return -1;
    }
    if (gpio_pin_configure_dt(&eve_cs, GPIO_OUTPUT_INACTIVE) < 0) {
        return -1;
    }

    controls::controls_thread_init();
	network::network_thread_init();
	DisplayThread::display_thread_init(&eve_cs, &eve_pd, &eve_spi);

    /* Keep UI alive */
    while (1) {
        // TODO: put LED blink back in
        lv_task_handler();
        k_msleep(5);
    }
	return 0;
}
