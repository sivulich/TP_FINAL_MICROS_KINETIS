#include "MP3Ui.h"
#include "UI.h"
#include "MP3PlayerData.h"
#include "MP3Player.h"			//hay que tratar de sacar esto de aca
#include "MP3Equalizer.h"
#include "fsl_rtc.h"
#include <stdio.h>
#include <stdlib.h>

#define STATUS_BAR_HEIGHT 20

static lv_res_t fileScreenUpdate(lv_obj_t* obj);
/*Screens*/
static lv_obj_t *baseScreen,* mainScreen, *equalizerScreen,*filesScreen,*playScreen, *settingsScreen;

/*Status bar info*/
static lv_obj_t* volumeLabel,*dateLabel,*playInfoLabel;
/*Main Screen Info*/
static lv_obj_t *mainAudioBtn,*mainEqBtn,*mainFileBtn,*mainSetBtn;
static lv_obj_t * mainBtns[4];

/*File Screen info*/
static lv_obj_t * currentFileList[MAX_FILES],*fileList[2];//*fileListBackBtn;
static unsigned fileListSz = 0,fileListPointer=0,newFile=0;
static char* newFileName;
static unsigned UIinit=0;

/*Equalizer Screen Info*/
static lv_obj_t *eqBackBtn,*rollers[3];

/*PlayScreen Info*/
static unsigned duration,currentTime;
static lv_obj_t* progressBar,*playBackBtn,*songNameLbl,*artistNameLbl,*prevLbl,*nextLbl,*playPauseLbl;

/*SettingsScreen Info*/
static lv_obj_t* setBackButton, *loopModeBtn;

/*Button styles*/
static lv_style_t style_bg,style_bgg;
static lv_style_t style_btn_rel;
static lv_style_t style_btn_pr;

/*Indev devices*/
static lv_indev_drv_t* kb_drv;
static lv_indev_t* kb_indev;

/*Groups*/
static lv_group_t * groups[SCREENS];

static void hideAllScreens()
{
	lv_obj_set_hidden(mainScreen,1);
	lv_obj_set_hidden(playScreen,1);
	lv_obj_set_hidden(equalizerScreen,1);
	lv_obj_set_hidden(filesScreen,1);
	lv_obj_set_hidden(settingsScreen,1);
}

static void setActiveGroup(int p,int qnt,lv_obj_t** ob)
{
	if (groups[p] != NULL)
		lv_group_del(groups[p]);
	groups[p] = lv_group_create();
	for(int i=0;i<qnt;i++)
		lv_group_add_obj(groups[p], ob[i]);
	lv_indev_set_group(kb_indev, groups[p]);
	for (int i = 0; i < SCREENS; i++)
		if (groups[i] != 0)
		{
			if (i != p)
			{
				lv_group_del(groups[i]);
				groups[i] = 0;
			}
		}
			
	MP3PlayerData.currentScreen=p;
}

static int checkMP3file(char* fn, size_t sz)
{
	if (fn[sz - 1] == '3' && fn[sz - 2] == 'p' && fn[sz - 3] == 'm'&& fn[sz - 4] == '.')
		return 1;
	if (fn[sz - 1] == '3' && fn[sz - 2] == 'P' && fn[sz - 3] == 'M'&& fn[sz - 4] == '.')
			return 1;
	return 0;
}

static lv_res_t retMainScreen(lv_obj_t* obj)
{
	//lv_scr_load(mainScreen);
	hideAllScreens();
	lv_obj_set_hidden(mainScreen,0);
	
	setActiveGroup(MAIN_SCREEN,4,mainBtns);
	return LV_RES_OK;
}

static lv_res_t btn_action(lv_obj_t * btn)
{
	if (btn == mainEqBtn)
	{
		hideAllScreens();
		lv_obj_set_hidden(equalizerScreen,0);
		lv_obj_t* obs[4] = { rollers[0], rollers[1], rollers[2], eqBackBtn};
		setActiveGroup(EQ_SCREEN,4,obs);
		lv_group_set_editing(groups[EQ_SCREEN],false);
	}
	else if (btn == mainFileBtn)
	{
		fileScreenUpdate(NULL);
		hideAllScreens();
		lv_obj_set_hidden(filesScreen,0);
		lv_obj_t * obs[2]={fileList[fileListPointer]};
		setActiveGroup(FILE_SCREEN0 + fileListPointer,1, obs);
		lv_group_set_editing(groups[FILE_SCREEN0+fileListPointer],true);
	}
	else if(btn == mainAudioBtn)
	{
		setActiveGroup(PLAY_SCREEN,1,&playBackBtn);
		hideAllScreens();
		lv_obj_set_hidden(playScreen,0);
	}
	else if(btn == mainSetBtn)
	{
		hideAllScreens();
		lv_obj_set_hidden(settingsScreen,0);
		lv_obj_t * obs[2]={loopModeBtn, setBackButton};
		setActiveGroup(SETTING_SCREEN,2,obs);
	}
	return LV_RES_OK; /*Return OK because the button matrix is not deleted*/
}

