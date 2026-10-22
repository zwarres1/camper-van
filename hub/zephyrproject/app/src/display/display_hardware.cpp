#include <display_hardware.hpp>

#include <zephyr/kernel.h>
#include <lvgl.h>
#include <Gpu.h>

namespace {
    const struct gpio_dt_spec* m_eve_cs = nullptr;
    const struct gpio_dt_spec* m_eve_pd = nullptr;
    const struct spi_dt_spec*  m_eve_spi = nullptr;

    void zephyr_spi_cb(lv_display_t * disp, lv_draw_eve_operation_t operation,
                    void * data, uint32_t length)
    {
        switch(operation) {
            case LV_DRAW_EVE_OPERATION_POWERDOWN_SET:
                /* setting the pin low powers down the EVE chip */
                gpio_pin_set_dt(m_eve_pd, 0);
                break;
            case LV_DRAW_EVE_OPERATION_POWERDOWN_CLEAR:
                /* setting the pin high powers on the EVE chip */
                gpio_pin_set_dt(m_eve_pd, 1);
                break;
            case LV_DRAW_EVE_OPERATION_CS_ASSERT:
                /* setting the pin low asserts the EVE chip SPI device */
                gpio_pin_set_dt(m_eve_cs, 0);
                break;
            case LV_DRAW_EVE_OPERATION_CS_DEASSERT:
                /* setting the pin high de-asserts the EVE chip SPI device */
                gpio_pin_set_dt(m_eve_cs, 1);
                break;
            case LV_DRAW_EVE_OPERATION_SPI_SEND:
            {
                /* `data` is the data to send */
                struct spi_buf tx_buf = { .buf = data, .len = length };
                struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };
                spi_write_dt(m_eve_spi, &tx_set);
                break;
            }
            case LV_DRAW_EVE_OPERATION_SPI_RECEIVE:
            {
                /* `data` is the destination for the data */
                const struct spi_buf rx_buf = { .buf = data, .len = length };
                const struct spi_buf_set rx_set = { .buffers = &rx_buf, .count = 1 };
                spi_read_dt(m_eve_spi, &rx_set);
                break;
            }
        }
    }
}

void DisplayHardware::init(const struct gpio_dt_spec* eve_cs,
            const struct gpio_dt_spec* eve_pd,
            const struct spi_dt_spec* eve_spi)
{
    m_eve_cs = eve_cs;
    m_eve_pd = eve_pd;
    m_eve_spi = eve_spi;

    lv_draw_eve_parameters_t params = {
        .hor_res = 1024,
        .ver_res = 600,
        .hcycle = 1344,
        .hoffset = 160,
        .hsync0 = 0,
        .hsync1 = 70,
        .vcycle = 635,
        .voffset = 23,
        .vsync0 = 0,
        .vsync1 = 10,
        .swizzle = 0,
        .pclkpol = 1,
        .cspread = 0,
        .pclk = 2,
        .has_crystal = true,
        .has_gt911 = false,
        .backlight_pwm = 30,
        .backlight_freq = 4000,
    };
    lv_init();
    lv_tick_set_cb(k_uptime_get_32);
    auto disp = lv_draw_eve_display_create(&params, zephyr_spi_cb, NULL);
    lv_draw_eve_memwrite16(disp, REG_ROTATE, 0x1);

    /* Set LVGL screen background to dark grey to verify rendering */
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFF0000), 0);

    /* Draw a test button for the camper van dashboard UI */
    lv_obj_t * btn = lv_btn_create(lv_screen_active());
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, "Power Levels2");

    // TODO: make this interrupt based
    while (1) {
        lv_task_handler();
        k_msleep(5);
    }
}