#include "MP3Ui.h"
#include "UI.h"
#include <stdio.h>
/*Screens*/
static lv_obj_t * mainScreen, *equalizerScreen,*filesScreen,*playScreen;

/*File Screen info*/
static lv_obj_t * currentFileList[MAX_FILES],*fileList[2];
static unsigned fileListSz = 0,fileListPointer=0,newFile=0;
static char* newFileName;

/*Main Screen Info*/
lv_obj_t * btnm1;

/*PlayScreen Info*/
static unsigned duration,currentTime;
static lv_obj_t* progressBar,*playBackBtn,*songNameLbl,*artistNameLbl;

/*Button styles*/
static lv_style_t style_bg,style_bgg;
static lv_style_t style_btn_rel;
static lv_style_t style_btn_pr;

/*Indev devices*/
static lv_indev_drv_t* kb_drv;
static lv_indev_t* kb_indev;

/*Groups*/
enum{MAIN_SCREEN=0,PLAY_SCREEN,EQ_SCREEN,FILE_SCREEN0,FILE_SCREEN1,SETTING_SCREEN,SCREENS}screenEnum;
static lv_group_t * groups[SCREENS];



static void setActiveGroup(int p,lv_obj_t* ob)
{
	if (groups[p] != NULL)
		lv_group_del(groups[p]);
	groups[p] = lv_group_create();
	lv_group_add_obj(groups[p], ob);
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
			
}


static int checkMP3file(char* fn, size_t sz)
{
	if (fn[sz - 1] == '3' && fn[sz - 2] == 'p' && fn[sz - 3] == 'm'&& fn[sz - 4] == '.')
		return 1;
	return 0;
}

static lv_res_t retMainScreen(lv_obj_t* obj)
{
	lv_scr_load(mainScreen);
	
	setActiveGroup(MAIN_SCREEN,btnm1);
	return LV_RES_OK;
}

static lv_res_t btnm_action(lv_obj_t * btnm, const char *txt)
{
	if (strcmp(SYMBOL_EDIT, txt) == 0)
	{
		lv_scr_load(equalizerScreen);
		setActiveGroup(EQ_SCREEN,equalizerScreen);
	}
	else if (strcmp(SYMBOL_DIRECTORY, txt) == 0)
	{
		lv_scr_load(filesScreen);
		setActiveGroup(FILE_SCREEN0 + fileListPointer, fileList[fileListPointer]);
	}
	else if(strcmp(SYMBOL_AUDIO,txt)==0)
	{
		setActiveGroup(PLAY_SCREEN,playBackBtn);
		lv_scr_load(playScreen);
	}
	return LV_RES_OK; /*Return OK because the button matrix is not deleted*/
}

