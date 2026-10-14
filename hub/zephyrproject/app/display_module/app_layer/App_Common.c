#include "Gpu_Hal.h"
#include "App_Common.h"
#include "modules.h"
#include "Gpu.h"

/*************************************************************************************************
 *                      Application Utilities
 **************************************************************************************************/

static uint8_t sk = 0;
uint8_t App_Read_Tag() {
	static uint8_t Read_tag = 0, temp_tag = 0, ret_tag = 0;
	Read_tag = Gpu_Hal_Rd8(REG_TOUCH_TAG);
	ret_tag = 0;
	if (Read_tag != 0)                           // Allow if the Key is released
			{
		if (temp_tag != Read_tag) {
			temp_tag = Read_tag;
			sk = Read_tag;                 // Load the Read tag to temp variable
		}
	} else {
		if (temp_tag != 0) {
			ret_tag = temp_tag;
			temp_tag = 0; //The event will be processed. Clear the tag
		}
		sk = 0;
	}
	return ret_tag;
}

uint8_t App_Touch_Update(uint8_t *currTag,
		uint16_t *x, uint16_t *y) {
	static uint8_t Read_tag = 0, temp_tag = 0, ret_tag = 0;
	uint32_t touch;
	Read_tag = Gpu_Hal_Rd8(REG_TOUCH_TAG);
	ret_tag = 0;
	if (Read_tag != 0)                           // Allow if the Key is released
			{
		ret_tag = 1;
		if (temp_tag != Read_tag) {
			temp_tag = Read_tag;
			sk = Read_tag;                 // Load the Read tag to temp variable
		}
	} else {
		if (temp_tag != 0) {
			ret_tag = temp_tag;
			temp_tag = 0; //The event will be processed. Clear the tag
		}
		sk = 0;
	}
	*currTag = Read_tag;
	touch = Gpu_Hal_Rd32(REG_TOUCH_SCREEN_XY);
	*x = (uint16_t) (touch >> 16);
	*y = (uint16_t) (touch & 0xFFFF);
	return ret_tag;
}
void App_Play_Sound(uint8_t sound, uint8_t vol,
		uint8_t midi) {
	uint16_t val = (midi << 8) | sound;
	Gpu_Hal_Wr8(REG_VOL_SOUND, vol);
	Gpu_Hal_Wr16(REG_SOUND, val);
	Gpu_Hal_Wr8(REG_PLAY, 1);
}

/******************************************************************************/

void App_Common_Init() {
	uint8_t chipid;

	Gpu_Hal_Powercycle(TRUE);

	Gpu_CoreReset();

	
	Gpu_HostCommand(GPU_EXTERNAL_OSC);
	Gpu_Hal_Wr32(REG_FREQUENCY, 72000000);
	
	Gpu_Hal_Sleep(100);
	Gpu_HostCommand(GPU_ACTIVE_M);
	Gpu_Hal_Sleep(300);
	/* read Register ID to check if chip ID series is correct */
	chipid = Gpu_Hal_Rd8(REG_ID);
	while (chipid != 0x7C) {
		chipid = Gpu_Hal_Rd8(REG_ID);
		Gpu_Hal_Sleep(100);
	}

	/* read REG_CPURESET to confirm 0 is returned */
	{
		uint8_t engine_status;

		/* Read REG_CPURESET to check if engines are ready.
		 Bit 0 for coprocessor engine,
		 Bit 1 for touch engine,
		 Bit 2 for audio engine.
		 */
		engine_status = Gpu_Hal_Rd8(REG_CPURESET);
		while (engine_status != 0x00) {
			engine_status = Gpu_Hal_Rd8(REG_CPURESET);
			Gpu_Hal_Sleep(100);
		}
	}

	Gpu_Hal_Wr16(REG_PWM_HZ, 4000);
	Gpu_Hal_Wr8(REG_PWM_DUTY, 30);

	Gpu_Hal_Wr16(REG_PCLK_FREQ, DispPLCLKFREQ);
	Gpu_Hal_Wr8(REG_PCLK_2X, DispPCLK2x);

	/* configuration of LCD display */
	Gpu_Hal_Wr16(REG_HCYCLE, DispHCycle);
	Gpu_Hal_Wr16(REG_HOFFSET, DispHOffset);
	Gpu_Hal_Wr16(REG_HSYNC0, DispHSync0);
	Gpu_Hal_Wr16(REG_HSYNC1, DispHSync1);
	Gpu_Hal_Wr16(REG_VCYCLE, DispVCycle);
	Gpu_Hal_Wr16(REG_VOFFSET, DispVOffset);
	Gpu_Hal_Wr16(REG_VSYNC0, DispVSync0);
	Gpu_Hal_Wr16(REG_VSYNC1, DispVSync1);
	Gpu_Hal_Wr8(REG_SWIZZLE, DispSwizzle);
	Gpu_Hal_Wr8(REG_PCLK_POL, DispPCLKPol);
	Gpu_Hal_Wr16(REG_HSIZE, DispWidth);
	Gpu_Hal_Wr16(REG_VSIZE, DispHeight);
	Gpu_Hal_Wr16(REG_CSPREAD, DispCSpread);
	Gpu_Hal_Wr16(REG_DITHER, DispDither);

	Gpu_ClearScreen();
	Gpu_Hal_Wr8(REG_PCLK, 2);

	// Turn on the screen
	Gpu_Hal_Wr16(REG_GPIOX, 0x8000);
}

void App_Common_Close() {
	Gpu_Hal_Close();
	Gpu_Hal_DeInit();
}
