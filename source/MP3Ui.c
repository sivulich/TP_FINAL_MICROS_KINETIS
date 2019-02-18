#include "MP3Ui.h"
#include "UI.h"
#include "MP3Player.h"
#include <stdio.h>

static lv_res_t fileScreenUpdate(lv_obj_t* obj);
/*Screens*/
static lv_obj_t * mainScreen, *equalizerScreen,*filesScreen,*playScreen;

/*Main Screen Info*/
lv_obj_t *mainAudioBtn,*mainEqBtn,*mainFileBtn,*mainSetBtn;
lv_obj_t * mainBtns[4];

/*File Screen info*/
static lv_obj_t * currentFileList[MAX_FILES],*fileList[2];//*fileListBackBtn;
static unsigned fileListSz = 0,fileListPointer=0,newFile=0;
static char* newFileName;
static unsigned UIinit=0;

/*Equalizer Screen Info*/
static lv_obj_t * bassRoller, *midRoller, *trebbleRoller, *eqBackBtn;

/*PlayScreen Info*/
static unsigned duration,currentTime;
static lv_obj_t* progressBar,*playBackBtn,*songNameLbl,*artistNameLbl,*volumeLabel,*eqGraph;
lv_chart_series_t * ser1;

/*Button styles*/
static lv_style_t style_bg,style_bgg;
static lv_style_t style_btn_rel;
static lv_style_t style_btn_pr;

/*Indev devices*/
static lv_indev_drv_t* kb_drv;
static lv_indev_t* kb_indev;

/*Groups*/

static lv_group_t * groups[SCREENS];
static int currentScreen=MAIN_SCREEN;

int MP3UiGetCurrentScreen()
{
	return currentScreen;
}

static void setActiveGroup(int p,int qnt,lv_obj_t** ob)
{
	if (groups[p] != NULL)
		lv_group_del(groups[p]);
	groups[p] = lv_group_create();
	for(int i=0;i<qnt;i++)
		lv_group_add_obj(groups[p], ob[i]);
	for (int i = 0; i < SCREENS; i++)
		if (groups[i] != 0)
		{
			if (i != p)
			{
				lv_group_del(groups[i]);
				groups[i] = 0;
			}
			else
			{
				lv_indev_set_group(kb_indev, groups[i]);
			}
		}
			
	currentScreen=p;
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
	lv_scr_load(mainScreen);
	
	setActiveGroup(MAIN_SCREEN,4,mainBtns);
	return LV_RES_OK;
}

static lv_res_t btn_action(lv_obj_t * btn)
{
	if (btn == mainEqBtn)
	{
		lv_scr_load(equalizerScreen);
		lv_obj_t* obs[4] = { bassRoller, midRoller, trebbleRoller, eqBackBtn};
		setActiveGroup(EQ_SCREEN,4,obs);
	}
	else if (btn == mainFileBtn)
	{
		fileScreenUpdate(NULL);
		lv_scr_load(filesScreen);
		lv_obj_t * obs[2]={fileList[fileListPointer]};
		setActiveGroup(FILE_SCREEN0 + fileListPointer,1, obs);
	}
	else if(btn == mainAudioBtn)
	{
		setActiveGroup(PLAY_SCREEN,1,&playBackBtn);
		lv_scr_load(playScreen);
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

	printf("Current path %s\n", UI.getPath());
	if (currFile != 0 && currFile!=(char*)-1)
	{
		printf("Selected file %s\n", currFile);
		newFile=1;
		newFileName=currFile;
		setActiveGroup(PLAY_SCREEN,1,&playBackBtn);
		lv_scr_load(playScreen);
	}
	else
	{
		MP3Player.update();
		//hide the current list and change to the other
		lv_obj_set_hidden(fileList[fileListPointer], true);
		fileListPointer = (fileListPointer + 1) % 2;
		lv_obj_set_hidden(fileList[fileListPointer], false);
		
		//delete the list and group to re-create them 
		lv_obj_del(fileList[fileListPointer]);
		MP3Player.update();
		//re-creating...
		fileList[fileListPointer] = lv_list_create(filesScreen, NULL);
		lv_obj_set_height(fileList[fileListPointer], LV_VER_RES);
		lv_obj_set_width(fileList[fileListPointer], LV_HOR_RES);
		lv_obj_t * obs[2]={fileList[fileListPointer]};
		setActiveGroup(FILE_SCREEN0 + fileListPointer,1, obs);

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
			MP3Player.update();
		}
	}
	MP3Player.update();
	return LV_RES_OK;
}

