#include "UI.h"
#include "ff.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
//#include "fsl_debug_console.h"
#include "ff.h"
#include "diskio.h"
#include "fsl_sd_disk.h"
#include "fsl_sysmpu.h"

static unsigned init(void);
static void updateCurrent();
static FATFS g_fileSystem; /* File system object */
static FRESULT error;

const TCHAR driverNumberBuffer[3U] = {SDDISK + '0', ':', '/'};

static char current[MAX_FILES][MAX_FILE_NAME+1];
static char file[MAX_FILE_NAME]=".";
static char selectedPath[MAX_FILE_NAME]="";

FILINFO de;  // Pointer for directory entry

static int i = 0, cnt = 0, newFile = 0, onFile=0, ini = 1,len=1,currentIndex=0,selectedIndex=0,selectedQnt=0;

static DIR tmp, dr;



static const sdmmchost_detect_card_t s_sdCardDetect = {
#ifndef BOARD_SD_DETECT_TYPE
	.cdType = kSDMMCHOST_DetectCardByGpioCD,
#else
	.cdType = BOARD_SD_DETECT_TYPE,
#endif
	.cdTimeOut_ms = 0,//(~0U),
};

static int pwrOn=0;
static status_t sdcardWaitCardInsert(void)
{
	/* Save host information. */
	g_sd.host.base = SD_HOST_BASEADDR;
	g_sd.host.sourceClock_Hz = SD_HOST_CLK_FREQ;
	/* card detect type */
	g_sd.usrParam.cd = &s_sdCardDetect;
	#if defined DEMO_SDCARD_POWER_CTRL_FUNCTION_EXIST
		g_sd.usrParam.pwr = &s_sdCardPwrCtrl;
	#endif
	/* SD host init function */
	if (SD_HostInit(&g_sd) != kStatus_Success)
	{
		//PRINTF("\r\nSD host init fail\r\n");
		return kStatus_Fail;
	}
	/* power off card */
	SD_PowerOffCard(g_sd.host.base, g_sd.usrParam.pwr);
	/* wait card insert */
	if (SD_WaitCardDetectStatus(SD_HOST_BASEADDR, &s_sdCardDetect, true) == kStatus_Success)
	{
		//PRINTF("\r\nCard inserted.\r\n");
		/* power on the card */
		if(pwrOn==0)
		{
			pwrOn=1;
			SD_PowerOnCard(g_sd.host.base, g_sd.usrParam.pwr);
		}

	}
	else
	{
		//PRINTF("\r\nCard detect fail.\r\n");
		pwrOn=0;
		SD_PowerOffCard(g_sd.host.base,g_sd.usrParam.pwr);
		return kStatus_Fail;
	}

	return kStatus_Success;
}

static int checkSdCard()
{
	if (sdcardWaitCardInsert() != kStatus_Success)
	{
		ini=2;
		updateCurrent();
	}
	return ini;
}

static void delLastDir()
{
	for (int k = len - 1; k > 0; k--)
	{
		if (file[k] == '/')
		{
			len = k;
			file[k] = 0;
			break;
		}
		file[k] = 0;

	}
}
static void updateCurrent()
{
	cnt=0;
	i=0;
	if(ini==1)
	{
		strncpy(current[cnt++], "..",MAX_FILE_NAME);
		if(len>1)
			current[cnt-1][MAX_FILE_NAME]=2;
		else
			current[cnt-1][MAX_FILE_NAME]=3;
		while (f_readdir(&dr,&de) == FR_OK && de.fname[0]!=0 && cnt < MAX_FILES-2)
		{
			if((de.fattrib & AM_HID) == 0 && (de.fattrib & AM_SYS) == 0)
			{

				strncpy(current[cnt++], de.fname, MAX_FILE_NAME);
				current[cnt-1][MAX_FILE_NAME]=(de.fattrib&AM_DIR)&&1;
				//PRINTF("Found a dir: %d with name: %s",de.fattrib&AM_DIR,current[cnt-1]);
			}

		}
		if(cnt==MAX_FILES-2)
		{
			strncpy(current[cnt++], "More",MAX_FILE_NAME);
			current[cnt-1][MAX_FILE_NAME]=5;
		}


	}
	else
	{
		if(ini==2)
		{
			cnt=1;
			strncpy(current[0], "Please Insert SD card",MAX_FILE_NAME);
			current[0][MAX_FILE_NAME]=4;
			file[0]='.';
			file[1]=0;
			len=1;
		}
	}


}

