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

	/**********************
	* GLOBAL PROTOTYPES
	**********************/

	/**
	* Create the MP3_UI
	*/
	void MP3UiCreate(lv_indev_drv_t* kb_dr);

	char* getMP3file();

	void MP3UiSetSongInfo(const char* title, const char*artist, int dur);
	/**********************
	*      MACROS
	**********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*MP3UI_H*/