static void MainScreenCreate(void)
{
	mainScreen = lv_obj_create(NULL, NULL);
	lv_obj_set_size(mainScreen,LV_HOR_RES,LV_VER_RES);
	//lv_obj_set_pos(mainScreen,0,0);
	//static const char * btnm_map[] = { SYMBOL_AUDIO, SYMBOL_EDIT,"\n", SYMBOL_DIRECTORY,SYMBOL_SETTINGS, "" };
	/*Create a default button matrix*/
	//btnm1 = lv_btnm_create(mainScreen, NULL);
	//lv_btnm_set_map(btnm1, btnm_map);
	//lv_btnm_set_action(btnm1, btnm_action);
	//lv_obj_set_size(btnm1, LV_HOR_RES, LV_VER_RES);
	lv_obj_t* label;
	mainAudioBtn=lv_btn_create(mainScreen,NULL);
	lv_cont_set_fit(mainAudioBtn, false, false);
	lv_obj_set_size(mainAudioBtn,LV_HOR_RES/2, LV_VER_RES/2);
	lv_obj_align(mainAudioBtn,mainScreen,LV_ALIGN_IN_TOP_LEFT,0,0);
	lv_btn_set_action(mainAudioBtn, LV_BTN_ACTION_CLICK, btn_action);
	label = lv_label_create(mainAudioBtn, NULL);
	lv_label_set_text(label, SYMBOL_AUDIO);

	mainEqBtn=lv_btn_create(mainScreen,NULL);
	lv_cont_set_fit(mainEqBtn, false, false);
	lv_obj_set_size(mainEqBtn,LV_HOR_RES/2, LV_VER_RES/2);
	lv_obj_align(mainEqBtn,mainAudioBtn,LV_ALIGN_OUT_RIGHT_MID,0,0);
	lv_btn_set_action(mainEqBtn, LV_BTN_ACTION_CLICK, btn_action);
	label = lv_label_create(mainEqBtn, NULL);
	lv_label_set_text(label, SYMBOL_EDIT);

	mainFileBtn=lv_btn_create(mainScreen,NULL);
	lv_cont_set_fit(mainFileBtn, false, false);
	lv_obj_set_size(mainFileBtn,LV_HOR_RES/2, LV_VER_RES/2);
	lv_obj_align(mainFileBtn,mainAudioBtn,LV_ALIGN_OUT_BOTTOM_MID,0,0);
	lv_btn_set_action(mainFileBtn, LV_BTN_ACTION_CLICK, btn_action);
	label = lv_label_create(mainFileBtn, NULL);
	lv_label_set_text(label, SYMBOL_DIRECTORY);

	mainSetBtn=lv_btn_create(mainScreen,NULL);
	lv_cont_set_fit(mainSetBtn, false, false);
	lv_obj_set_size(mainSetBtn,LV_HOR_RES/2, LV_VER_RES/2);
	lv_obj_align(mainSetBtn,mainFileBtn,LV_ALIGN_OUT_RIGHT_MID,0,0);
	lv_btn_set_action(mainSetBtn, LV_BTN_ACTION_CLICK, btn_action);
	label = lv_label_create(mainSetBtn, NULL);
	lv_label_set_text(label, SYMBOL_SETTINGS);

	mainBtns[0]=mainAudioBtn;
	mainBtns[1]=mainEqBtn;
	mainBtns[2]=mainFileBtn;
	mainBtns[3]=mainSetBtn;
	/*Create group of MainScren*/
	groups[MAIN_SCREEN] = lv_group_create();
	/*Create a second button matrix with the new styles*/
	for(int i=0;i<4;i++)
	{
		//lv_btn_set_style(mainBtns[i], LV_BTN_STYLE_BG, &style_bg);
		lv_btn_set_style(mainBtns[i], LV_BTN_STYLE_REL, &style_btn_rel);
		lv_btn_set_style(mainBtns[i], LV_BTN_STYLE_PR, &style_btn_pr);
		lv_group_add_obj(groups[MAIN_SCREEN], mainBtns[i]);
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
	
	lv_roller_set_options(bassRoller, "-2\n"
		"-1\n"
		"0\n"
		"1\n"
		"2\n");
	lv_roller_set_hor_fit(bassRoller, false);
	lv_obj_set_pos(bassRoller, x, y);
	lv_obj_set_width(bassRoller, w);
	//lv_obj_set_height(bassRoller, h);
	lv_roller_set_visible_row_count(bassRoller, 3);
	lv_roller_set_selected(bassRoller, 2, false);
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

static void EqualizerScreenCreate(void)
{
	equalizerScreen = lv_obj_create(NULL, NULL);
	lv_obj_set_style(equalizerScreen, &style_bg);
	static char *names[] = { "Bass","Mid","Trebble" };
	bassRoller    = createRoller(names[0], 0* LV_HOR_RES / 3, LV_VER_RES / 3, LV_HOR_RES / 3, LV_VER_RES * 2 / 3);
	midRoller     = createRoller(names[1], 1*LV_HOR_RES / 3, LV_VER_RES / 3, LV_HOR_RES / 3, LV_VER_RES * 2 / 3);
	trebbleRoller = createRoller(names[2], 2* LV_HOR_RES / 3, LV_VER_RES / 3, LV_HOR_RES / 3, LV_VER_RES * 2 / 3);
	eqBackBtn = BackButtonCreate(equalizerScreen, retMainScreen);
}

static void FilesScreenCreate(void)
{
	filesScreen = lv_obj_create(NULL, NULL);
	lv_obj_set_style(filesScreen, &style_bg);

	unsigned pos;
	currarr current = UI.getCurrent(&fileListSz, &pos);
	fileList[0] = lv_list_create(filesScreen, NULL);
	fileList[1] = lv_list_create(filesScreen, NULL);
	lv_obj_set_height(fileList[0], LV_VER_RES);
	lv_obj_set_height(fileList[1], LV_VER_RES);
	lv_obj_set_width(fileList[0], LV_HOR_RES);
	lv_obj_set_width(fileList[1], LV_HOR_RES);
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
	playScreen = lv_obj_create(NULL, NULL);

	eqGraph = lv_chart_create(playScreen, NULL);

	ser1 = lv_chart_add_series(eqGraph, LV_COLOR_RED);
	lv_chart_set_series_width(eqGraph, 70);
	lv_chart_set_point_count(eqGraph, 8);
	lv_chart_set_type(eqGraph, LV_CHART_TYPE_COLUMN);
	lv_obj_set_size(eqGraph, LV_HOR_RES*3/4, LV_VER_RES*2/5);
	lv_obj_align(eqGraph, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
	lv_chart_set_style(eqGraph, &style_bgg);
	lv_chart_set_div_line_count(eqGraph, 0, 0);
	lv_chart_set_series_darking(eqGraph, LV_OPA_100);

	/*Create label on the screen. By default it will inherit the style of the screen*/
	songNameLbl = lv_label_create(playScreen, NULL);
	lv_obj_set_width(songNameLbl, LV_HOR_RES *3/ 4);
	lv_label_set_text(songNameLbl, "Song Name");
	lv_obj_align(songNameLbl, NULL, LV_ALIGN_IN_TOP_MID, 0, lv_obj_get_height(songNameLbl));

	lv_label_set_long_mode(songNameLbl, LV_LABEL_LONG_ROLL);

	artistNameLbl = lv_label_create(playScreen, NULL);
	lv_obj_set_width(artistNameLbl, LV_HOR_RES*3 / 4);
	lv_label_set_text(artistNameLbl, "Artist");
	lv_obj_align(artistNameLbl, songNameLbl, LV_ALIGN_OUT_BOTTOM_MID, 0, +lv_obj_get_height(artistNameLbl));

	lv_label_set_long_mode(artistNameLbl, LV_LABEL_LONG_ROLL);

	progressBar = lv_bar_create(playScreen, NULL);
	lv_bar_set_range(progressBar, 0, 0);
	lv_obj_set_width(progressBar, LV_HOR_RES *3/ 4);
	lv_obj_align(progressBar, artistNameLbl, LV_ALIGN_OUT_BOTTOM_MID, 0, +lv_obj_get_height(progressBar)/2);

	volumeLabel = lv_label_create(playScreen,NULL);
	lv_label_set_text(volumeLabel,SYMBOL_VOLUME_MAX" 30");
	lv_obj_align(volumeLabel, NULL, LV_ALIGN_IN_TOP_RIGHT, -20, 0);


	playBackBtn=BackButtonCreate(playScreen, retMainScreen);

}

void MP3UiSetSongInfo(const char* title, const char*artist, int dur,int first,int volume,float* eqPoints)
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
		ser1->points[0] = eqPoints[0];
		ser1->points[1] = eqPoints[1];
		ser1->points[2] = eqPoints[2];
		ser1->points[3] = eqPoints[3];
		ser1->points[4] = eqPoints[4];
		ser1->points[5] = eqPoints[5];
		ser1->points[6] = eqPoints[6];
		ser1->points[7] = eqPoints[7];

		lv_chart_refresh(eqGraph);
	}
	if(volume == 0)
	{
		lv_label_set_text(volumeLabel,SYMBOL_MUTE" 0");
	}
	else if(volume<20)
	{
		char txt[8];
		sprintf(txt,SYMBOL_VOLUME_MID" %d",volume);
		lv_label_set_text(volumeLabel,txt);
	}
	else
	{
		char txt[8];
		sprintf(txt,SYMBOL_VOLUME_MAX" %d",volume);
		lv_label_set_text(volumeLabel,txt);
	}
}

void StylesInit()
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

void MP3UiCreate(lv_indev_drv_t* kb_dr)
{
	kb_drv = kb_dr;
	kb_indev = lv_indev_drv_register(kb_drv);
	UIinit=UI.init();
	StylesInit();
	MainScreenCreate();
	EqualizerScreenCreate();
	FilesScreenCreate();
	PlayScreenCreate();
	lv_scr_load(mainScreen);
	setActiveGroup(MAIN_SCREEN,4,mainBtns);
}

char* getMP3file()
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