static lv_res_t fileScreenUpdate(lv_obj_t* obj)
{
	unsigned pos = 0;

	if(obj!=NULL)
	{
		for (unsigned i = 0; i < fileListSz; i++)
		{
			if (obj == currentFileList[i])
			{
				pos = i;
				break;
			}
		}
		UI.exitFile();
		UI.setPos(pos);
		if (UI.input(UI_SELECT) == 1)
		{
			retMainScreen(NULL);
			return LV_RES_OK;
		}
	}

	if(UIinit==2)
		UIinit=UI.init();
	char* currFile = UI.getFile();
	if(currFile==(char*)-1)
	{
		UIinit=2;
	}

	if (currFile != 0 && currFile!=(char*)-1)
	{
		newFile=1;
		newFileName=currFile;
		setActiveGroup(PLAY_SCREEN,1,&playBackBtn);
		hideAllScreens();
		lv_obj_set_hidden(playScreen,0);
	}
	else
	{
		MP3Player.update();
		//hide the current list and change to the other

		lv_obj_set_hidden(fileList[fileListPointer], true);
		fileListPointer = (fileListPointer + 1) % 2;
		lv_obj_set_hidden(fileList[fileListPointer], false);
		
		//delete the list and group to re-create them 
		int width=lv_obj_get_width(fileList[fileListPointer]);
		int height=lv_obj_get_height(fileList[fileListPointer]);
		lv_obj_del(fileList[fileListPointer]);
		MP3Player.update();
		//re-creating...
		fileList[fileListPointer] = lv_list_create(filesScreen, NULL);
		lv_obj_set_height(fileList[fileListPointer], height);
		lv_obj_set_width(fileList[fileListPointer], width);
		lv_list_set_anim_time(fileList[fileListPointer], 0);
		lv_obj_t * obs[2]={fileList[fileListPointer]};
		setActiveGroup(FILE_SCREEN0 + fileListPointer,1, obs);
		lv_group_set_editing(groups[FILE_SCREEN0+fileListPointer],true);

		lv_obj_align(fileList[fileListPointer], NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
		currarr current = UI.getCurrent(&fileListSz, &pos);
		MP3Player.update();
		for (unsigned i = 0; i < fileListSz; i++)
		{
			if (current[i][MAX_FILE_NAME] == 1)
			{
				currentFileList[i] = lv_list_add(fileList[fileListPointer], SYMBOL_DIRECTORY, current[i], fileScreenUpdate);
				lv_btn_set_style(currentFileList[i], LV_BTN_STATE_REL, &style_btn_rel);
				lv_btn_set_style(currentFileList[i], LV_BTN_STATE_PR, &style_btn_pr);
			}
			else if (current[i][MAX_FILE_NAME] == 2)
			{
				currentFileList[i] = lv_list_add(fileList[fileListPointer], SYMBOL_LEFT, "Back", fileScreenUpdate);
				lv_btn_set_style(currentFileList[i], LV_BTN_STATE_REL, &style_btn_rel);
				lv_btn_set_style(currentFileList[i], LV_BTN_STATE_PR, &style_btn_pr);
			}
			else if (current[i][MAX_FILE_NAME] == 3)
			{
				currentFileList[i] = lv_list_add(fileList[fileListPointer], SYMBOL_HOME, "Home", fileScreenUpdate);
				lv_btn_set_style(currentFileList[i], LV_BTN_STATE_REL, &style_btn_rel);
				lv_btn_set_style(currentFileList[i], LV_BTN_STATE_PR, &style_btn_pr);
			}
			else if (current[i][MAX_FILE_NAME] == 4)
			{
				currentFileList[i] = lv_list_add(fileList[fileListPointer], SYMBOL_HOME, current[i], fileScreenUpdate);
				lv_btn_set_style(currentFileList[i], LV_BTN_STATE_REL, &style_btn_rel);
				lv_btn_set_style(currentFileList[i], LV_BTN_STATE_PR, &style_btn_pr);
			}


			MP3Player.update();
		}
		for (unsigned i = 0; i < fileListSz; i++)
		{
			if (current[i][MAX_FILE_NAME] == 0)
			{
				if (checkMP3file(current[i], strlen(current[i])) == 0)
					currentFileList[i] = lv_list_add(fileList[fileListPointer], SYMBOL_FILE, current[i], fileScreenUpdate);
				else
					currentFileList[i] = lv_list_add(fileList[fileListPointer], SYMBOL_AUDIO, current[i], fileScreenUpdate);
				lv_btn_set_style(currentFileList[i], LV_BTN_STATE_REL, &style_btn_rel);
				lv_btn_set_style(currentFileList[i], LV_BTN_STATE_PR, &style_btn_pr);
			}
			else if (current[i][MAX_FILE_NAME] == 5)
			{
				currentFileList[i] = lv_list_add(fileList[fileListPointer], SYMBOL_DOWN, current[i], fileScreenUpdate);
				lv_btn_set_style(currentFileList[i], LV_BTN_STATE_REL, &style_btn_rel);
				lv_btn_set_style(currentFileList[i], LV_BTN_STATE_PR, &style_btn_pr);
			}
			lv_label_set_long_mode(lv_list_get_btn_label(currentFileList[i]), LV_LABEL_LONG_DOT);
			MP3Player.update();
		}
	}
	MP3Player.update();
	return LV_RES_OK;
}
lv_style_t style;
static void MainScreenCreate(void)
{
	mainScreen = lv_cont_create(baseScreen, NULL);
	lv_obj_set_size(mainScreen,LV_HOR_RES,LV_VER_RES-STATUS_BAR_HEIGHT);
	lv_obj_align(mainScreen,NULL,LV_ALIGN_IN_BOTTOM_MID,0,0);
	int width = lv_obj_get_width(mainScreen);
	int height= lv_obj_get_height(mainScreen);
	//lv_obj_set_pos(mainScreen,0,0);
	//static const char * btnm_map[] = { SYMBOL_AUDIO, SYMBOL_EDIT,"\n", SYMBOL_DIRECTORY,SYMBOL_SETTINGS, "" };
	/*Create a default button matrix*/
	//btnm1 = lv_btnm_create(mainScreen, NULL);
	//lv_btnm_set_map(btnm1, btnm_map);
	//lv_btnm_set_action(btnm1, btnm_action);
	//lv_obj_set_size(btnm1, LV_HOR_RES, LV_VER_RES);

	lv_style_copy(&style, &lv_style_plain);
	style.text.color = LV_COLOR_WHITE;
	style.text.font = &lv_font_dejavu_40;

	mainAudioBtn=lv_btn_create(mainScreen,NULL);
	lv_cont_set_fit(mainAudioBtn, false, false);
	lv_obj_set_size(mainAudioBtn,width/2, height/2);
	lv_obj_align(mainAudioBtn,mainScreen,LV_ALIGN_IN_TOP_LEFT,0,0);
	lv_btn_set_action(mainAudioBtn, LV_BTN_ACTION_CLICK, btn_action);
	lv_obj_t* label = lv_label_create(mainAudioBtn, NULL);
	lv_label_set_text(label, SYMBOL_AUDIO);
	lv_label_set_style(label,&style);

	mainEqBtn=lv_btn_create(mainScreen,NULL);
	lv_cont_set_fit(mainEqBtn, false, false);
	lv_obj_set_size(mainEqBtn,width/2, height/2);
	lv_obj_align(mainEqBtn,mainAudioBtn,LV_ALIGN_OUT_RIGHT_MID,0,0);
	lv_btn_set_action(mainEqBtn, LV_BTN_ACTION_CLICK, btn_action);
	label = lv_label_create(mainEqBtn, NULL);
	lv_label_set_text(label, SYMBOL_EDIT);
	lv_label_set_style(label,&style);

	mainFileBtn=lv_btn_create(mainScreen,NULL);
	lv_cont_set_fit(mainFileBtn, false, false);
	lv_obj_set_size(mainFileBtn,width/2, height/2);
	lv_obj_align(mainFileBtn,mainAudioBtn,LV_ALIGN_OUT_BOTTOM_MID,0,0);
	lv_btn_set_action(mainFileBtn, LV_BTN_ACTION_CLICK, btn_action);
	label = lv_label_create(mainFileBtn, NULL);
	lv_label_set_text(label, SYMBOL_DIRECTORY);
	lv_label_set_style(label,&style);

	mainSetBtn=lv_btn_create(mainScreen,NULL);
	lv_cont_set_fit(mainSetBtn, false, false);
	lv_obj_set_size(mainSetBtn,width/2, height/2);
	lv_obj_align(mainSetBtn,mainFileBtn,LV_ALIGN_OUT_RIGHT_MID,0,0);
	lv_btn_set_action(mainSetBtn, LV_BTN_ACTION_CLICK, btn_action);
	label = lv_label_create(mainSetBtn, NULL);
	lv_label_set_text(label, SYMBOL_SETTINGS);
	lv_label_set_style(label,&style);

	mainBtns[0]=mainAudioBtn;
	mainBtns[1]=mainEqBtn;
	mainBtns[2]=mainFileBtn;
	mainBtns[3]=mainSetBtn;

	/*Create a second button matrix with the new styles*/

	for(int i=0;i<4;i++)
	{
		//lv_btn_set_style(mainBtns[i], LV_BTN_STYLE_BG, &style_bg);
		lv_btn_set_style(mainBtns[i], LV_BTN_STYLE_REL, &style_btn_rel);
		lv_btn_set_style(mainBtns[i], LV_BTN_STYLE_PR, &style_btn_pr);
	}

	//lv_indev_set_group(kb_indev_main, groups[MAIN_SCREEN]);
}

static lv_obj_t* createRoller(const char* name, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h)
{
	lv_obj_t *bassRoller = lv_roller_create(equalizerScreen, NULL);
	lv_obj_t *bassLabel = lv_label_create(equalizerScreen, NULL);
	lv_label_set_long_mode(bassLabel, LV_LABEL_LONG_ROLL);
	lv_label_set_align(bassLabel, LV_LABEL_ALIGN_CENTER);
	lv_obj_set_width(bassLabel, w);
	lv_label_set_static_text(bassLabel, name);
	char str[200]="";
	for(int i=-EQ_MAX_DB;i<=EQ_MAX_DB;i+=EQ_STEP_DB)
	{
		char strTemp[20];
		if(i+EQ_STEP_DB<=EQ_MAX_DB)
			sprintf(strTemp,"%d dB\n",i);
		else
			sprintf(strTemp,"%d dB",i);
		strcat(str,strTemp);
	}
	lv_roller_set_options(bassRoller, str);
	lv_roller_set_hor_fit(bassRoller, false);
	lv_obj_set_pos(bassRoller, x, y);
	lv_obj_set_width(bassRoller, w);
	//lv_obj_set_height(bassRoller, h);
	lv_roller_set_visible_row_count(bassRoller, 3);
	lv_roller_set_selected(bassRoller, EQ_MAX_DB, false);
	lv_obj_align(bassLabel, bassRoller, LV_ALIGN_OUT_TOP_MID, 0,0);
	lv_roller_set_style(bassRoller, LV_ROLLER_STYLE_BG, &style_btn_rel);
	lv_roller_set_style(bassRoller, LV_ROLLER_STYLE_SEL, &style_btn_pr);
	return bassRoller;
}

static  lv_obj_t* BackButtonCreate(lv_obj_t* p, lv_res_t (*fn)(lv_obj_t* obj))
{
	lv_obj_t* backBtn = lv_btn_create(p, NULL);
	lv_obj_set_pos(backBtn, 0, 0);
	lv_cont_set_fit(backBtn, false, false);
	lv_obj_set_width(backBtn, LV_HOR_RES / 15);
	lv_obj_set_height(backBtn, LV_HOR_RES / 15);
	lv_btn_set_action(backBtn, LV_BTN_ACTION_CLICK, fn);
	lv_obj_t* lab = lv_label_create(backBtn, NULL);
	lv_obj_set_height(lab, LV_HOR_RES / 15 - 3);
	lv_label_set_text(lab, SYMBOL_LEFT);
	lv_btn_set_style(backBtn, LV_BTN_STATE_REL, &style_btn_rel);
	lv_btn_set_style(backBtn, LV_BTN_STATE_PR, &style_btn_pr);
	return backBtn;
}

static lv_res_t EqualizerScreenCB(lv_obj_t* r)
{
	//int gains[EQ_BANDS];
	for(int i = 0; i < EQ_BANDS; i++)
	{
		char dbs[10];
		lv_roller_get_selected_str(rollers[i],dbs);
		dbs[strlen(dbs)-3]=0;
		MP3PlayerData.equalizeBands[i] = atoi(dbs);
	}
	MP3Equalizer.setGains(MP3PlayerData.equalizeBands);
	return LV_RES_OK;
}

static void EqualizerScreenCreate(void)
{
	equalizerScreen = lv_cont_create(baseScreen, NULL);
	lv_obj_set_size(equalizerScreen,LV_HOR_RES,LV_VER_RES-STATUS_BAR_HEIGHT);
	lv_obj_align(equalizerScreen,NULL,LV_ALIGN_IN_BOTTOM_MID,0,0);
	int width = lv_obj_get_width(equalizerScreen);
	int height= lv_obj_get_height(equalizerScreen);

	lv_obj_set_style(equalizerScreen, &style_bg);
	static char *names[] = { "Bass","Mid","Trebble" };
	rollers[0]  = createRoller(names[0], 0* width / 3, height / 3, width / 3, height * 2 / 3);
	rollers[1]  = createRoller(names[1], 1*width/ 3, height / 3, width / 3, height * 2 / 3);
	rollers[2]  = createRoller(names[2], 2* width / 3, height / 3, width / 3, height * 2 / 3);
	lv_roller_set_action(rollers[0], EqualizerScreenCB);
	lv_roller_set_action(rollers[1], EqualizerScreenCB);
	lv_roller_set_action(rollers[2], EqualizerScreenCB);
	eqBackBtn = BackButtonCreate(equalizerScreen, retMainScreen);
}

lv_res_t LoopModeCB(lv_obj_t * btn)
{
	if(MP3PlayerData.playMode)
		lv_btn_set_state(btn, LV_BTN_STATE_TGL_PR);
	else
		lv_btn_set_state(btn, LV_BTN_STATE_TGL_REL);
	MP3PlayerData.playMode ^= 1;
	return LV_RES_OK;
}

static void SettingsScreenCreate(void)
{
	settingsScreen = lv_cont_create(baseScreen, NULL);
	lv_obj_set_size(settingsScreen,LV_HOR_RES,LV_VER_RES-STATUS_BAR_HEIGHT);
	lv_obj_align(settingsScreen,NULL,LV_ALIGN_IN_BOTTOM_MID,0,0);
	int width = lv_obj_get_width(playScreen);
	int height= lv_obj_get_height(playScreen);

	lv_obj_set_style(settingsScreen, &style_bg);

	lv_obj_t* label;
	loopModeBtn=lv_btn_create(settingsScreen,NULL);
	lv_cont_set_fit(loopModeBtn, false, false);
	lv_obj_set_size(loopModeBtn,width/2, height/2);

	lv_btn_set_state(loopModeBtn, LV_BTN_STATE_TGL_REL);  /*Set toggled state*/
	lv_obj_align(loopModeBtn,settingsScreen,LV_ALIGN_CENTER,0,0);

	lv_btn_set_action(loopModeBtn,LV_BTN_ACTION_CLICK, LoopModeCB);
	label = lv_label_create(loopModeBtn, NULL);
	lv_label_set_text(label, "Loop Mode Enable");

	setBackButton = BackButtonCreate(settingsScreen, retMainScreen);
}

static void FilesScreenCreate(void)
{
	filesScreen = lv_cont_create(baseScreen, NULL);
	lv_obj_set_size(filesScreen,LV_HOR_RES,LV_VER_RES-STATUS_BAR_HEIGHT);
	lv_obj_align(filesScreen,NULL,LV_ALIGN_IN_BOTTOM_MID,0,0);
	lv_obj_set_style(filesScreen, &style_bg);

	int width = lv_obj_get_width(filesScreen);
	int height= lv_obj_get_height(filesScreen);
	unsigned pos;
	currarr current = UI.getCurrent(&fileListSz, &pos);
	fileList[0] = lv_list_create(filesScreen, NULL);
	fileList[1] = lv_list_create(filesScreen, NULL);
	lv_obj_set_height(fileList[0], height);
	lv_obj_set_height(fileList[1], height);
	lv_obj_set_width(fileList[0], width);
	lv_obj_set_width(fileList[1], width);
	lv_obj_set_hidden(fileList[1], true);
	for (unsigned i = 0; i < fileListSz; i++)
	{
		if (current[i][MAX_FILE_NAME] == 1)
		{
			currentFileList[i] = lv_list_add(fileList[fileListPointer], SYMBOL_DIRECTORY, current[i], fileScreenUpdate);
			//lv_btn_set_style(currentFileList[i], LV_BTN_STYLE_BG, &style_bg);
			lv_btn_set_style(currentFileList[i], LV_BTN_STATE_REL, &style_btn_rel);
			lv_btn_set_style(currentFileList[i], LV_BTN_STATE_PR, &style_btn_pr);
		}
	}
	for (unsigned i = 0; i < fileListSz; i++)
	{
		if (current[i][MAX_FILE_NAME] == 0)
		{
			if (checkMP3file(current[i], strlen(current[i])) == 0)
				currentFileList[i] = lv_list_add(fileList[fileListPointer], SYMBOL_FILE, current[i], fileScreenUpdate);
			else
				currentFileList[i] = lv_list_add(fileList[fileListPointer], SYMBOL_AUDIO, current[i], fileScreenUpdate);
			lv_btn_set_style(currentFileList[i], LV_BTN_STATE_REL, &style_btn_rel);
			lv_btn_set_style(currentFileList[i], LV_BTN_STATE_PR, &style_btn_pr);
		}
	}
	lv_obj_align(fileList[fileListPointer], NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
	//fileListBackBtn=BackButtonCreate(filesScreen, retMainScreen);
	groups[FILE_SCREEN0] = lv_group_create();
	lv_group_add_obj(groups[FILE_SCREEN0], fileList[0]);
	//lv_group_add_obj(groups[FILE_SCREEN0], fileListBackBtn);
	//lv_indev_set_group(kb_indev_list, groups[FILE_SCREEN0]);
	//groups[FILE_SCREEN1] = lv_group_create();
	//lv_group_add_obj(groups[FILE_SCREEN1], fileList[1]);
}
static void PlayScreenCreate(void)
{
	playScreen = lv_cont_create(baseScreen, NULL);
	lv_obj_set_size(playScreen,LV_HOR_RES,LV_VER_RES-STATUS_BAR_HEIGHT);
	lv_obj_align(playScreen,NULL,LV_ALIGN_IN_BOTTOM_MID,0,0);

	/*Create label on the screen. By default it will inherit the style of the screen*/

	songNameLbl = lv_label_create(playScreen, NULL);
	int width = lv_obj_get_width(playScreen);
	int height= lv_obj_get_height(playScreen);
	lv_obj_set_width(songNameLbl, width *3/ 4);
	lv_label_set_text(songNameLbl, "Song Name");
	lv_obj_align(songNameLbl, NULL, LV_ALIGN_IN_TOP_MID, 0, lv_obj_get_height(songNameLbl));

	lv_label_set_long_mode(songNameLbl, LV_LABEL_LONG_ROLL);

	artistNameLbl = lv_label_create(playScreen, NULL);
	lv_obj_set_width(artistNameLbl, width*3 / 4);
	lv_label_set_text(artistNameLbl, "Artist");
	lv_obj_align(artistNameLbl, songNameLbl, LV_ALIGN_OUT_BOTTOM_MID, 0, +lv_obj_get_height(artistNameLbl));

	lv_label_set_long_mode(artistNameLbl, LV_LABEL_LONG_ROLL);

	progressBar = lv_bar_create(playScreen, NULL);
	lv_bar_set_range(progressBar, 0, 0);
	lv_obj_set_width(progressBar, width *3/ 4);
	lv_obj_align(progressBar, artistNameLbl, LV_ALIGN_OUT_BOTTOM_MID, 0, +lv_obj_get_height(progressBar)/2);


	prevLbl = lv_label_create(playScreen,NULL);
	lv_label_set_text(prevLbl,SYMBOL_PREV);
	lv_obj_align(prevLbl,NULL,LV_ALIGN_IN_BOTTOM_LEFT,width/3,-5);

	nextLbl = lv_label_create(playScreen,NULL);
	lv_label_set_text(nextLbl,SYMBOL_NEXT);
	lv_obj_align(nextLbl,NULL,LV_ALIGN_IN_BOTTOM_RIGHT,-width/3,-5);

	playPauseLbl = lv_label_create(playScreen,NULL);
	lv_label_set_text(playPauseLbl,SYMBOL_PLAY);
	lv_obj_align(playPauseLbl,NULL,LV_ALIGN_IN_BOTTOM_MID,0,-5);


	playBackBtn=BackButtonCreate(playScreen, retMainScreen);
}

static void MP3UiSetSongInfo(const char* title, const char*artist, int dur,int first,float* eqPoints)
{
	if (first == 1)
	{
		lv_label_set_text(songNameLbl, title);
		lv_obj_set_width(songNameLbl, LV_HOR_RES *3 / 4);
		lv_obj_align(songNameLbl, NULL, LV_ALIGN_IN_TOP_MID, 0, lv_obj_get_height(songNameLbl));

		lv_label_set_text(artistNameLbl, artist);
		lv_obj_set_width(artistNameLbl, LV_HOR_RES *3/ 4);
		lv_obj_align(artistNameLbl, songNameLbl, LV_ALIGN_OUT_BOTTOM_MID, 0, +lv_obj_get_height(artistNameLbl));

		lv_bar_set_range(progressBar, 0, dur);
		lv_bar_set_value(progressBar, 0);
		lv_obj_align(progressBar, artistNameLbl, LV_ALIGN_OUT_BOTTOM_MID, 0, +lv_obj_get_height(progressBar)/2);
		currentTime = 0;
		duration = dur;
	}
	else
	{
		currentTime = dur;
		lv_bar_set_value(progressBar, currentTime);
	}
}

static void StylesInit()
{
	/*Background style*/
	lv_style_copy(&style_bg, &lv_style_plain);
	style_bg.body.main_color = LV_COLOR_SILVER;
	style_bg.body.grad_color = LV_COLOR_SILVER;
	style_bg.body.padding.hor = 0;
	style_bg.body.padding.ver = 0;
	style_bg.body.padding.inner = 0;

	/*General background style*/
	lv_style_copy(&style_bgg, &style_bg);
	style_bgg.body.main_color = LV_COLOR_MAKE(0xFF, 0xFF, 0xFF);
	style_bgg.body.grad_color = LV_COLOR_MAKE(0xFF, 0xFF, 0xFF);
	style_bgg.body.padding.hor = 0;
	style_bgg.body.padding.ver = 0;
	style_bgg.body.padding.inner = 0;

	
	/*Create button realese style*/
	lv_style_copy(&style_btn_rel, &lv_style_btn_rel);
	style_btn_rel.body.main_color = LV_COLOR_MAKE(0x30, 0x30, 0x30);
	style_btn_rel.body.grad_color = LV_COLOR_BLACK;
	style_btn_rel.body.border.color = LV_COLOR_SILVER;
	style_btn_rel.body.border.width = 1;
	style_btn_rel.body.border.opa = LV_OPA_50;
	style_btn_rel.body.radius = 5;

	/*Create button pressed style*/
	lv_style_copy(&style_btn_pr, &style_btn_rel);
	style_btn_pr.body.main_color = LV_COLOR_MAKE(0x55, 0x96, 0xd8);
	style_btn_pr.body.grad_color = LV_COLOR_MAKE(0x37, 0x62, 0x90);
	style_btn_pr.text.color = LV_COLOR_MAKE(0xbb, 0xd5, 0xf1);
}

static void statusBarCreate(void)
{
	volumeLabel = lv_label_create(baseScreen,NULL);
	lv_label_set_text(volumeLabel,SYMBOL_VOLUME_MAX " 15");
	lv_obj_align(volumeLabel, NULL, LV_ALIGN_IN_TOP_RIGHT, -20, 0);

	dateLabel = lv_label_create(baseScreen,NULL);
	lv_label_set_text(dateLabel,"dd/mm/aaaa hh:mm");
	lv_obj_align(dateLabel,NULL,LV_ALIGN_IN_TOP_MID,0,0);

	playInfoLabel = lv_label_create(baseScreen,NULL);
	lv_label_set_text(playInfoLabel,SYMBOL_PLAY" "SYMBOL_LOOP);
	lv_obj_align(playInfoLabel,NULL,LV_ALIGN_IN_TOP_LEFT,10,0);
}


static void MP3UiCreate(lv_indev_drv_t* kb_dr)
{
	kb_drv = kb_dr;
	kb_indev = lv_indev_drv_register(kb_drv);
	UIinit=UI.init();
	StylesInit();
	baseScreen = lv_obj_create(NULL,NULL);
	statusBarCreate();
	MainScreenCreate();
	EqualizerScreenCreate();
	FilesScreenCreate();
	PlayScreenCreate();
	SettingsScreenCreate();
	lv_scr_load(baseScreen);
	hideAllScreens();
	lv_obj_set_hidden(mainScreen,0);
	setActiveGroup(MAIN_SCREEN,4,mainBtns);
	rtc_config_t rtcConf;
	RTC_GetDefaultConfig(&rtcConf);
	RTC_Init(RTC,&rtcConf);
	RTC_SetClockSource(RTC);
	/*rtc_datetime_t date;
	date.year = 2019U;
	date.month = 3U;
	date.day = 14U;
	date.hour = 19U;
	date.minute = 48;
	date.second = 30;*/

	/* RTC time counter has to be stopped before setting the date & time in the TSR register */
	RTC_StopTimer(RTC);

	/* Set RTC time to default */
	//RTC_SetDatetime(RTC, &date);
	RTC_StartTimer(RTC);
	MP3UI.update();
}

static char* getMP3file()
{
	char* ret = UI.getFile();
	if(ret == (char*)-1)
	{
		return (char*)-1;
	}
	else if(newFile==1)
	{
		newFile=0;
		return newFileName;
	}
	return 0;

}

static void getMP3AdjFile(int off,char* dest)
{
	UI.getAdjFile(off,dest);
	while(checkMP3file(dest,strlen(dest))==0 && dest[0]!=0 && dest[0]!=(char)-1)
		UI.getAdjFile(off,dest);
}

static int lastVolume=0;

static void update()
{
	if(lastVolume!=MP3PlayerData.volume)
	{
		lastVolume=MP3PlayerData.volume;
		if(MP3PlayerData.volume == 0)
		{
			lv_label_set_text(volumeLabel,SYMBOL_MUTE" 0");
		}
		else if(MP3PlayerData.volume<(MAX_VOLUME * 2.0 / 3.0))
		{
			char txt[8];
			sprintf(txt,SYMBOL_VOLUME_MID" %d",MP3PlayerData.volume);
			lv_label_set_text(volumeLabel,txt);
		}
		else
		{
			char txt[8];
			sprintf(txt,SYMBOL_VOLUME_MAX" %d",MP3PlayerData.volume);
			lv_label_set_text(volumeLabel,txt);
		}

	}

	if(MP3PlayerData.currentScreen==PLAY_SCREEN)
	{
		if(MP3PlayerData.play==1)
			lv_label_set_text(playPauseLbl,SYMBOL_PAUSE);
		else
			lv_label_set_text(playPauseLbl,SYMBOL_PLAY);
	}

	rtc_datetime_t date;
	char text[50];
	RTC_GetDatetime(RTC,&date);
	sprintf(text,"%02d/%02d/%d %02d:%02d",date.day,date.month,date.year,date.hour,date.minute);
	lv_label_set_text(dateLabel,text);
	if(MP3PlayerData.play==1 && MP3PlayerData.playMode==1)
		lv_label_set_text(playInfoLabel,SYMBOL_PLAY" "SYMBOL_LOOP);
	else if(MP3PlayerData.play==0 && MP3PlayerData.playMode==1)
		lv_label_set_text(playInfoLabel,SYMBOL_PAUSE" "SYMBOL_LOOP);
	else if(MP3PlayerData.play==1 && MP3PlayerData.playMode==0)
		lv_label_set_text(playInfoLabel,SYMBOL_PLAY);
	else if(MP3PlayerData.play==0 && MP3PlayerData.playMode==0)
		lv_label_set_text(playInfoLabel,SYMBOL_PAUSE);



}

MP3UI_ MP3UI = {.init=MP3UiCreate,.getMP3file=getMP3file,.getAdjFile=getMP3AdjFile,.setSongInfo=MP3UiSetSongInfo,.update=update};
