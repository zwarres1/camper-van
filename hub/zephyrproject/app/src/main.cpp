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
    extern const struct spi_dt_spec eve_spi;
    extern const struct gpio_dt_spec eve_pd;
}

int main(void)
{
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

//     /* Proven Riverdi EVE init */
      App_Common_Init();

// //     /* LVGL core */
// //     //lv_init();

// //     /* LVGL EVE display backend
// //        Note: internal EVE_init() inside LVGL renderer must be disabled */
// //     // disp = lv_draw_eve_display_create(&eve_params, eve_op_cb, phost);
// //     // (void)disp;

//     /* --- HARDWARE PROOF TEST --- */
//     /* 1. Force the GPIO pins to be outputs, then turn them HIGH.
//         This keeps the physical LCD "DISP" (Display Enable) pin powered on. */
     Gpu_Hal_Wr16(REG_GPIOX_DIR, 0xFFFF);
     Gpu_Hal_Wr16(REG_GPIOX, 0xFFFF);

//     /* 2. Manually write the Red Screen Display List */
     Gpu_Hal_Wr32(RAM_DL + 0, 0x0200FF00); /* CLEAR_COLOR_RGB (Red) */
     Gpu_Hal_Wr32(RAM_DL + 4, 0x26000007); /* CLEAR */
     Gpu_Hal_Wr32(RAM_DL + 8, 0x00000000); /* DISPLAY */

//     /* 3. Swap the active frame buffer to show our new list */
     Gpu_Hal_Wr8(REG_DLSWAP, DLSWAP_FRAME);

//     /* 4. Turn on the Pixel Clock so the EVE chip starts sending pixels to the glass.
//         (If REG_PCLK is 0, the screen stays permanently blank) */
     Gpu_Hal_Wr8(REG_PCLK, 5); 

//     /* 5. Turn the Backlight PWM up to 100% */
     Gpu_Hal_Wr16(REG_PWM_DUTY, 128);

    /* UI */
    // create_test_ui();
    // lv_refr_now(NULL);

	while (1) {
        // lv_tick_inc(5);
        // lv_timer_handler();
        k_msleep(1000);
		ret = gpio_pin_toggle_dt(&led);

        Gpu_Hal_Wr32(RAM_DL + 0, 0x0200FF00); /* CLEAR_COLOR_RGB (Red) */
        Gpu_Hal_Wr32(RAM_DL + 4, 0x26000007); /* CLEAR */
        Gpu_Hal_Wr32(RAM_DL + 8, 0x00000000); /* DISPLAY */

    //     /* 3. Swap the active frame buffer to show our new list */
        Gpu_Hal_Wr8(REG_DLSWAP, DLSWAP_FRAME);

        k_msleep(1000);

        Gpu_Hal_Wr32(RAM_DL + 0, 0x020000FF); /* CLEAR_COLOR_RGB (Red) */
        Gpu_Hal_Wr32(RAM_DL + 4, 0x38000007); /* CLEAR */
        Gpu_Hal_Wr32(RAM_DL + 8, 0x00000000); /* DISPLAY */

    //     /* 3. Swap the active frame buffer to show our new list */
        Gpu_Hal_Wr8(REG_DLSWAP, DLSWAP_FRAME);

		if (ret < 0) {
			return 0;
		}

		led_state = !led_state;
	}
	return 0;
}
