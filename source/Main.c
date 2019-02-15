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
#define PING_PONG_BUFFS 10
/* buffer size (in byte) for read/write operations */
#define BUFFER_SIZE (100U)
/*******************************************************************************
 * Prototypes


/*******************************************************************************
 * Variables
 ******************************************************************************/
static int play=1;
static unsigned buffLen;
static short buff[MP3_BUFFER_SIZE*PING_PONG_BUFFS/2];
static short* outBuff[PING_PONG_BUFFS];
static int len=0,dur=0,lastStatus;
static long long pos=0;

//Drivers for input and display
static lv_indev_drv_t kb_drv;
static lv_indev_t * kb_indev;
static lv_disp_drv_t disp;


/******************************************************************************/


int checkMP3file(char* fn, unsigned sz)
{
	if (fn[sz - 1] == '3' && fn[sz - 2] == 'p' && fn[sz - 3] == 'm'&& fn[sz - 4] == '.')
		return 1;
	return 0;
}


/*Volume MAP*/
static int volume=20;
static float volumeMap[] = {	0.00398107170553497,
						0.00493291291186596,
						0.00611233145140850,
						0.00757373917589501,
						0.00938455735924933,
						0.0116283271424690,
						0.0144085636600656,
						0.0178535316561522,
						0.0221221629107045,
						0.0274113884733337,
						0.0339652239733048,
						0.0420860271517816,
						0.0521484469766079,
						0.0646167078746697,
						0.0800660264807754,
						0.0992091489534587,
						0.122929233142759,
						0.152320592611433,
						0.188739182213510,
						0.233865154355699,
						0.289780371941764,
						0.359064454018608,
						0.444913785139290,
						0.551288978877067,
						0.683097598641883,
						0.846420565527684,
						1.04879269839711,
						1.29955032877224,
						1.61026202756094,
						1.99526231496888};

/*******************************************************************************/

int main(void)
{

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

	/*Initialize LittlevGL*/
	lv_init();

	ili9431_init();


	for(int i=0;i<PING_PONG_BUFFS;i++)
		outBuff[i]=buff+i*MP3_BUFFER_SIZE/2;


	lv_disp_drv_init(&disp);
	disp.disp_flush = ili9431_flush;
	//disp.disp_fill = ili9431_fill;
	disp.disp_map = ili9431_map;
	lv_disp_drv_register(&disp);

	InputHandlerInit(&play);
	kb_drv.type = LV_INDEV_TYPE_KEYPAD;
	kb_drv.read = InputHandlerRead;
	kb_indev = lv_indev_drv_register(&kb_drv);


	MP3UiCreate(&kb_drv);
	MP3DEC.init();
	SigGen.init();



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
				dur =0;
				while(dur<=0)
					dur=MP3DEC.decode(outBuff[0],&buffLen);
				MP3UiSetSongInfo((char*)MP3DEC.getMP3Info("TIT2",&len),(char*)MP3DEC.getMP3Info("TPE1",&len),dur/1000,1);
				while(len<=0)
					len=MP3DEC.decode(outBuff[0],&buffLen);
				MP3FrameInfo finfo=MP3DEC.getFrameInfo();

				SigGen.stop();
				SigGen.setupSignal(outBuff,PING_PONG_BUFFS,finfo.outputSamps,finfo.samprate*finfo.nChans);
				SigGen.start();
				len=0;
				pos=0;
				play=1;
				for(int i=0;i<PING_PONG_BUFFS;i++)
					len+=MP3DEC.decode(outBuff[i],&buffLen);
				lastStatus=1;
			}
			if(play==1 && MP3DEC.onFile()==1)
			{
				SigGen.start();
				int status = SigGen.status();
				if(status!=0)
				{
					int currBuff = status-1;
					int circ=lastStatus-1;
					if(currBuff<circ)
						currBuff+=PING_PONG_BUFFS;
					while(circ<currBuff)
					{
						circ++;
						int ret = MP3DEC.decode(outBuff[circ%PING_PONG_BUFFS],&buffLen);
						for(int i=0;i<buffLen;i++)
							outBuff[circ%PING_PONG_BUFFS][i]=(((((int)outBuff[circ%PING_PONG_BUFFS][i] + 32768))>>4))/2;
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
							break;
						}

					}
					lastStatus=status;
				}


			}
			else if(play==0)
			{
				SigGen.pause();
			}
		}
		else
		{
			SigGen.stop();
		}
		lv_task_handler();
	}

	printf("Thanks for using MP3\n");
	GETCHAR();
    while (true)
    {
    }
}

