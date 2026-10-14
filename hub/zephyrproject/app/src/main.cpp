#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

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

/* Define the hardware handles with C linkage */
extern "C" {
    extern const struct spi_dt_spec eve_spi;
    extern const struct gpio_dt_spec eve_pd;
}

const uint8_t DL_RED[12]   = { 0, 0, 255, 2,    7, 0, 0, 38,    0, 0, 0, 0 };
const uint8_t DL_GREEN[12] = { 0, 255, 0, 2,    7, 0, 0, 38,    0, 0, 0, 0 };
const uint8_t DL_BLUE[12]  = { 255, 0, 0, 2,    7, 0, 0, 38,    0, 0, 0, 0 };

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

	/* Check if devices are ready */
    if (!spi_is_ready_dt(&eve_spi) || !gpio_is_ready_dt(&eve_pd)) {
        return -1; 
    }

      /* Riverdi EVE init */
      App_Common_Init();

	while (1) {
		ret = gpio_pin_toggle_dt(&led);
        Gpu_Hal_WrMem(RAM_DL, (uint8_t *)DL_RED, 12);
        Gpu_Hal_Wr8(REG_DLSWAP,DLSWAP_FRAME);
        k_msleep(1000);

		ret = gpio_pin_toggle_dt(&led);
        Gpu_Hal_WrMem(RAM_DL, (uint8_t *)DL_GREEN, 12);
        Gpu_Hal_Wr8(REG_DLSWAP,DLSWAP_FRAME);
        k_msleep(1000);

		ret = gpio_pin_toggle_dt(&led);
        Gpu_Hal_WrMem(RAM_DL, (uint8_t *)DL_BLUE, 12);
        Gpu_Hal_Wr8(REG_DLSWAP,DLSWAP_FRAME);
        k_msleep(1000);

		if (ret < 0) {
			return 0;
		}
	}
	return 0;
}
