#include "platform.h"
#include "modules.h"
#include "Gpu_Hal.h"
#include "CoPro_Cmds.h"
#include "Gpu.h"

/* --- ZEPHYR DIRECT HARDWARE ACCESS --- */
#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>

/* Must be defined in your main.cpp without the 'static' keyword */
extern const struct spi_dt_spec eve_spi;

/*****************************************************************************/

bool_t Gpu_Hal_Init (Gpu_HalInit_t *halinit)
{
  platform_init (halinit);
  return TRUE;
}

bool_t Gpu_Hal_Open (Gpu_Hal_Context_t *host)
{
  bool_t ret;

  platform_gpio_init (host, GPIO_CS);
  platform_gpio_init (host, GPIO_PD);

  platform_gpio_value (host, GPIO_PD, GPIO_HIGH);
  platform_gpio_value (host, GPIO_CS, GPIO_HIGH);

  ret = platform_spi_init (host);

  host->cmd_fifo_wp = host->dl_buff_wp = 0;
  host->spinumdummy = GPU_SPI_ONEDUMMY;
  host->status = GPU_HAL_OPENED;

  return ret;
}

void Gpu_Hal_Close (Gpu_Hal_Context_t *host)
{
  platform_spi_deinit (host);
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
void Gpu_Hal_StartTransfer (Gpu_Hal_Context_t *host, GPU_TRANSFERDIR_T rw, uint32_t addr) {}
void Gpu_Hal_StartCmdTransfer (Gpu_Hal_Context_t *host, GPU_TRANSFERDIR_T rw, uint16_t count) {}
void Gpu_Hal_TransferString (Gpu_Hal_Context_t *host, const char8_t *string) {}
uint8_t Gpu_Hal_Transfer8 (Gpu_Hal_Context_t *host, uint8_t value) { return 0; }
uint16_t Gpu_Hal_Transfer16 (Gpu_Hal_Context_t *host, uint16_t value) { return 0; }
uint32_t Gpu_Hal_Transfer32 (Gpu_Hal_Context_t *host, uint32_t value) { return 0; }
void Gpu_Hal_EndTransfer (Gpu_Hal_Context_t *host) {}

/*****************************************************************************/
/* MONOLITHIC ZEPHYR BURST READS */
/*****************************************************************************/

uint8_t Gpu_Hal_Rd8 (Gpu_Hal_Context_t *host, uint32_t addr)
{
  uint8_t tx[5] = { (uint8_t)(addr >> 16), (uint8_t)(addr >> 8), (uint8_t)(addr), 0, 0 };
  uint8_t rx[5] = { 0 };

  struct spi_buf tx_buf = { .buf = tx, .len = 5 };
  struct spi_buf rx_buf = { .buf = rx, .len = 5 };
  struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };
  struct spi_buf_set rx_set = { .buffers = &rx_buf, .count = 1 };

  platform_gpio_value (host, GPIO_CS, GPIO_LOW);
  spi_transceive_dt(&eve_spi, &tx_set, &rx_set);
  platform_gpio_value (host, GPIO_CS, GPIO_HIGH);

  return rx[4];
}

uint16_t Gpu_Hal_Rd16 (Gpu_Hal_Context_t *host, uint32_t addr)
{
  uint8_t tx[6] = { (uint8_t)(addr >> 16), (uint8_t)(addr >> 8), (uint8_t)(addr), 0, 0, 0 };
  uint8_t rx[6] = { 0 };

  struct spi_buf tx_buf = { .buf = tx, .len = 6 };
  struct spi_buf rx_buf = { .buf = rx, .len = 6 };
  struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };
  struct spi_buf_set rx_set = { .buffers = &rx_buf, .count = 1 };

  platform_gpio_value (host, GPIO_CS, GPIO_LOW);
  spi_transceive_dt(&eve_spi, &tx_set, &rx_set);
  platform_gpio_value (host, GPIO_CS, GPIO_HIGH);

  return (uint16_t)(rx[4] | (rx[5] << 8));
}

