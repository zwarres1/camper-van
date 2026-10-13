/*
 * Copyright (c) Riverdi Sp. z o.o. sp. k. <riverdi@riverdi.com>
 * Copyright (c) Skalski Embedded Technologies <contact@lukasz-skalski.com>
 */

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>

/*****************************************************************************/

/* type definitions for EVE HAL library */

#define TRUE		(1)
#define FALSE		(0)

typedef char		bool_t;
typedef unsigned char	uchar8_t;
typedef signed char	schar8_t;
typedef float		float_t;

/* Predefined Riverdi modules */

#ifdef __cplusplus
extern "C" {
#endif

/* Zephyr device specs shared with main */
extern const struct spi_dt_spec eve_spi;
extern const struct gpio_dt_spec eve_cs;
extern const struct gpio_dt_spec eve_pd;

#ifdef __cplusplus
}
#endif

/*****************************************************************************/
typedef enum {
  GPIO_CS   = 0,
  GPIO_PD   = 0xBB,
  GPIO_INT  = 0
} gpio_name;

typedef enum {
  GPIO_HIGH = 1,
  GPIO_LOW  = 0
} gpio_val;

/*****************************************************************************/

bool_t platform_init ();
void platform_sleep_ms (uint32_t);

bool_t platform_spi_init ();
void platform_spi_deinit ();

uchar8_t platform_spi_send_recv_byte (uchar8_t, uint32_t);
uint16_t platform_spi_send_data (uchar8_t*, uint16_t, uint32_t);
void platform_spi_recv_data (uchar8_t*, uint16_t, uint32_t);

bool_t platform_gpio_init (gpio_name);
bool_t platform_gpio_value (gpio_name, gpio_val);

/*****************************************************************************/

#endif /*_PLATFORM_H_*/
