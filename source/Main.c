/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include "fsl_sd.h"
#include "fsl_debug_console.h"
#include "board.h"

#include "pin_mux.h"
#include "clock_config.h"

#include "MP3Decoder.h"
#include "MP3UI.h"
#include "lvgl/lvgl.h"
#include "ili9341.h"
#include "InputHandler.h"
#include "SigGen.h"
#include "fsl_sdmmc_event.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* buffer size (in byte) for read/write operations */
#define BUFFER_SIZE (100U)
/*******************************************************************************
 * Prototypes


/*******************************************************************************
 * Variables
 ******************************************************************************/



//Drivers for input and display
static lv_indev_drv_t kb_drv;
static lv_indev_t * kb_indev;
static lv_disp_drv_t disp;


/* @brief decription about the read/write buffer
* The size of the read/write buffer should be a multiple of 512, since SDHC/SDXC card uses 512-byte fixed
* block length and this driver example is enabled with a SDHC/SDXC card.If you are using a SDSC card, you
* can define the block length by yourself if the card supports partial access.
* The address of the read/write buffer should align to the specific DMA data buffer address align value if
* DMA transfer is used, otherwise the buffer address is not important.
* At the same time buffer address/size should be aligned to the cache line size if cache is supported.
*/
/*
SDK_ALIGN(uint8_t g_bufferWrite[SDK_SIZEALIGN(BUFFER_SIZE, SDMMC_DATA_BUFFER_ALIGN_CACHE)],
          MAX(SDMMC_DATA_BUFFER_ALIGN_CACHE, SDMMCHOST_DMA_BUFFER_ADDR_ALIGN));
SDK_ALIGN(uint8_t g_bufferRead[SDK_SIZEALIGN(BUFFER_SIZE, SDMMC_DATA_BUFFER_ALIGN_CACHE)],
          MAX(SDMMC_DATA_BUFFER_ALIGN_CACHE, SDMMCHOST_DMA_BUFFER_ADDR_ALIGN));*/
/*! @brief SDMMC host detect card configuration */


/*! @brief SDMMC card power control configuration */
#if defined DEMO_SDCARD_POWER_CTRL_FUNCTION_EXIST
static const sdmmchost_pwr_card_t s_sdCardPwrCtrl = {
    .powerOn = BOARD_PowerOnSDCARD, .powerOnDelay_ms = 500U, .powerOff = BOARD_PowerOffSDCARD, .powerOffDelay_ms = 0U,
};
#endif
/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function
 */

/******************************************************************************/


int checkMP3file(char* fn, unsigned sz)
{
	if (fn[sz - 1] == '3' && fn[sz - 2] == 'p' && fn[sz - 3] == 'm'&& fn[sz - 4] == '.')
		return 1;
	return 0;
}




/*******************************************************************************/
int main(void)
{

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();


	/*Initialize LittlevGL*/
	lv_init();

	ili9431_init();



	lv_disp_drv_init(&disp);
	disp.disp_flush = ili9431_flush;
	disp.disp_fill = ili9431_fill;
	disp.disp_map = ili9431_map;
	lv_disp_drv_register(&disp);

	InputHandlerInit();
	kb_drv.type = LV_INDEV_TYPE_KEYPAD;
	kb_drv.read = InputHandlerRead;
	kb_indev = lv_indev_drv_register(&kb_drv);


	MP3UiCreate(&kb_drv);
	MP3DEC.init();
	SigGen.init();
	int play=1;
	unsigned buffLen;
	short outBuff[2][MP3_BUFFER_SIZE];
	int len=0,dur=0;
	long long pos=0;


	while (1) {
		/* Periodically call the lv_task handler.
		* It could be done in a timer interrupt or an OS task too.*/

		char* file = getMP3file();
		if(file!=(char*)-1)
		{
			if(file!=NULL)
			{
				MP3DEC.unloadFile();
				MP3DEC.loadFile(file);
				dur=MP3DEC.decode(outBuff[0],&buffLen);
				MP3UiSetSongInfo((char*)MP3DEC.getMP3Info("TIT2",&len),(char*)MP3DEC.getMP3Info("TPE1",&len),dur/1000,1);
				while(len<=0)
					len=MP3DEC.decode(outBuff[0],&buffLen);
				MP3FrameInfo finfo=MP3DEC.getFrameInfo();
				SigGen.stop();
				SigGen.setupSignal(outBuff[0],outBuff[1],finfo.outputSamps,finfo.samprate*finfo.nChans);
				SigGen.start();
				len=0;
				pos=0;
			}
			if(play==1 && MP3DEC.onFile()==1)
			{
				int status = SigGen.status();
				if(status!=0)
				{
					int currBuff = status-1;
					int ret = MP3DEC.decode(outBuff[currBuff],&buffLen);
					len+=ret;
					if(ret>0 && len>1000)
					{
						pos+=len;
						MP3UiSetSongInfo(NULL,NULL,pos/1000,0);
						len=0;
					}
					else if(ret<0)
						PRINTF("ERROR DECODING %d\n",ret);
					else if(ret  == 0)
					{
						PRINTF("Finished Decoding File!\n");
						MP3UiSetSongInfo(NULL,NULL,dur/1000,0);
						MP3DEC.unloadFile();
						SigGen.stop();
					}
				}


			}
		}
		InputUpdate();
		SigGen.update();
		lv_task_handler();

		//if(SDMMCEVENT_GetMillis()-t>0)
			//lv_tick_inc(SDMMCEVENT_GetMillis()-t);
	}

	printf("Thanks for using MP3\n");
	GETCHAR();
    while (true)
    {
    }
}