uint32_t Gpu_Hal_Rd32 (Gpu_Hal_Context_t *host, uint32_t addr)
{
  uint8_t tx[8] = { (uint8_t)(addr >> 16), (uint8_t)(addr >> 8), (uint8_t)(addr), 0, 0, 0, 0, 0 };
  uint8_t rx[8] = { 0 };

  struct spi_buf tx_buf = { .buf = tx, .len = 8 };
  struct spi_buf rx_buf = { .buf = rx, .len = 8 };
  struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };
  struct spi_buf_set rx_set = { .buffers = &rx_buf, .count = 1 };

  platform_gpio_value (host, GPIO_CS, GPIO_LOW);
  spi_transceive_dt(&eve_spi, &tx_set, &rx_set);
  platform_gpio_value (host, GPIO_CS, GPIO_HIGH);

  return (uint32_t)(rx[4] | (rx[5] << 8) | (rx[6] << 16) | (rx[7] << 24));
}

/*****************************************************************************/
/* MONOLITHIC ZEPHYR BURST WRITES */
/*****************************************************************************/

void Gpu_Hal_Wr8 (Gpu_Hal_Context_t *host, uint32_t addr, uint8_t v)
{
  uint8_t tx[4] = { (uint8_t)((addr >> 16) | 0x80), (uint8_t)(addr >> 8), (uint8_t)(addr), v };
  struct spi_buf tx_buf = { .buf = tx, .len = 4 };
  struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };

  platform_gpio_value (host, GPIO_CS, GPIO_LOW);
  spi_write_dt(&eve_spi, &tx_set);
  platform_gpio_value (host, GPIO_CS, GPIO_HIGH);
}

void Gpu_Hal_Wr16 (Gpu_Hal_Context_t *host, uint32_t addr, uint16_t v)
{
  uint8_t tx[5] = { (uint8_t)((addr >> 16) | 0x80), (uint8_t)(addr >> 8), (uint8_t)(addr),
                    (uint8_t)(v & 0xFF), (uint8_t)(v >> 8) };
  struct spi_buf tx_buf = { .buf = tx, .len = 5 };
  struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };

  platform_gpio_value (host, GPIO_CS, GPIO_LOW);
  spi_write_dt(&eve_spi, &tx_set);
  platform_gpio_value (host, GPIO_CS, GPIO_HIGH);
}

void Gpu_Hal_Wr32 (Gpu_Hal_Context_t *host, uint32_t addr, uint32_t v)
{
  uint8_t tx[7] = { (uint8_t)((addr >> 16) | 0x80), (uint8_t)(addr >> 8), (uint8_t)(addr),
                    (uint8_t)(v & 0xFF), (uint8_t)((v >> 8) & 0xFF), 
                    (uint8_t)((v >> 16) & 0xFF), (uint8_t)((v >> 24) & 0xFF) };
  struct spi_buf tx_buf = { .buf = tx, .len = 7 };
  struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };

  platform_gpio_value (host, GPIO_CS, GPIO_LOW);
  spi_write_dt(&eve_spi, &tx_set);
  platform_gpio_value (host, GPIO_CS, GPIO_HIGH);
}

/*****************************************************************************/
/* SCATTER-GATHER NATIVE MEMORY STREAMING */
/*****************************************************************************/

void Gpu_Hal_WrMem (Gpu_Hal_Context_t *host, uint32_t addr, const uint8_t *buffer, uint32_t length)
{
  /* 3-byte address header */
  uint8_t header[3] = { (uint8_t)((addr >> 16) | 0x80), (uint8_t)(addr >> 8), (uint8_t)(addr) };

  /* Scatter-Gather streams the header then the payload without copying payload to RAM */
  struct spi_buf tx_bufs[2] = {
      { .buf = header, .len = 3 },
      { .buf = (void *)buffer, .len = length }
  };
  struct spi_buf_set tx_set = { .buffers = tx_bufs, .count = 2 };

  platform_gpio_value (host, GPIO_CS, GPIO_LOW);
  spi_write_dt(&eve_spi, &tx_set);
  platform_gpio_value (host, GPIO_CS, GPIO_HIGH);
}

