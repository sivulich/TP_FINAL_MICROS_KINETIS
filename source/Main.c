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

#include "MP3UI.h"
#include "lvgl/lvgl.h"
#include "ili9341.h"
#include "InputHandler.h"
#include "MP3Player.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/
static int play=1,currentScreen=MAIN_SCREEN;

//Drivers for input and display
static lv_indev_drv_t kb_drv;
static lv_indev_t * kb_indev;
static lv_disp_drv_t disp;

/*Volume MAP*/
static int volume=30;


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

	/*Creamos la interfaz de usuario*/
	MP3UiCreate(&kb_drv);

	/*Creamos el reproductor*/
	MP3Player.init(&play,&volume);

	/*Main loop del programa*/
	while (1)
	{
		MP3Player.update();
		lv_task_handler();
		currentScreen=MP3UiGetCurrentScreen();
	}

	printf("Thanks for using MP3\n");
	GETCHAR();
}

