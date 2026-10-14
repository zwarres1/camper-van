#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "modules.h"
#include "Gpu_Hal.h"
#include "Gpu.h"

/* --- ZEPHYR DIRECT HARDWARE ACCESS --- */
#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>

/* EVE chips use SPI Mode 0 (CPOL=0, CPHA=0), MSB first */
const struct spi_dt_spec eve_spi = SPI_DT_SPEC_GET(
    DT_NODELABEL(eve_display), 
    SPI_WORD_SET(8) | SPI_TRANSFER_MSB
);

/* Grabs the Power Down and chip select pins from the Devicetree alias */
const struct gpio_dt_spec eve_pd = GPIO_DT_SPEC_GET(DT_ALIAS(eve_pd), gpios);

/*****************************************************************************/

bool_t Gpu_Hal_Init (Gpu_HalInit_t *halinit)
{
  return TRUE;
}

bool_t Gpu_Hal_Open (Gpu_Hal_Context_t *host)
{
  gpio_pin_set_dt(&eve_pd, 1);

  return true;
}

void Gpu_Hal_Close (Gpu_Hal_Context_t *host)
{
  host->status = GPU_HAL_CLOSED;
}

void Gpu_Hal_DeInit()
{
  return;
}

/*****************************************************************************/
/* DEPRECATED LEGACY STREAMING API 
 * We leave these as empty stubs so legacy Riverdi helper files still compile, 
 * but the architecture now uses direct monolithic addressing for everything. 
 */
void Gpu_Hal_StartTransfer (GPU_TRANSFERDIR_T rw, uint32_t addr) {}
void Gpu_Hal_StartCmdTransfer (GPU_TRANSFERDIR_T rw, uint16_t count) {}
void Gpu_Hal_TransferString (const char *string) {}
uint8_t Gpu_Hal_Transfer8 (uint8_t value) { return 0; }
uint16_t Gpu_Hal_Transfer16 (uint16_t value) { return 0; }
uint32_t Gpu_Hal_Transfer32 (uint32_t value) { return 0; }
void Gpu_Hal_EndTransfer (Gpu_Hal_Context_t *host) {}

/*****************************************************************************/
/* MONOLITHIC ZEPHYR BURST READS */
/*****************************************************************************/

uint8_t Gpu_Hal_Rd8 (uint32_t addr)
{
  uint8_t tx[5] = { (uint8_t)(addr >> 16), (uint8_t)(addr >> 8), (uint8_t)(addr), 0, 0 };
  uint8_t rx[5] = { 0 };

  struct spi_buf tx_buf = { .buf = tx, .len = 5 };
  struct spi_buf rx_buf = { .buf = rx, .len = 5 };
  struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };
  struct spi_buf_set rx_set = { .buffers = &rx_buf, .count = 1 };

  spi_transceive_dt(&eve_spi, &tx_set, &rx_set);

  return rx[4];
}

uint16_t Gpu_Hal_Rd16 (uint32_t addr)
{
  uint8_t tx[6] = { (uint8_t)(addr >> 16), (uint8_t)(addr >> 8), (uint8_t)(addr), 0, 0, 0 };
  uint8_t rx[6] = { 0 };

  struct spi_buf tx_buf = { .buf = tx, .len = 6 };
  struct spi_buf rx_buf = { .buf = rx, .len = 6 };
  struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };
  struct spi_buf_set rx_set = { .buffers = &rx_buf, .count = 1 };

  spi_transceive_dt(&eve_spi, &tx_set, &rx_set);

  return (uint16_t)(rx[4] | (rx[5] << 8));
}

uint32_t Gpu_Hal_Rd32 (uint32_t addr)
{
  uint8_t tx[8] = { (uint8_t)(addr >> 16), (uint8_t)(addr >> 8), (uint8_t)(addr), 0, 0, 0, 0, 0 };
  uint8_t rx[8] = { 0 };

  struct spi_buf tx_buf = { .buf = tx, .len = 8 };
  struct spi_buf rx_buf = { .buf = rx, .len = 8 };
  struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };
  struct spi_buf_set rx_set = { .buffers = &rx_buf, .count = 1 };

  spi_transceive_dt(&eve_spi, &tx_set, &rx_set);

  return (uint32_t)(rx[4] | (rx[5] << 8) | (rx[6] << 16) | (rx[7] << 24));
}

/*****************************************************************************/
/* MONOLITHIC ZEPHYR BURST WRITES */
/*****************************************************************************/