void Gpu_Hal_RdMem (Gpu_Hal_Context_t *host, uint32_t addr, uint8_t *buffer, uint32_t length)
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

  platform_gpio_value (host, GPIO_CS, GPIO_LOW);
  spi_transceive_dt(&eve_spi, &tx_set, &rx_set);
  platform_gpio_value (host, GPIO_CS, GPIO_HIGH);
}

/*****************************************************************************/

void Gpu_HostCommand (Gpu_Hal_Context_t *host, uint8_t cmd)
{
  uint8_t tx[3] = { cmd, 0x00, 0x00 };
  struct spi_buf tx_buf = { .buf = tx, .len = 3 };
  struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };

  platform_gpio_value (host, GPIO_CS, GPIO_LOW);
  spi_write_dt(&eve_spi, &tx_set);
  platform_gpio_value (host, GPIO_CS, GPIO_HIGH);
}

void Gpu_HostCommand_Ext3 (Gpu_Hal_Context_t *host, uint32_t cmd)
{
  uint8_t tx[3] = { (uint8_t)cmd, (uint8_t)(cmd >> 8), (uint8_t)(cmd >> 16) };
  struct spi_buf tx_buf = { .buf = tx, .len = 3 };
  struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };

  platform_gpio_value (host, GPIO_CS, GPIO_LOW);
  spi_write_dt(&eve_spi, &tx_set);
  platform_gpio_value (host, GPIO_CS, GPIO_HIGH);
}

/*****************************************************************************/

void Gpu_Hal_Powercycle (Gpu_Hal_Context_t *host, bool_t up)
{
  if (up)
    {
      platform_gpio_value (host, GPIO_PD, GPIO_LOW);
      Gpu_Hal_Sleep(20);
      platform_gpio_value (host, GPIO_PD, GPIO_HIGH);
      Gpu_Hal_Sleep(20);
    }
  else
    {
      platform_gpio_value (host, GPIO_PD, GPIO_HIGH);
      Gpu_Hal_Sleep(20);
      platform_gpio_value (host, GPIO_PD, GPIO_LOW);
      Gpu_Hal_Sleep(20);
    }
}

void Gpu_Hal_Sleep (uint32_t ms)
{
  platform_sleep_ms (ms);
}

/*****************************************************************************/

void Gpu_Hal_Updatecmdfifo (Gpu_Hal_Context_t *host, uint32_t count)
{
  host->cmd_fifo_wp  = (host->cmd_fifo_wp + count) & FIFO_SIZE_MASK;
  host->cmd_fifo_wp = (host->cmd_fifo_wp + 3) & FIFO_BYTE_ALIGNMENT_MASK;
  Gpu_Hal_Wr16(host,REG_CMD_WRITE,host->cmd_fifo_wp);
}

uint16_t Gpu_Cmdfifo_Freespace (Gpu_Hal_Context_t *host)
{
  uint16_t fullness,retval;
  fullness = (host->cmd_fifo_wp-Gpu_Hal_Rd16(host,REG_CMD_READ))&FIFO_SIZE_MASK;
  retval = (CMD_FIFO_SIZE - 4) - fullness;
  return (retval);
}

void Gpu_Hal_WrCmdBuf (Gpu_Hal_Context_t *host, uint8_t *buffer, uint32_t count)
{
  uint32_t length = 0, availablefreesize;

  do
    {
      length = count;
      availablefreesize = Gpu_Cmdfifo_Freespace(host);

      if (length > availablefreesize)
        length = availablefreesize;

      Gpu_Hal_CheckCmdBuffer(host,length);
      
      /* Direct burst write using our new scatter-gather WrMem! */
      uint32_t addr = host->cmd_fifo_wp + RAM_CMD;
      Gpu_Hal_WrMem(host, addr, buffer, length);

      Gpu_Hal_Updatecmdfifo(host,length);
      Gpu_Hal_WaitCmdfifo_empty(host);

      buffer += length;
      count -= length;

    } while (count > 0);
}

