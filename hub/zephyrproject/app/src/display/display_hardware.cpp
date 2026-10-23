#include <display_hardware.hpp>

#include <events.hpp>

#include <zephyr/kernel.h>
#include <lvgl.h>
#include <libs/FT800-FT813/EVE.h>

static struct gpio_callback eve_int_cb_data;

namespace
{
    const gpio_dt_spec* m_eve_cs = nullptr;
    const gpio_dt_spec* m_eve_pd = nullptr;
    const gpio_dt_spec* m_eve_int = nullptr;
    const spi_dt_spec*  m_eve_spi = nullptr;

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
                spi_buf tx_buf = { .buf = data, .len = length };
                spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };
                spi_write_dt(m_eve_spi, &tx_set);
                break;
            }
            case LV_DRAW_EVE_OPERATION_SPI_RECEIVE:
            {
                /* `data` is the destination for the data */
                const spi_buf rx_buf = { .buf = data, .len = length };
                const spi_buf_set rx_set = { .buffers = &rx_buf, .count = 1 };
                spi_read_dt(m_eve_spi, &rx_set);
                break;
            }
        }
    }

    /* The Hardware Interrupt: executes inside the Zephyr ISR context */
    static void eve_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
        k_sem_give(&ui_wake_sem);
    }
}

void DisplayHardware::init(const EveHardware* eve_hw_ptr)
{
    m_eve_cs = eve_hw_ptr->cs;
    m_eve_pd = eve_hw_ptr->pd;
    m_eve_int = eve_hw_ptr->int_pin;
    m_eve_spi = eve_hw_ptr->spi;

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
        .pclk = 1,
        .has_crystal = true,
        .has_gt911 = false,
        .backlight_pwm = 20,
        .backlight_freq = 4000,
    };
    
    // TODO: On PCB, undo rotation, increase backlight_pwm
    k_msleep(500);
    
    lv_init();
    lv_tick_set_cb(k_uptime_get_32);
    auto disp = lv_draw_eve_display_create(&params, zephyr_spi_cb, NULL);
    lv_draw_eve_memwrite32(disp, REG_TOUCH_MODE, 2);
    lv_draw_eve_memwrite32(disp, REG_INT_MASK, 0x2);
    lv_draw_eve_memwrite32(disp, REG_INT_EN, 1);
    lv_draw_eve_memread32(lv_display_get_default(), REG_INT_FLAGS);

    // uint32_t mask_val = lv_draw_eve_memread32(disp, 0x3020B0); // REG_INT_MASK
    // uint32_t en_val   = lv_draw_eve_memread32(disp, 0x3020AC); // REG_INT_EN

    // printk("EVE REG_INT_MASK: %u (Expected 255)\n", mask_val);
    // printk("EVE REG_INT_EN: %u (Expected 1)\n", en_val);

    // while(1) {
    //     /* Reading this register tells us what the chip is trying to do.
    //      * (Note: Reading it automatically clears the flags in hardware!) */
    //     uint32_t flags = lv_draw_eve_memread32(disp, REG_INT_FLAGS);
        
    //     if (flags != 0) {
    //         printk("EVE INT FLAGS: 0x%02X\n", flags);
    //     }

    //     k_msleep(500); 
    // }
    lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_180);
    auto touch_indev = lv_draw_eve_touch_create(disp);
    lv_indev_set_mode(touch_indev, LV_INDEV_MODE_EVENT);


    /* Register the Zephyr callback */
    gpio_init_callback(&eve_int_cb_data, eve_isr, BIT(m_eve_int->pin));
    gpio_add_callback(m_eve_int->port, &eve_int_cb_data);
}

void DisplayHardware::clear_isr()
{
    lv_draw_eve_memread32(lv_display_get_default(), REG_INT_FLAGS);
}