void Gpu_Hal_Wr8 (uint32_t addr, uint8_t v)
{
  uint8_t tx[4] = { (uint8_t)((addr >> 16) | 0x80), (uint8_t)(addr >> 8), (uint8_t)(addr), v };
  struct spi_buf tx_buf = { .buf = tx, .len = 4 };
  struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };

  spi_write_dt(&eve_spi, &tx_set);
}

void Gpu_Hal_Wr16 (uint32_t addr, uint16_t v)
{
  uint8_t tx[5] = { (uint8_t)((addr >> 16) | 0x80), (uint8_t)(addr >> 8), (uint8_t)(addr),
                    (uint8_t)(v & 0xFF), (uint8_t)(v >> 8) };
  struct spi_buf tx_buf = { .buf = tx, .len = 5 };
  struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };

  spi_write_dt(&eve_spi, &tx_set);
}

void Gpu_Hal_Wr32 (uint32_t addr, uint32_t v)
{
  uint8_t tx[7] = { (uint8_t)((addr >> 16) | 0x80), (uint8_t)(addr >> 8), (uint8_t)(addr),
                    (uint8_t)(v & 0xFF), (uint8_t)((v >> 8) & 0xFF), 
                    (uint8_t)((v >> 16) & 0xFF), (uint8_t)((v >> 24) & 0xFF) };
  struct spi_buf tx_buf = { .buf = tx, .len = 7 };
  struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };

  spi_write_dt(&eve_spi, &tx_set);
}

/*****************************************************************************/
/* SCATTER-GATHER NATIVE MEMORY STREAMING */
/*****************************************************************************/

void Gpu_Hal_WrMem (uint32_t addr, const uint8_t *buffer, uint32_t length)
{
  /* 3-byte address header */
  uint8_t header[3] = { (uint8_t)((addr >> 16) | 0x80), (uint8_t)(addr >> 8), (uint8_t)(addr) };

  /* Scatter-Gather streams the header then the payload without copying payload to RAM */
  struct spi_buf tx_bufs[2] = {
      { .buf = header, .len = 3 },
      { .buf = (void *)buffer, .len = length }
  };
  struct spi_buf_set tx_set = { .buffers = tx_bufs, .count = 2 };

  spi_write_dt(&eve_spi, &tx_set);
}

void Gpu_Hal_RdMem (uint32_t addr, uint8_t *buffer, uint32_t length)
{
  /* 3-byte address + 1 dummy byte */
  uint8_t header[4] = { (uint8_t)(addr >> 16), (uint8_t)(addr >> 8), (uint8_t)(addr), 0 };

  struct spi_buf tx_bufs[2] = {
      { .buf = header, .len = 4 },
      { .buf = NULL, .len = length } /* NULL means send dummy zeroes during payload read */
  };
  struct spi_buf rx_bufs[2] = {
      { .buf = NULL, .len = 4 },     /* Ignore MISO during address phase */
      { .buf = buffer, .len = length }
  };
  struct spi_buf_set tx_set = { .buffers = tx_bufs, .count = 2 };
  struct spi_buf_set rx_set = { .buffers = rx_bufs, .count = 2 };

  spi_transceive_dt(&eve_spi, &tx_set, &rx_set);
}

/*****************************************************************************/

void Gpu_HostCommand (uint8_t cmd)
{
  uint8_t tx[3] = { cmd, 0x00, 0x00 };
  struct spi_buf tx_buf = { .buf = tx, .len = 3 };
  struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };

  spi_write_dt(&eve_spi, &tx_set);
}

void Gpu_HostCommand_Ext3 (uint32_t cmd)
{
  uint8_t tx[3] = { (uint8_t)cmd, (uint8_t)(cmd >> 8), (uint8_t)(cmd >> 16) };
  struct spi_buf tx_buf = { .buf = tx, .len = 3 };
  struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };

  spi_write_dt(&eve_spi, &tx_set);
}

/*****************************************************************************/

void Gpu_Hal_Powercycle (bool_t up)
{
  if (up)
    {
      gpio_pin_set_dt(&eve_pd, 0);
      Gpu_Hal_Sleep(20);
      gpio_pin_set_dt(&eve_pd, 1);
      Gpu_Hal_Sleep(20);
    }
  else
    {
      gpio_pin_set_dt(&eve_pd, 1);
      Gpu_Hal_Sleep(20);
      gpio_pin_set_dt(&eve_pd, 0);
      Gpu_Hal_Sleep(20);
    }
}

void Gpu_Hal_Sleep (uint32_t ms)
{
  k_msleep(ms);
}

/*****************************************************************************/

