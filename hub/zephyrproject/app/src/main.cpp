#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "Gpu_Hal.h"
#include "platform.h"
#include "App_Common.h"
#ifdef __cplusplus
}
#endif

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

/* Define the hardware handles with C linkage */
extern "C" {
    /* EVE chips use SPI Mode 0 (CPOL=0, CPHA=0), MSB first */
    const struct spi_dt_spec eve_spi = SPI_DT_SPEC_GET(
        DT_NODELABEL(eve_display), 
        SPI_WORD_SET(8) | SPI_TRANSFER_MSB
    );

    /* 2. Grab the Power Down and chip select pins from the Devicetree alias */
    const struct gpio_dt_spec eve_pd = GPIO_DT_SPEC_GET(DT_ALIAS(eve_pd), gpios);
}

void eve_host_cmd(uint8_t cmd) {
    uint8_t tx[3] = { cmd, 0x00, 0x00 };
    struct spi_buf tx_buf = { .buf = tx, .len = 3 };
    struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };
    spi_write_dt(&eve_spi, &tx_set);
}

/* Helper to write a 32-bit value to an EVE register */
void eve_write32(uint32_t addr, uint32_t val) {
    uint8_t tx_buf[7];
    /* 3-byte address with Write bit (0x80) set */
    tx_buf[0] = 0x80 | ((addr >> 16) & 0x3F); 
    tx_buf[1] = (addr >> 8) & 0xFF;
    tx_buf[2] = addr & 0xFF;
    
    /* 4-byte data payload (Little Endian) */
    tx_buf[3] = val & 0xFF;
    tx_buf[4] = (val >> 8) & 0xFF;
    tx_buf[5] = (val >> 16) & 0xFF;
    tx_buf[6] = (val >> 24) & 0xFF;

    struct spi_buf tx = {.buf = tx_buf, .len = sizeof(tx_buf)};
    struct spi_buf_set tx_set = {.buffers = &tx, .count = 1};
    spi_write_dt(&eve_spi, &tx_set);
}

/* Helper to write an 16-bit value (like a backlight duty cycle) */
void eve_write16(uint32_t addr, uint16_t val) {
    uint8_t tx_buf[5];
    /* 3-byte address with Write bit (0x80) set */
    tx_buf[0] = 0x80 | ((addr >> 16) & 0x3F); 
    tx_buf[1] = (addr >> 8) & 0xFF;
    tx_buf[2] = addr & 0xFF;
    
    /* 2-byte data payload (Little Endian) */
    tx_buf[3] = val & 0xFF;
    tx_buf[4] = (val >> 8) & 0xFF;

    struct spi_buf tx = {.buf = tx_buf, .len = sizeof(tx_buf)};
    struct spi_buf_set tx_set = {.buffers = &tx, .count = 1};
    spi_write_dt(&eve_spi, &tx_set);
}

/* Helper to write an 8-bit value (like a backlight duty cycle) */
void eve_write8(uint32_t addr, uint8_t val) {
    uint8_t tx_buf[4];
    tx_buf[0] = 0x80 | ((addr >> 16) & 0x3F);
    tx_buf[1] = (addr >> 8) & 0xFF;
    tx_buf[2] = addr & 0xFF;
    tx_buf[3] = val;

    struct spi_buf tx = {.buf = tx_buf, .len = sizeof(tx_buf)};
    struct spi_buf_set tx_set = {.buffers = &tx, .count = 1};
    spi_write_dt(&eve_spi, &tx_set);
}




#include "platform.h"
#include "App_Common.h"
#include "lvgl.h"

#define DISP_WIDTH      1024L
#define DISP_HEIGHT     600L
#define DISP_HCYCLE     1344L
#define DISP_HOFFSET    160L
#define DISP_HSYNC0     0L
#define DISP_HSYNC1     70L
#define DISP_VCYCLE     635L
#define DISP_VOFFSET    23L
#define DISP_VSYNC0     0L
#define DISP_VSYNC1     10L
#define DISP_PCLK       1
#define DISP_SWIZZLE    0
#define DISP_PCLKPOL    1
#define DISP_CSPREAD    0

/* Riverdi color palette (safe subset) */
#define COLOR_BG        lv_color_hex(0x0060A9)  /* Riverdi blue */
#define COLOR_TEXT      lv_color_hex(0xFFFFFF)  /* white */
#define COLOR_ACCENT    lv_color_hex(0xFF6229)  /* Riverdi orange */
#define COLOR_WIDGET    lv_color_hex(0xE6F2FA)  /* very light blue */

/* Global used for buffer optimization */
Gpu_Hal_Context_t host, *phost;

static void eve_op_cb(lv_display_t * disp,
                      lv_draw_eve_operation_t operation,
                      void * data,
                      uint32_t length);

