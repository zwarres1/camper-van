/*

 Copyright (c) Bridgetek Pte Ltd
 Copyright (c) Riverdi Sp. z o.o. sp. k.
 Copyright (c) Lukasz Skalski <contact@lukasz-skalski.com>

 THIS SOFTWARE IS PROVIDED BY BRIDGETEK PTE LTD "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 BRIDGETEK PTE LTD BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 OF SUBSTITUTE GOODS OR SERVICES LOSS OF USE, DATA, OR PROFITS OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 BRIDGETEK DRIVERS MAY BE USED ONLY IN CONJUNCTION WITH PRODUCTS BASED ON BRIDGETEK PARTS.

 BRIDGETEK DRIVERS MAY BE DISTRIBUTED IN ANY FORM AS LONG AS LICENSE INFORMATION IS NOT MODIFIED.

 IF A CUSTOM VENDOR ID AND/OR PRODUCT ID OR DESCRIPTION STRING ARE USED, IT IS THE
 RESPONSIBILITY OF THE PRODUCT MANUFACTURER TO MAINTAIN ANY CHANGES AND SUBSEQUENT WHQL
 RE-CERTIFICATION AS A RESULT OF MAKING THESE CHANGES.

 Abstract:

 Application to demonstrate function of EVE.

 Author : Bridgetek

 Revision History:
 0.1 - date 2017.03.24 - Initial version
 */

#include "platform.h"
#include "App_Common.h"
#include "modules.h"
#include "Gpu.h"
#include "Gpu_Hal.h"

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

	Gpu_Hal_Sleep(100);
	Gpu_HostCommand(GPU_ACTIVE_M);
	Gpu_Hal_Sleep(300);

	Gpu_HostCommand(GPU_INTERNAL_OSC);

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

	Gpu_Hal_Wr32(REG_FREQUENCY, 72000000);

	Gpu_Hal_Wr16(REG_PWM_HZ, 4000);
	Gpu_Hal_Wr8(REG_PWM_DUTY, 30);

	// Gpu_Hal_Wr16(REG_PCLK_FREQ, DispPLCLKFREQ);
	// Gpu_Hal_Wr8(REG_PCLK_2X, DispPCLK2x);

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
	
	/* after this display is visible on the LCD */
	Gpu_Hal_Wr8(REG_PCLK, 1);

	// GPIO directions/backlight
	Gpu_Hal_Wr16(REG_GPIOX_DIR, 0xFFFF);
	Gpu_Hal_Wr16(REG_GPIOX, 0xFFFF);
}

void App_Common_Close() {
	Gpu_Hal_Close();
	Gpu_Hal_DeInit();
}
