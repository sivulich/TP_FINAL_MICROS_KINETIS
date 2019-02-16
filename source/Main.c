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
static int play=1,currentScreen=MAIN_SCREEN;
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
static int volume=29;
static int volumeMap[] = {	0,
							141,
							282,
							424,
							565,
							706,
							847,
							989,
							1130,
							1271,
							1412,
							1554,
							1695,
							1836,
							1977,
							2119,
							2260,
							2401,
							2542,
							2684,
							2825,
							2966,
							3107,
							3249,
							3390,
							3531,
							3672,
							3814,
							3955,
							4096};

/*******************************************************************************/

int main(void)
{

	/*Inicialización de la placa*/
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

	/*inicialización de littlevgl*/
	lv_init();

	/*Inicialización de la pantalla*/
	ili9431_init();
	//Registramos la pantalla como output
	lv_disp_drv_init(&disp);
	disp.disp_flush = ili9431_flush;
	//disp.disp_fill = ili9431_fill;
	disp.disp_map = ili9431_map;
	lv_disp_drv_register(&disp);

	/*Inicialización de los inputs*/
	InputHandlerInit(&play,&currentScreen,&volume);
	//Registramos los inputs como un keypad
	kb_drv.type = LV_INDEV_TYPE_KEYPAD;
	kb_drv.read = InputHandlerRead;
	kb_indev = lv_indev_drv_register(&kb_drv);


	/*Inicializamos los ping pong buffers*/
	for(int i=0;i<PING_PONG_BUFFS;i++)
		outBuff[i]=buff+i*MP3_BUFFER_SIZE/2;

	/*Creamos la interfaz de usuario*/
	MP3UiCreate(&kb_drv);

	/*Inicializamos el decodificador MP3 y el generador de señales*/
	MP3DEC.init();
	SigGen.init();

	/*Main loop del programa*/
	while (1) {

		//Si se selecciono un archivo nos devolvera el nombre, si no hay SD -1 y si no se selecciono nada 0
		char* file = getMP3file();
		if(file!=(char*)-1)
		{
			//Si se selecciono un archivo
			if(file!=NULL)
			{
				//Reiniciamos el decodificador MP3
				MP3DEC.unloadFile();
				MP3DEC.loadFile(file);

				//Obtenemos el header extraemos la información y lo salteamos
				dur =0;
				while(dur<=0)
					dur=MP3DEC.decode(outBuff[0],&buffLen);
				MP3UiSetSongInfo((char*)MP3DEC.getMP3Info("TIT2",&len),(char*)MP3DEC.getMP3Info("TPE1",&len),dur/1000,1,volume);
				while(len<=0)
					len=MP3DEC.decode(outBuff[0],&buffLen);
				MP3FrameInfo finfo=MP3DEC.getFrameInfo();

				//Reinicamos el generador de señales y lo configuramos para esta canción
				SigGen.stop();
				SigGen.setupSignal(outBuff,PING_PONG_BUFFS,finfo.outputSamps,finfo.samprate*finfo.nChans);
				len=0;
				pos=0;
				play=1;
				lastStatus=1;

				//Preparamos los ping pong buffers
				for(int i=0;i<PING_PONG_BUFFS;i++)
				{
					len+=MP3DEC.decode(outBuff[i],&buffLen);
					for(int j=0;j<buffLen;j++)
					{
						unsigned temp=(((((int)outBuff[i][j] + 32768))>>4));
						temp*=volumeMap[volume];
						temp>>=12;
						outBuff[i][j]=temp;
					}
				}


				//Arrancamos la señal
				SigGen.start();
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
						{
							unsigned temp=(((((int)outBuff[circ%PING_PONG_BUFFS][i] + 32768))>>4))/2;
							temp*=volumeMap[volume];
							temp>>=12;
							outBuff[circ%PING_PONG_BUFFS][i]=temp;
						}
						len+=ret;
						if(ret>0 && len>200)
						{
							pos+=len;
							MP3UiSetSongInfo(NULL,NULL,pos/1000,0,volume);
							len=0;
						}
						else if(ret<0)
							PRINTF("ERROR DECODING %d\n",ret);
						else if(ret  == 0)
						{
							PRINTF("Finished Decoding File!\n");
							MP3UiSetSongInfo(NULL,NULL,dur/1000,0,volume);
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
		currentScreen=MP3UiGetCurrentScreen();
	}

	printf("Thanks for using MP3\n");
	GETCHAR();
    while (true)
    {
    }
}

