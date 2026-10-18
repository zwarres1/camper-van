#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

// TODO: move this to its own file
#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "Gpu_Hal.h"
#include "App_Common.h"
#ifdef __cplusplus
}
#endif

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec eve_cs = GPIO_DT_SPEC_GET(DT_ALIAS(eve_cs), gpios);

/* Define the hardware handles with C linkage */
extern "C" {
    extern const struct spi_dt_spec eve_spi;
    extern const struct gpio_dt_spec eve_pd;
}

static void zephyr_spi_cb(lv_display_t * disp, lv_draw_eve_operation_t operation,
                  void * data, uint32_t length)
{
    switch(operation) {
        case LV_DRAW_EVE_OPERATION_POWERDOWN_SET:
            /* setting the pin low powers down the EVE chip */
            gpio_pin_set_dt(&eve_pd, 0);
            break;
        case LV_DRAW_EVE_OPERATION_POWERDOWN_CLEAR:
            /* setting the pin high powers on the EVE chip */
            gpio_pin_set_dt(&eve_pd, 1);
            break;
        case LV_DRAW_EVE_OPERATION_CS_ASSERT:
            /* setting the pin low asserts the EVE chip SPI device */
            gpio_pin_set_dt(&eve_cs, 0);
            break;
        case LV_DRAW_EVE_OPERATION_CS_DEASSERT:
            /* setting the pin high de-asserts the EVE chip SPI device */
            gpio_pin_set_dt(&eve_cs, 1);
            break;
        case LV_DRAW_EVE_OPERATION_SPI_SEND:
        {
            /* `data` is the data to send */
            struct spi_buf tx_buf = { .buf = data, .len = length };
            struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };
            spi_write_dt(&eve_spi, &tx_set);
            break;
        }
        case LV_DRAW_EVE_OPERATION_SPI_RECEIVE:
        {
            /* `data` is the destination for the data */
            const struct spi_buf rx_buf = { .buf = data, .len = length };
            const struct spi_buf_set rx_set = { .buffers = &rx_buf, .count = 1 };
            spi_read_dt(&eve_spi, &rx_set);
            break;
        }
    }
}

int main(void)
{
    int ret;

	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}
	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}

    if (!gpio_is_ready_dt(&eve_pd)) {
        return 0;
    }
    ret = gpio_pin_configure_dt(&eve_pd, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        return 0;
    }

    if (!gpio_is_ready_dt(&eve_cs)) {
        return 0;
    }
    ret = gpio_pin_configure_dt(&eve_cs, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        return 0;
    }

	/* Check if devices are ready */
    if (!spi_is_ready_dt(&eve_spi) || !gpio_is_ready_dt(&eve_pd)) {
        return -1; 
    }

    /* Riverdi EVE init */
    // App_Common_Init();

    lv_draw_eve_parameters_t params = {
        .hor_res = 1024,
        .ver_res = 600,
        .hcycle = 816,
        .hoffset = 8,
        .hsync0 = 0,
        .hsync1 = 4,
        .vcycle = 496,
        .voffset = 8,
        .vsync0 = 0,
        .vsync1 = 4,
        .swizzle = 0,
        .pclkpol = 1,
        .cspread = 0,
        .pclk = 3,
        .has_crystal = true,
        .has_gt911 = false,
        .backlight_pwm = 128,
        .backlight_freq = 4000,
    };
    lv_init();
    lv_tick_set_cb(k_uptime_get_32);
    //lv_draw_eve_init();
    auto disp = lv_draw_eve_display_create(&params, zephyr_spi_cb, NULL);


    /* Set LVGL screen background to dark grey to verify rendering */
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFF0000), 0);

    /* Draw a test button for the camper van dashboard UI */
    lv_obj_t * btn = lv_btn_create(lv_screen_active());
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, "Water Pump");

    /* Keep UI alive */
    while (1) {
        lv_task_handler();
        k_msleep(5);
        lv_tick_inc(5);
    }
	return 0;
}
