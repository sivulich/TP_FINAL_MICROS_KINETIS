#include "UI.h"
#include "ff.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "fsl_debug_console.h"
#ifdef __cplusplus
extern "C" {
#endif

	static char current[MAX_FILES][MAX_FILE_NAME+1];
	static char file[MAX_FILE_NAME]=".";

	FILINFO de;  // Pointer for directory entry

	static int i = 0, cnt = 0, newFile = 0, onFile=0, ini = 1,len=1;

	static DIR tmp, dr;
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
		while (f_readdir(&dr,&de) == FR_OK && de.fname[0]!=0 && cnt < 255)
		{
			if((de.fattrib & AM_HID) == 0 && (de.fattrib & AM_SYS) == 0)
			{

				strncpy(current[cnt++], de.fname, MAX_FILE_NAME);
				current[cnt-1][MAX_FILE_NAME]=(de.fattrib&AM_DIR)&&1;
				//PRINTF("Found a dir: %d with name: %s",de.fattrib&AM_DIR,current[cnt-1]);
			}

		}

		strncpy(current[cnt++], "..",MAX_FILE_NAME);

	}

	unsigned init() {
		if (f_opendir(&dr,".") != FR_OK)  // opendir returns NULL if couldn't open directory
			ini = 0;
		else
		{
			updateCurrent();
		}

		return ini;

	};


	char input(char cmd) {
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

			if(cnt-1==i)
			{

				delLastDir();
				f_opendir(&dr,file);

				updateCurrent();
			}
			else
			{
				FILINFO info;
				strcat(file, "/");
				strcat(file, current[i]);
				len += strlen(current[i]) + 1;
				FRESULT res = f_stat(file,&info);
				PRINTF("Current is: %s\n",current[i]);
				if (res == FR_OK && (info.fattrib&AM_DIR) )
				{
					f_opendir(&tmp,current[i]);
					cnt = 0;
					i = 0;
					dr = tmp;
					updateCurrent();

				}
				else if(res == FR_OK )
				{
					newFile = 1;
					onFile = 1;
				}
				else
				{
					delLastDir();
				}
			}


			return 0;
		}
		return cmd;
	};
	char* getPath() {
		return file;
	}
	char* getFile() {
		if (newFile == 1)
		{
			
			newFile = 0;
			return getPath();
		}
		return 0;
	};

	currarr  getCurrent(unsigned* sz, unsigned *pos)
	{
		*sz = cnt;
		*pos = i;
		return (currarr)current;
	}
	void close() {
		f_closedir(&dr);

	};
	void exitFile()
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
	void setPos(int p)
	{
		if(i>=0 && i<cnt)
			i = p;
	}
	UI_ UI = { .init = init,.input = input,.getFile = getFile,.getPath=getPath,.getCurrent = getCurrent,.close = close,.exitFile=exitFile , .setPos=setPos};
#ifdef __cplusplus
}
#endif