void Gpu_Hal_CheckCmdBuffer (Gpu_Hal_Context_t *host, uint32_t count)
{
  uint16_t getfreespace;
  do
    {
      getfreespace = Gpu_Cmdfifo_Freespace(host);
    } while(getfreespace < count);
}

void Gpu_Hal_WaitCmdfifo_empty (Gpu_Hal_Context_t *host)
{
  while(Gpu_Hal_Rd16(host,REG_CMD_READ) != Gpu_Hal_Rd16(host,REG_CMD_WRITE));
  host->cmd_fifo_wp = Gpu_Hal_Rd16(host,REG_CMD_WRITE);
}

void Gpu_Hal_WrCmdBuf_nowait (Gpu_Hal_Context_t *host, uint8_t *buffer, uint32_t count)
{
  uint32_t length = 0, availablefreesize;

  do
    {
      length = count;
      availablefreesize = Gpu_Cmdfifo_Freespace(host);

      if (length > availablefreesize)
        length = availablefreesize;

      Gpu_Hal_CheckCmdBuffer(host,length);

      /* Direct burst write */
      uint32_t addr = host->cmd_fifo_wp + RAM_CMD;
      Gpu_Hal_WrMem(host, addr, buffer, length);

      Gpu_Hal_Updatecmdfifo(host,length);

      buffer += length;
      count -= length;

    } while (count > 0);
}

uint8_t Gpu_Hal_WaitCmdfifo_empty_status (Gpu_Hal_Context_t *host)
{
  if(Gpu_Hal_Rd16(host,REG_CMD_READ) != Gpu_Hal_Rd16(host,REG_CMD_WRITE))
    {
      return 0;
    }
  else
    {
      host->cmd_fifo_wp = Gpu_Hal_Rd16(host,REG_CMD_WRITE);
      return 1;
    }
}

void Gpu_Hal_WaitLogo_Finish (Gpu_Hal_Context_t *host)
{
  int16_t cmdrdptr,cmdwrptr;
  do
    {
      cmdrdptr = Gpu_Hal_Rd16(host,REG_CMD_READ);
      cmdwrptr = Gpu_Hal_Rd16(host,REG_CMD_WRITE);
    } while ((cmdwrptr != cmdrdptr) || (cmdrdptr != 0));

  host->cmd_fifo_wp = 0;
}

void Gpu_Hal_ResetCmdFifo (Gpu_Hal_Context_t *host)
{
  host->cmd_fifo_wp = 0;
}

void Gpu_Hal_WrCmd32 (Gpu_Hal_Context_t *host, uint32_t cmd)
{
  Gpu_Hal_CheckCmdBuffer(host,sizeof(cmd));
  Gpu_Hal_Wr32(host,RAM_CMD + host->cmd_fifo_wp,cmd);
  Gpu_Hal_Updatecmdfifo(host,sizeof(cmd));
}

/*****************************************************************************/

void Fifo_Init (Fifo_t *pFifo, uint32_t StartAddress, uint32_t Length, uint32_t HWReadRegAddress, uint32_t HWWriteRegAddress)
{
  pFifo->fifo_buff = StartAddress;
  pFifo->fifo_len = Length;
  pFifo->fifo_rp = pFifo->fifo_wp = 0;
  pFifo->HW_Read_Reg = HWReadRegAddress;
  pFifo->HW_Write_Reg = HWWriteRegAddress;
}

void Fifo_Update (Gpu_Hal_Context_t *host, Fifo_t *pFifo)
{
  pFifo->fifo_rp = Gpu_Hal_Rd32(host,pFifo->HW_Read_Reg);
}