static void create_test_ui(void);

static lv_draw_eve_parameters_t eve_params = {};

static void eve_op_cb(lv_display_t * disp,
                      lv_draw_eve_operation_t operation,
                      void * data,
                      uint32_t length)
{
    Gpu_Hal_Context_t * ctx =
        (Gpu_Hal_Context_t *)lv_display_get_driver_data(disp);

    switch(operation) {
        case LV_DRAW_EVE_OPERATION_POWERDOWN_SET:
            platform_gpio_value(ctx, GPIO_PD, GPIO_LOW);
            break;

        case LV_DRAW_EVE_OPERATION_POWERDOWN_CLEAR:
            platform_gpio_value(ctx, GPIO_PD, GPIO_HIGH);
            break;

        case LV_DRAW_EVE_OPERATION_CS_ASSERT:
            platform_gpio_value(ctx, GPIO_CS, GPIO_LOW);
            break;

        case LV_DRAW_EVE_OPERATION_CS_DEASSERT:
            platform_gpio_value(ctx, GPIO_CS, GPIO_HIGH);
            break;

        case LV_DRAW_EVE_OPERATION_SPI_SEND:
            platform_spi_send_data(ctx, (uchar8_t *)data, (uint16_t)length, 0);
            break;

        case LV_DRAW_EVE_OPERATION_SPI_RECEIVE:
            platform_spi_recv_data(ctx, (uchar8_t *)data, (uint16_t)length, 0);
            break;
    }
}

static void create_test_ui(void)
{
    lv_obj_t * scr = lv_screen_active();

    /* Background */
    lv_obj_set_style_bg_color(scr, COLOR_BG, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    /* Title */
    lv_obj_t * title = lv_label_create(scr);
    lv_label_set_text(title, "LVGL on Riverdi EVE");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_44, 0);
    lv_obj_set_style_text_color(title, COLOR_TEXT, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 48);

    /* Subtitle */
    lv_obj_t * subtitle = lv_label_create(scr);
    lv_label_set_text(subtitle, "Basic widget compatibility test");
    lv_obj_set_style_text_font(subtitle, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(subtitle, COLOR_TEXT, 0);
    lv_obj_align(subtitle, LV_ALIGN_TOP_MID, 0, 100);

    /* --- layout anchors --- */
    int start_y = 170;
    int col_left = 180;
    int col_right = 640;

    /* Button */
    lv_obj_t * btn = lv_button_create(scr);
    lv_obj_set_size(btn, 200, 60);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, col_left, start_y);
    lv_obj_set_style_bg_color(btn, COLOR_ACCENT, 0);

    lv_obj_t * btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "Button");
    lv_obj_center(btn_label);

    /* Slider */
    lv_obj_t * slider = lv_slider_create(scr);
    lv_obj_set_width(slider, 260);
    lv_obj_align(slider, LV_ALIGN_TOP_LEFT, col_right, start_y + 15);
    lv_slider_set_value(slider, 45, LV_ANIM_OFF);

    lv_obj_set_style_bg_color(slider, COLOR_WIDGET, LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider, COLOR_ACCENT, LV_PART_INDICATOR);

    /* Switch */
    lv_obj_t * sw = lv_switch_create(scr);
    lv_obj_align(sw, LV_ALIGN_TOP_LEFT, col_left, start_y + 120);

    lv_obj_t * sw_label = lv_label_create(scr);
    lv_label_set_text(sw_label, "Switch");
    lv_obj_set_style_text_color(sw_label, COLOR_TEXT, 0);
    lv_obj_align_to(sw_label, sw, LV_ALIGN_OUT_RIGHT_MID, 20, 0);

    /* Checkbox */
    lv_obj_t * cb = lv_checkbox_create(scr);
    lv_checkbox_set_text(cb, "Checkbox");
    lv_obj_set_style_text_color(cb, COLOR_TEXT, 0);
    lv_obj_align(cb, LV_ALIGN_TOP_LEFT, col_right, start_y + 120);

    /* Bar */
    lv_obj_t * bar = lv_bar_create(scr);
    lv_obj_set_size(bar, 500, 20);
    lv_obj_align(bar, LV_ALIGN_TOP_MID, 0, start_y + 220);
    lv_bar_set_value(bar, 70, LV_ANIM_OFF);

    lv_obj_set_style_bg_color(bar, COLOR_WIDGET, LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, COLOR_ACCENT, LV_PART_INDICATOR);

    lv_obj_t * bar_label = lv_label_create(scr);
    lv_label_set_text(bar_label, "Bar");
    lv_obj_set_style_text_color(bar_label, COLOR_TEXT, 0);
    lv_obj_align_to(bar_label, bar, LV_ALIGN_OUT_TOP_MID, 0, -8);
}









