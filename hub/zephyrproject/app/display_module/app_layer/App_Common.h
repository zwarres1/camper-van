#ifndef _APP_COMMON_H_
#define _APP_COMMON_H_

#include "modules.h"
#include "Gpu.h"

#if defined(FT80X_ENABLE)
    #define RAM_G_END_ADDR 0x40000 //General purpose graphics RAM 256 kB
#elif defined(FT81X_ENABLE)
    #define RAM_G_END_ADDR (0x100000) //General purpose graphics RAM 1024 kB
#elif defined(BT81X_ENABLE)
    #define RAM_G_END_ADDR (0x100000) //General purpose graphics RAM 1024 kB
#else
    #error "Should select a GPU chip in Platform.h"
#endif

#define SIZE_HOME_START_ICON (460)
#define SIZE_LOGO (6703)

#define START_ICON_ADDR (RAM_G_END_ADDR  - SIZE_HOME_START_ICON*6) //*6 to Reserve space for inflate images.
#define LOGO_ADDR       (START_ICON_ADDR - SIZE_LOGO)

#define START_ICON_HANDLE 14
#define LOGO_ICON_HANDLE 15

/******************************************************************************/

void App_WrCoCmd_Buffer(uint32_t cmd);
void App_WrDl_Buffer(uint32_t cmd);
void App_WrCoStr_Buffer(const char *s);
void App_Flush_DL_Buffer();
void App_Flush_Co_Buffer();
void App_Flush_Co_Buffer_nowait();
void App_Set_DlBuffer_Index(uint32_t index);
void App_Set_CmdBuffer_Index(uint32_t index);

/******************************************************************************/

uint8_t App_Read_Tag();
uint8_t App_Touch_Update( uint8_t *currTag, uint16_t *x, uint16_t *y);
void App_Play_Sound( uint8_t sound,uint8_t vol,uint8_t midi);
void App_Calibrate_Screen();
void App_Show_Logo();

/******************************************************************************/

void App_Common_Init();
void App_Common_Close();

#endif /* _APP_COMMON_H_ */