uint32_t Fifo_Write (Gpu_Hal_Context_t *host, Fifo_t *pFifo, uint8_t *buffer, uint32_t NumbytetoWrite)
{
  uint32_t FreeSpace = Fifo_GetFreeSpace(host,pFifo),TotalBytes = NumbytetoWrite;

  if(NumbytetoWrite > FreeSpace)
    {
      Fifo_Update(host,pFifo);
      FreeSpace = Fifo_GetFreeSpace(host,pFifo);

      if(NumbytetoWrite > FreeSpace)
        TotalBytes = FreeSpace;
    }

  if(TotalBytes <= 0)
    return 0; 

  if(pFifo->fifo_wp + TotalBytes >= pFifo->fifo_len)
    {
      uint32_t partialchunk, secpartialchunk;
      partialchunk = pFifo->fifo_len - pFifo->fifo_wp;
      secpartialchunk = TotalBytes - partialchunk;

      Gpu_Hal_WrMem(host,pFifo->fifo_buff + pFifo->fifo_wp,buffer,partialchunk);
      if(secpartialchunk > 0)
        Gpu_Hal_WrMem(host,pFifo->fifo_buff,buffer + partialchunk,secpartialchunk);

      pFifo->fifo_wp = secpartialchunk;
    }
  else
    {
      Gpu_Hal_WrMem(host,pFifo->fifo_buff + pFifo->fifo_wp,buffer,TotalBytes);
      pFifo->fifo_wp += TotalBytes;
    }

  Gpu_Hal_Wr32(host,pFifo->HW_Write_Reg,pFifo->fifo_wp);

  return TotalBytes;
}

void Fifo_Write32 (Gpu_Hal_Context_t *host, Fifo_t *pFifo, uint32_t WriteWord)
{
  Fifo_WriteWait(host,pFifo,(uint8_t *)&WriteWord,4);
}

void Fifo_WriteWait (Gpu_Hal_Context_t *host, Fifo_t *pFifo, uint8_t *buffer, uint32_t Numbyte)
{
  uint32_t TotalBytes = Numbyte,currchunk = 0,FreeSpace;
  uint8_t *pbuff = buffer;

  while(TotalBytes > 0)
    {
      currchunk = TotalBytes;
      FreeSpace = Fifo_GetFreeSpace(host,pFifo);

      if(currchunk > FreeSpace)
        currchunk = FreeSpace;

      Fifo_Write(host,pFifo,pbuff,currchunk);
      pbuff += currchunk;
      TotalBytes -= currchunk;
    }
}

uint32_t Fifo_GetFreeSpace (Gpu_Hal_Context_t *host, Fifo_t *pFifo)
{
  uint32_t FreeSpace = 0;

  Fifo_Update(host,pFifo);

  if(pFifo->fifo_wp >= pFifo->fifo_rp)
    FreeSpace = pFifo->fifo_len - pFifo->fifo_wp + pFifo->fifo_rp;
  else
    FreeSpace = pFifo->fifo_rp - pFifo->fifo_wp;

  if(FreeSpace >= 4)
    FreeSpace -= 4;

  return FreeSpace;
}

/*****************************************************************************/

int32_t Gpu_Hal_Dec2Ascii (char8_t *pSrc, int32_t value)
{
    int16_t Length;
    char8_t *pdst,charval;
    int32_t CurrVal = value,tmpval,i;
    char8_t tmparray[16];
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
  0,255,0,2,  //GPU instruction CLEAR_COLOR_RGB
  7,0,0,38, //GPU instruction CLEAR
  0,0,0,0,  //GPU instruction DISPLAY
};

void Gpu_ClearScreen (Gpu_Hal_Context_t *host)
{
  Gpu_Hal_WrMem(host, RAM_DL,(uint8_t *)DLCODE_BOOTUP,sizeof(DLCODE_BOOTUP));
  Gpu_Hal_Wr8(host, REG_DLSWAP,DLSWAP_FRAME);
}

void Gpu_Hal_DLSwap (Gpu_Hal_Context_t *host, uint8_t DL_Swap_Type)
{
  uint8_t Swap_Type = DLSWAP_FRAME,Swap_Done = DLSWAP_FRAME;

  if (DL_Swap_Type == DLSWAP_LINE)
    Swap_Type = DLSWAP_LINE;

  Gpu_Hal_Wr8(host,REG_DLSWAP,Swap_Type);

  while(Swap_Done)
    {
      Swap_Done = Gpu_Hal_Rd8(host,REG_DLSWAP);
      if(DLSWAP_DONE != Swap_Done)
        Gpu_Hal_Sleep(1);
    }
}