int32_t Gpu_Hal_Dec2Ascii (char *pSrc, int32_t value)
{
    int16_t Length;
    char *pdst,charval;
    int32_t CurrVal = value,tmpval,i;
    char tmparray[16];
    uchar8_t idx = 0;

    Length = strlen(pSrc);
    pdst = pSrc + Length;

    if(0 == value)
    {
        *pdst++ = '0';
        *pdst++ = '\0';
        return 0;
    }

    if(CurrVal < 0)
    {
        *pdst++ = '-';
        CurrVal = - CurrVal;
    }
    while(CurrVal > 0){
        tmpval = CurrVal;
        CurrVal /= 10;
        tmpval = tmpval - CurrVal*10;
        charval = '0' + tmpval;
        tmparray[idx++] = charval;
    }

    for(i=0;i<idx;i++)
    {
        *pdst++ = tmparray[idx - i - 1];
    }
    *pdst++ = '\0';

    return 0;
}

const uint8_t DLCODE_BOOTUP[12] =
{
  0,0,0,2,  //GPU instruction CLEAR_COLOR_RGB
  7,0,0,38, //GPU instruction CLEAR
  0,0,0,0,  //GPU instruction DISPLAY
};

void Gpu_ClearScreen ()
{
  Gpu_Hal_WrMem(RAM_DL,(uint8_t *)DLCODE_BOOTUP,sizeof(DLCODE_BOOTUP));
  Gpu_Hal_Wr8(REG_DLSWAP,DLSWAP_FRAME);
}

void Gpu_Hal_DLSwap (uint8_t DL_Swap_Type)
{
  uint8_t Swap_Type = DLSWAP_FRAME,Swap_Done = DLSWAP_FRAME;

  if (DL_Swap_Type == DLSWAP_LINE)
    Swap_Type = DLSWAP_LINE;

  Gpu_Hal_Wr8(REG_DLSWAP,Swap_Type);

  while(Swap_Done)
    {
      Swap_Done = Gpu_Hal_Rd8(REG_DLSWAP);
      if(DLSWAP_DONE != Swap_Done)
        Gpu_Hal_Sleep(1);
    }
}

void Gpu_Hal_ResetDLBuffer(Gpu_Hal_Context_t *host)
{
  host->dl_buff_wp = 0;
}

void Gpu_ClockSelect (GPU_PLL_SOURCE_T pllsource)
{
  Gpu_HostCommand(pllsource);
}

void Gpu_PLL_FreqSelect (GPU_PLL_FREQ_T freq)
{
  Gpu_HostCommand(freq);
}

void Gpu_PowerModeSwitch (GPU_POWER_MODE_T pwrmode)
{
  Gpu_HostCommand(pwrmode);
}

void Gpu_CoreReset ()
{
  Gpu_HostCommand(0x68);
  Gpu_Hal_Sleep(20);
}

#if (defined(FT81X_ENABLE))  || (defined(BT81X_ENABLE))
void Gpu_81X_SelectSysCLK (GPU_81X_PLL_FREQ_T freq)
{
  if(GPU_SYSCLK_72M == freq)
    Gpu_HostCommand_Ext3( (uint32_t)0x61 | (0x40 << 8) | (0x06 << 8));
  else if(GPU_SYSCLK_60M == freq)
    Gpu_HostCommand_Ext3( (uint32_t)0x61 | (0x40 << 8) | (0x05 << 8));
  else if(GPU_SYSCLK_48M == freq)
    Gpu_HostCommand_Ext3( (uint32_t)0x61 | (0x40 << 8) | (0x04 << 8));
  else if(GPU_SYSCLK_36M == freq)
    Gpu_HostCommand_Ext3( (uint32_t)0x61 | (0x03 << 8));
  else if(GPU_SYSCLK_24M == freq)
    Gpu_HostCommand_Ext3( (uint32_t)0x61 | (0x02 << 8));
  else if(GPU_SYSCLK_DEFAULT == freq)
    Gpu_HostCommand_Ext3( 0x61);
}

void Gpu_81X_PowerOffComponents (uint8_t val)
{
  Gpu_HostCommand_Ext3( (uint32_t)0x49 | (val<<8));
}

void Gpu_81X_PadDriveStrength (GPU_81X_GPIO_DRIVE_STRENGTH_T strength, GPU_81X_GPIO_GROUP_T group)
{
  Gpu_HostCommand_Ext3( (uint32_t)0x70 | (group << 8) | (strength << 8));
}

void Gpu_81X_ResetActive (Gpu_Hal_Context_t *host)
{
  Gpu_HostCommand_Ext3( GPU_81X_RESET_ACTIVE);
}

void Gpu_81X_ResetRemoval (Gpu_Hal_Context_t *host)
{
  Gpu_HostCommand_Ext3( GPU_81X_RESET_REMOVAL);
}
#endif