int main(void)
{
    eve_params.hor_res = DISP_WIDTH;
    eve_params.ver_res = DISP_HEIGHT;
    eve_params.vsync0 = DISP_VSYNC0;
    eve_params.vsync1 = DISP_VSYNC1;
    eve_params.voffset = DISP_VOFFSET;
    eve_params.vcycle = DISP_VCYCLE;
    eve_params.hsync0 = DISP_HSYNC0;
    eve_params.hsync1 = DISP_HSYNC1;
    eve_params.hoffset = DISP_HOFFSET;
    eve_params.hcycle = DISP_HCYCLE;
    eve_params.pclk = DISP_PCLK;
    eve_params.pclkpol = DISP_PCLKPOL;
    eve_params.swizzle = DISP_SWIZZLE;
    eve_params.cspread = DISP_CSPREAD;
    eve_params.has_crystal = true;
    eve_params.has_gt911 = false;
    eve_params.backlight_freq = 50000;
    eve_params.backlight_pwm = 255;


	int ret;
	bool led_state = true;

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

	/* Check if devices are ready */
    if (!spi_is_ready_dt(&eve_spi) || !gpio_is_ready_dt(&eve_pd)) {
        return -1; 
    }

    lv_display_t * disp;

    phost = &host;

//     /* Proven Riverdi EVE init */
      App_Common_Init(phost);

// //     /* LVGL core */
// //     //lv_init();

// //     /* LVGL EVE display backend
// //        Note: internal EVE_init() inside LVGL renderer must be disabled */
// //     // disp = lv_draw_eve_display_create(&eve_params, eve_op_cb, phost);
// //     // (void)disp;

//     /* --- HARDWARE PROOF TEST --- */
//     /* 1. Force the GPIO pins to be outputs, then turn them HIGH.
//         This keeps the physical LCD "DISP" (Display Enable) pin powered on. */
     Gpu_Hal_Wr16(phost, REG_GPIOX_DIR, 0xFFFF);
     Gpu_Hal_Wr16(phost, REG_GPIOX, 0xFFFF);

//     /* 2. Manually write the Red Screen Display List */
     Gpu_Hal_Wr32(phost, RAM_DL + 0, 0x0200FF00); /* CLEAR_COLOR_RGB (Red) */
     Gpu_Hal_Wr32(phost, RAM_DL + 4, 0x26000007); /* CLEAR */
     Gpu_Hal_Wr32(phost, RAM_DL + 8, 0x00000000); /* DISPLAY */

//     /* 3. Swap the active frame buffer to show our new list */
     Gpu_Hal_Wr8(phost, REG_DLSWAP, DLSWAP_FRAME);

//     /* 4. Turn on the Pixel Clock so the EVE chip starts sending pixels to the glass.
//         (If REG_PCLK is 0, the screen stays permanently blank) */
     Gpu_Hal_Wr8(phost, REG_PCLK, 5); 

//     /* 5. Turn the Backlight PWM up to 100% */
     Gpu_Hal_Wr16(phost, REG_PWM_DUTY, 128);

    /* UI */
    // create_test_ui();
    // lv_refr_now(NULL);


/* 1. Hardware Power-On Reset (0V for 20ms, then 3.3V) */
    // gpio_pin_set_dt(&eve_pd, 1); 
    // k_msleep(20);
    // gpio_pin_set_dt(&eve_pd, 0); 
    // k_msleep(20);

    // // /* 2. Soft Reset & ACTIVE Command to turn on the clock */
    // eve_host_cmd(0x68); /* RST_PULSE */
    // k_msleep(20);
    // eve_host_cmd(0x00); /* ACTIVE */
    // k_msleep(300);      /* Wait for PLL to lock */

    // // /* 3. Write a Blue Screen to the Display List Memory */
    // // /* CLEAR_COLOR_RGB (Blue: 255, Green: 0, Red: 0) */
    // eve_write32(RAM_DL + 0, 0x020000FF); 
    
    // // /* CLEAR (Clear color, stencil, and tag buffers) */
    // eve_write32(RAM_DL + 4, 0x26000007); 
    
    // // /* DISPLAY (End of display list) */
    // eve_write32(RAM_DL + 8, 0x00000000); 

    // // /* 4. Swap the buffer to display it, turn on PCLK, and set backlight to max */
    // eve_write8(REG_DLSWAP, 2);       /* DLSWAP_FRAME */
    // eve_write8(REG_PCLK, 5);         /* Turn on pixel clock */
    // eve_write16(REG_PWM_DUTY, 128);  /* Turn on backlight */

	while (1) {
        lv_tick_inc(5);
        lv_timer_handler();
        k_msleep(5);
		ret = gpio_pin_toggle_dt(&led);
		if (ret < 0) {
			return 0;
		}

		led_state = !led_state;
	}
	return 0;
}