void Gpu_Hal_ResetDLBuffer(Gpu_Hal_Context_t *host)
{
  host->dl_buff_wp = 0;
}

void Gpu_ClockSelect (Gpu_Hal_Context_t *host, GPU_PLL_SOURCE_T pllsource)
{
  Gpu_HostCommand(host,pllsource);
}

void Gpu_PLL_FreqSelect (Gpu_Hal_Context_t *host, GPU_PLL_FREQ_T freq)
{
  Gpu_HostCommand(host,freq);
}

void Gpu_PowerModeSwitch (Gpu_Hal_Context_t *host, GPU_POWER_MODE_T pwrmode)
{
  Gpu_HostCommand(host,pwrmode);
}

void Gpu_CoreReset (Gpu_Hal_Context_t *host)
{
  Gpu_HostCommand(host,0x68);
  Gpu_Hal_Sleep(20);
}

#if (defined(FT81X_ENABLE))  || (defined(BT81X_ENABLE))
void Gpu_81X_SelectSysCLK (Gpu_Hal_Context_t *host, GPU_81X_PLL_FREQ_T freq)
{
  if(GPU_SYSCLK_72M == freq)
    Gpu_HostCommand_Ext3(host, (uint32_t)0x61 | (0x40 << 8) | (0x06 << 8));
  else if(GPU_SYSCLK_60M == freq)
    Gpu_HostCommand_Ext3(host, (uint32_t)0x61 | (0x40 << 8) | (0x05 << 8));
  else if(GPU_SYSCLK_48M == freq)
    Gpu_HostCommand_Ext3(host, (uint32_t)0x61 | (0x40 << 8) | (0x04 << 8));
  else if(GPU_SYSCLK_36M == freq)
    Gpu_HostCommand_Ext3(host, (uint32_t)0x61 | (0x03 << 8));
  else if(GPU_SYSCLK_24M == freq)
    Gpu_HostCommand_Ext3(host, (uint32_t)0x61 | (0x02 << 8));
  else if(GPU_SYSCLK_DEFAULT == freq)
    Gpu_HostCommand_Ext3(host, 0x61);
}

void Gpu_81X_PowerOffComponents (Gpu_Hal_Context_t *host, uint8_t val)
{
  Gpu_HostCommand_Ext3(host, (uint32_t)0x49 | (val<<8));
}

void Gpu_81X_PadDriveStrength (Gpu_Hal_Context_t *host, GPU_81X_GPIO_DRIVE_STRENGTH_T strength, GPU_81X_GPIO_GROUP_T group)
{
  Gpu_HostCommand_Ext3(host, (uint32_t)0x70 | (group << 8) | (strength << 8));
}

void Gpu_81X_ResetActive (Gpu_Hal_Context_t *host)
{
  Gpu_HostCommand_Ext3(host, GPU_81X_RESET_ACTIVE);
}

void Gpu_81X_ResetRemoval (Gpu_Hal_Context_t *host)
{
  Gpu_HostCommand_Ext3(host, GPU_81X_RESET_REMOVAL);
}

int16_t Gpu_Hal_SetSPI (Gpu_Hal_Context_t *host, GPU_SPI_NUMCHANNELS_T numchnls, GPU_SPI_NUMDUMMYBYTES numdummy)
{
  uint8_t writebyte = 0;

  if((numchnls > GPU_SPI_QUAD_CHANNEL) || (numdummy > GPU_SPI_TWODUMMY) || (numdummy < GPU_SPI_ONEDUMMY))
    return -1;

  writebyte = numchnls;
  if(numdummy == GPU_SPI_TWODUMMY)
    writebyte |= SPI_TWO_DUMMY_BYTE;
  Gpu_Hal_Wr8(host, REG_SPI_WIDTH, writebyte);

  host->spichannel = numchnls;
  host->spinumdummy = numdummy;

  return 0;
}
#endif