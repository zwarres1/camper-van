#include "platform.h"

/*
 * platform_init()
 */
bool_t
platform_init (void *)
{
  return TRUE;
}

/*
 * platform_sleep_ms()
 */
void
platform_sleep_ms (uint32_t ms)
{
  k_msleep(ms);
}

/*
 * platform_spi_init()
 */
bool_t
platform_spi_init (void *)
{
  return TRUE;
}


/*
 * platform_spi_deinit()
 */
void
platform_spi_deinit (void *)
{
  // No-op
}


/*
 * platform_spi_send_recv_byte();
 */
uchar8_t
platform_spi_send_recv_byte (void  *,
                             uchar8_t            data,
                             uint32_t            opt)
{
  uchar8_t rx_data = 0;
  struct spi_buf tx_buf = { .buf = &data, .len = 1 };
  struct spi_buf rx_buf = { .buf = &rx_data, .len = 1 };
  struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };
  struct spi_buf_set rx_set = { .buffers = &rx_buf, .count = 1 };

  spi_transceive_dt(&eve_spi, &tx_set, &rx_set);
  return rx_data;
}


/*
 * platform_spi_send_data()
 */
uint16_t
platform_spi_send_data (void  *,
                        uchar8_t           *data,
                        uint16_t            size,
                        uint32_t            opt)
{
  struct spi_buf tx_buf = {
    .buf = data,
    .len = size,
  };
  struct spi_buf_set tx_set = {
    .buffers = &tx_buf,
    .count = 1,
  };

  if (spi_write_dt(&eve_spi, &tx_set) != 0) {
    return 0;
  }
  return size;
}


/*
 * platform_spi_recv_data()
 */
void
platform_spi_recv_data (void  *,
                        uchar8_t           *data,
                        uint16_t            size,
                        uint32_t            opt)
{
  struct spi_buf rx_buf = {
    .buf = data,
    .len = size,
  };
  struct spi_buf_set rx_set = {
    .buffers = &rx_buf,
    .count = 1,
  };

  spi_read_dt(&eve_spi, &rx_set);
}


/*
 * platform_gpio_init()
 */
bool_t
platform_gpio_init (void *,
                    gpio_name          /*ngpio*/)
{
  return TRUE;
}


/*
 * platform_gpio_value()
 */
bool_t
platform_gpio_value (void  *,
                     gpio_name           ngpio,
                     gpio_val            vgpio)
{
  // Invert the inverted logic
  int zephyr_val = (vgpio == GPIO_LOW) ? 1 : 0;

  switch (ngpio) {
      case GPIO_CS:
          // No-op, controlled by Zephyr
          break;
      case GPIO_PD:
          gpio_pin_set_dt(&eve_pd, zephyr_val);
          break;
          break;
      default:
          return FALSE;
  }
  return TRUE;
}