static lv_res_t fileScreenUpdate(lv_obj_t* obj)
{
	unsigned pos = 0;
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
		return LV_RES_OK;
		
	char* currFile = UI.getFile();
	printf("Current path %s\n", UI.getPath());
	if (currFile != 0)
	{
		printf("Selected file %s\n", currFile);
		newFile=1;
		newFileName=currFile;
		setActiveGroup(PLAY_SCREEN,playBackBtn);
		lv_scr_load(playScreen);
	}
	else
	{

		//hide the current list and change to the other
		lv_obj_set_hidden(fileList[fileListPointer], true);
		fileListPointer = (fileListPointer + 1) % 2;
		lv_obj_set_hidden(fileList[fileListPointer], false);
		
		//delete the list and group to re-create them 
		lv_obj_del(fileList[fileListPointer]);
		
		//re-creating...
		fileList[fileListPointer] = lv_list_create(filesScreen, NULL);
		lv_obj_set_height(fileList[fileListPointer], LV_VER_RES);
		setActiveGroup(FILE_SCREEN0 + fileListPointer, fileList[fileListPointer]);

		lv_obj_align(fileList[fileListPointer], NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
		currarr current = UI.getCurrent(&fileListSz, &pos);
		for (unsigned i = 0; i < fileListSz; i++)
		{
			if (current[i][MAX_FILE_NAME] == 1)
			{
				currentFileList[i] = lv_list_add(fileList[fileListPointer], SYMBOL_DIRECTORY, current[i], fileScreenUpdate);
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
	}
	return LV_RES_OK;
}

static void MainScreenCreate(void)
{
	mainScreen = lv_obj_create(NULL, NULL);
	static const char * btnm_map[] = { SYMBOL_AUDIO, SYMBOL_EDIT,"\n", SYMBOL_DIRECTORY,SYMBOL_SETTINGS, "" };
	/*Create a default button matrix*/
	btnm1 = lv_btnm_create(mainScreen, NULL);
	lv_btnm_set_map(btnm1, btnm_map);
	lv_btnm_set_action(btnm1, btnm_action);
	lv_obj_set_size(btnm1, LV_HOR_RES, LV_VER_RES);

	/*Create a second button matrix with the new styles*/
	lv_btnm_set_style(btnm1, LV_BTNM_STYLE_BG, &style_bg);
	lv_btnm_set_style(btnm1, LV_BTNM_STYLE_BTN_REL, &style_btn_rel);
	lv_btnm_set_style(btnm1, LV_BTNM_STYLE_BTN_PR, &style_btn_pr);

	/*Create group of MainScren*/
	groups[MAIN_SCREEN] = lv_group_create();
	lv_group_add_obj(groups[MAIN_SCREEN], btnm1);
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
	createRoller(names[0], 0* LV_HOR_RES / 3, LV_VER_RES / 3, LV_HOR_RES / 3, LV_VER_RES * 2 / 3);
	createRoller(names[1], 1*LV_HOR_RES / 3, LV_VER_RES / 3, LV_HOR_RES / 3, LV_VER_RES * 2 / 3);
	createRoller(names[2], 2* LV_HOR_RES / 3, LV_VER_RES / 3, LV_HOR_RES / 3, LV_VER_RES * 2 / 3);
	BackButtonCreate(equalizerScreen, retMainScreen);
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
	BackButtonCreate(filesScreen, retMainScreen);
	groups[FILE_SCREEN0] = lv_group_create();
	lv_group_add_obj(groups[FILE_SCREEN0], fileList[0]);
	//lv_indev_set_group(kb_indev_list, groups[FILE_SCREEN0]);
	groups[FILE_SCREEN1] = lv_group_create();
	lv_group_add_obj(groups[FILE_SCREEN1], fileList[1]);
}
static void PlayScreenCreate(void)
{
	playScreen = lv_obj_create(NULL, NULL);
	/*Create label on the screen. By default it will inherit the style of the screen*/
	songNameLbl = lv_label_create(playScreen, NULL);
	lv_obj_set_width(songNameLbl, LV_HOR_RES / 2);
	lv_label_set_text(songNameLbl, "Song Name");
	lv_obj_align(songNameLbl, NULL, LV_ALIGN_CENTER, 0, -2*lv_obj_get_height(songNameLbl));

	lv_label_set_long_mode(songNameLbl, LV_LABEL_LONG_ROLL);

	artistNameLbl = lv_label_create(playScreen, NULL);
	lv_obj_set_width(artistNameLbl, LV_HOR_RES / 2);
	lv_label_set_text(artistNameLbl, "Artist");
	lv_obj_align(artistNameLbl, songNameLbl, LV_ALIGN_OUT_BOTTOM_MID, 0, +lv_obj_get_height(artistNameLbl));

	lv_label_set_long_mode(artistNameLbl, LV_LABEL_LONG_ROLL);

	progressBar = lv_bar_create(playScreen, NULL);
	lv_bar_set_range(progressBar, 0, 0);
	lv_obj_set_width(progressBar, LV_HOR_RES *3/ 4);
	lv_obj_align(progressBar, artistNameLbl, LV_ALIGN_OUT_BOTTOM_MID, 0, +lv_obj_get_height(progressBar));

	playBackBtn=BackButtonCreate(playScreen, retMainScreen);

}

void MP3UiSetSongInfo(const char* title, const char*artist, int dur)
{
	if (title != NULL)
	{
		lv_label_set_text(songNameLbl, title);
		lv_obj_set_width(songNameLbl, LV_HOR_RES / 2);
		lv_obj_align(songNameLbl, NULL, LV_ALIGN_CENTER, 0, -2*lv_obj_get_height(songNameLbl));

		lv_label_set_text(artistNameLbl, artist);
		lv_obj_set_width(artistNameLbl, LV_HOR_RES / 2);
		lv_obj_align(artistNameLbl, songNameLbl, LV_ALIGN_OUT_BOTTOM_MID, 0, +lv_obj_get_height(artistNameLbl));

		lv_bar_set_range(progressBar, 0, dur);
		lv_bar_set_value(progressBar, 0);
		lv_obj_align(progressBar, artistNameLbl, LV_ALIGN_OUT_BOTTOM_MID, 0, +lv_obj_get_height(progressBar));
		currentTime = 0;
		duration = dur;
	}
	else
	{
		currentTime = duration - dur;
		lv_bar_set_value(progressBar, currentTime);
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
	UI.init();
	StylesInit();
	MainScreenCreate();
	EqualizerScreenCreate();
	FilesScreenCreate();
	PlayScreenCreate();
	lv_scr_load(mainScreen);
	setActiveGroup(MAIN_SCREEN,btnm1);
}

char* getMP3file()
{
	if(newFile==1)
	{
		newFile=0;
		return newFileName;
	}
	return 0;

}
