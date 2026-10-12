#include "platform.h"
#include "App_Common.h"
#include <lvgl.h>

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

static lv_draw_eve_parameters_t eve_params = {
    .hor_res = DISP_WIDTH,
    .ver_res = DISP_HEIGHT,
    .vsync0 = DISP_VSYNC0,
    .vsync1 = DISP_VSYNC1,
    .voffset = DISP_VOFFSET,
    .vcycle = DISP_VCYCLE,
    .hsync0 = DISP_HSYNC0,
    .hsync1 = DISP_HSYNC1,
    .hoffset = DISP_HOFFSET,
    .hcycle = DISP_HCYCLE,
    .pclk = DISP_PCLK,
    .pclkpol = DISP_PCLKPOL,
    .swizzle = DISP_SWIZZLE,
    .cspread = DISP_CSPREAD,
    .has_crystal = true,
    .has_gt911 = false,
    .backlight_freq = 50000,
    .backlight_pwm = 255,
};

int main(void)
{
    lv_display_t * disp;

    phost = &host;

    /* Proven Riverdi EVE init */
    App_Common_Init(phost);

    /* LVGL core */
    lv_init();

    /* LVGL EVE display backend
       Note: internal EVE_init() inside LVGL renderer must be disabled */
    disp = lv_draw_eve_display_create(&eve_params, eve_op_cb, phost);
    (void)disp;

    /* UI */
    create_test_ui();
    lv_refr_now(NULL);

    while(1)
    {
        lv_tick_inc(5);
        lv_timer_handler();
        k_m(5);
    }
}

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
