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
	/**********************
	* GLOBAL PROTOTYPES
	**********************/

	/**
	* Create the MP3_UI
	*/
	void MP3UiCreate(lv_indev_drv_t* kb_dr);

	char* getMP3file();

	void MP3UiSetSongInfo(const char* title, const char*artist, int dur,int first,int volume);

	int MP3UiGetCurrentScreen();
	/**********************
	*      MACROS
	**********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*MP3UI_H*/
