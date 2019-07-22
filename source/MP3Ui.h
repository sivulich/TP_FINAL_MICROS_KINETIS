#pragma once
#ifndef MP3UI_H
#define MP3UI_H

#ifdef __cplusplus
extern "C" {
#endif

	/*********************
	*      INCLUDES
	*********************/

#ifdef LV_CONF_INCLUDE_SIMPLE
#include "lvgl.h"
#include "lv_ex_conf.h"
#else
#include "./lvgl/lvgl.h"
#include "./lv_ex_conf.h"
#endif

	/*********************
	*      DEFINES
	*********************/

	/**********************
	*      TYPEDEFS
	**********************/
	typedef enum{MAIN_SCREEN=0,PLAY_SCREEN,EQ_SCREEN,FILE_SCREEN0,FILE_SCREEN1,SETTING_SCREEN,SCREENS} screenEnum;
	typedef struct MP3UI_ MP3UI_;

	extern MP3UI_ MP3UI;

	struct MP3UI_{
		void (*init)(lv_indev_drv_t* kb_dr);

		char* (*getMP3file)();

		void (*getAdjFile)(int off,char* dest);

		void (*setSongInfo)(const char* title, const char*artist, int dur,int first,float* eqPoints);

		void (*update)();

	};
	/**********************
	* GLOBAL PROTOTYPES
	**********************/


	//int MP3UiGetCurrentScreen();
	/**********************
	*      MACROS
	**********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*MP3UI_H*/