static unsigned init() {
	SYSMPU_Enable(SYSMPU, false);


	if (sdcardWaitCardInsert() != kStatus_Success)
	{
		ini=2;
		updateCurrent();
	}
	else
	{
		if (f_mount(&g_fileSystem, driverNumberBuffer, 0U))
		{
			//PRINTF("Mount volume failed.\r\n");
			ini=0;
			return 0;
		}

		#if (FF_FS_RPATH >= 2U)
			error = f_chdrive((char const *)&driverNumberBuffer[0U]);
			if (error)
			{
				//PRINTF("Change drive failed.\r\n");
				ini=0;
				return 0;
			}
		#endif

		if (f_opendir(&dr,".") != FR_OK)  // opendir returns NULL if couldn't open directory
			ini = 0;
		else
		{
			ini=1;
			updateCurrent();
		}
	}

	return ini;

};


static char input(char cmd) {
	checkSdCard();
	if(ini == 1)
	{
		if (cmd == 's')
		{
			i = i + 1 >= cnt ? 0 : i + 1;
			return 0;
		}
		else if (cmd == 'w')
		{
			i = i - 1 < 0 ? cnt-1 : i - 1;
			return 0;
		}

		else if (cmd == 'e')
		{

			if(0==i)
			{

				if(len==1)
					return 1;
				delLastDir();
				f_opendir(&dr,file);

				updateCurrent();
				currentIndex=0;
			}
			else if(current[i][MAX_FILE_NAME]==5)
			{
				updateCurrent();
				currentIndex+=MAX_FILES-2;
			}
			else
			{
				FILINFO info;
				strcat(file, "/");
				strcat(file, current[i]);
				len += strlen(current[i]) + 1;
				FRESULT res = f_stat(file,&info);

				if (res == FR_OK && (info.fattrib&AM_DIR) )
				{
					f_opendir(&tmp,file);
					cnt = 0;
					i = 0;
					dr = tmp;
					updateCurrent();
					currentIndex=0;

				}
				else if(res == FR_OK )
				{
					newFile = 1;
					onFile = 1;
					selectedIndex=i+currentIndex-1;
					for (int k = len - 1; k > 0; k--)
					{
						if (file[k] == '/')
						{
							memcpy(selectedPath,file,k);
							break;
						}
					}
					selectedQnt=0;
					DIR temp;
					FILINFO itemp;
					f_opendir(&temp,selectedPath);
					while(f_readdir(&temp,&itemp)==FR_OK && itemp.fname[0]!=0)
						selectedQnt++;
					//PRINTF("Selected file is: %s\n its index is: %d\n and its path is: %s\n\n",current[i],selectedIndex,selectedPath);
				}
				else
				{
					delLastDir();
				}
			}


			return 0;
		}
	}
	else if(ini==2)
		return 1;

	return cmd;
}
static char* getPath() {
	return file;
}
static char* getFile() {
	checkSdCard();

	if(ini == 1)
	{
		if (newFile == 1)
		{

			newFile = 0;
			return getPath();
		}
		return 0;
	}
	else if(ini == 2)
	{
		return (char*)-1;
	}
	else
		return 0;
};

static currarr  getCurrent(unsigned* sz, unsigned *pos)
{
	checkSdCard();
	*sz = cnt;
	*pos = i;
	return (currarr)current;
}

static void close()
{
	f_closedir(&dr);

}

static void exitFile()
{
	if (onFile == 1)
	{
		onFile = 0;
		newFile = 0;
		for (int k = len - 1; k > 0; k--)
		{
			if (file[k] == '/')
			{
				len = k;
				file[k] = 0;
				break;
			}
			file[k] = 0;
		}
	}
}

static void setPos(int p)
{
	if(p>=0 && p<cnt)
		i = p;
}
static void getAdjFile(int offset,char* dest)
{
	if(selectedPath[0]!=0)
	{
		DIR folder;
		FILINFO info;
		if(selectedIndex==selectedQnt)
		{
			selectedIndex--;
		}
		else if(selectedIndex+offset>=selectedQnt)
		{
			dest[0]=0;
			selectedIndex=selectedQnt;
			return;
		}
		f_opendir(&folder,selectedPath);

		int pos=(selectedIndex+offset+2*selectedQnt)%selectedQnt;
		selectedIndex=pos;
		for(int j=0;j<pos;j++)
		{
			f_readdir(&folder,&info);

		}
		f_readdir(&folder,&info);
		strcpy(dest,selectedPath);
		strcat(dest,"/");
		strcat(dest,info.fname);
		f_closedir(&folder);
	}
	else
		dest[0]=-1;
}

UI_ UI = { .init = init, .getAdjFile=getAdjFile,.input = input,.getFile = getFile,.getPath=getPath,.getCurrent = getCurrent,.close = close,.exitFile=exitFile , .setPos=setPos};
