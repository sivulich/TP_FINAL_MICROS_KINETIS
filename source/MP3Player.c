/*
 * MP3Player.c
 *
 *  Created on: Feb 16, 2019
 *      Author: santi
 */

#include "MP3Player.h"
#include "MP3Decoder.h"
#include "SigGen.h"
#include "MP3UI.h"
#include "LEDDisplay.h"
#include "MP3PlayerData.h"
#include <string.h>
#include "fsl_debug_console.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define PING_PONG_BUFFS 16
/* buffer size (in byte) for read/write operations */
#define BUFFER_SIZE (100U)

#define FFT_LENGTH 1024

/*******************************************************************************
 * Variables
 ******************************************************************************/
static unsigned buffLen;
static short buff[MP3_BUFFER_SIZE*PING_PONG_BUFFS/2];
static short* outBuff[PING_PONG_BUFFS];
static int len=0,dur=0,lastStatus;
static long long pos=0;

static int volumeMap[] = {	0,
							137,
							273,
							410,
							546,
							683,
							819,
							956,
							1092,
							1229,
							1365,
							1502,
							1638,
							1775,
							1911,
							2048,
							2185,
							2321,
							2458,
							2594,
							2731,
							2867,
							3004,
							3140,
							3277,
							3413,
							3550,
							3686,
							3823,
							3959,
							4096};

/*******************************************************************************
 * Functions
 ******************************************************************************/
static int init();
static void update();

static int init()
{
	/*Inicializamos los ping pong buffers*/
	for(int i=0;i<PING_PONG_BUFFS;i++)
		outBuff[i]=buff+i*MP3_BUFFER_SIZE/2;

	/*Inicializamos el decodificador MP3 y el generador de señales*/
	MP3DEC.init();
	SigGen.init();
	LEDDisplay.init();
	return 0;
}
static void startSong(char* file,int volume)
{
	//Reiniciamos el decodificador MP3
	MP3DEC.unloadFile();
	MP3DEC.loadFile(file);

	//Obtenemos el header extraemos la información y lo salteamos
	dur = 0;
	while(dur<=0)
		dur=MP3DEC.decode(outBuff[0],&buffLen);
	char* songTitle = (char*)MP3DEC.getMP3Info("TIT2",&len), *songArtist = (char*)MP3DEC.getMP3Info("TPE1",&len);
	if(!strcmp(songTitle,""))
	{
		songTitle = strrchr(file, (int)'/');
		songTitle++;
		int k = 0;
		while(songTitle[k++]!=0)
		{
			if(songTitle[k]=='.')
				if(songTitle[k+1]=='m' || songTitle[k+1]=='M')
					if(songTitle[k+2]=='p' || songTitle[k+2]=='P')
						if(songTitle[k+3]=='3')
							songTitle[k]='\0';
		}

	}
	MP3UiSetSongInfo(songTitle,songArtist,dur/1000,1,MP3PlayerData.volume,NULL);
	while(len<=0)
		len=MP3DEC.decode(outBuff[0],&buffLen);
	MP3FrameInfo finfo=MP3DEC.getFrameInfo();

	//Reinicamos el generador de señales y lo configuramos para esta canción
	SigGen.stop();
	SigGen.setupSignal(outBuff,PING_PONG_BUFFS,finfo.outputSamps,finfo.samprate*finfo.nChans);
	len=0;
	pos=0;
	lastStatus=1;

	//Preparamos los ping pong buffers
	for(int i=0;i<PING_PONG_BUFFS;i++)
	{
		len+=MP3DEC.decode(outBuff[i],&buffLen);
		for(int j=0;j<buffLen;j++)
		{
			unsigned temp=(((((int)outBuff[i][j] + 32768))>>4))/2;
			temp*=volumeMap[MP3PlayerData.volume];
			temp>>=12;
			outBuff[i][j]=temp;
		}
	}

	//Arrancamos la señal
	//SigGen.start();
}


static void update()
{
	//Si se selecciono un archivo nos devolvera el nombre, si no hay SD -1 y si no se selecciono nada 0
	char* file = getMP3file();
	if(file!=(char*)-1)
	{
		//Si se selecciono un archivo
		if(file!=NULL)
		{
			startSong(file,MP3PlayerData.volume);
			MP3PlayerData.play=1;
		}
		if(MP3PlayerData.play==1 && MP3DEC.onFile()==1)
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
					float input[1152];
					int ret = MP3DEC.decode(outBuff[circ%PING_PONG_BUFFS],&buffLen);
					len+=ret;
					for(int i=0;i<buffLen;i++)
					{
						if(len>65 && i%2==0)
						{
							input[i/2]=(outBuff[circ%PING_PONG_BUFFS][i]/2+outBuff[circ%PING_PONG_BUFFS][i+1]/2)*1.0/32768;
						}

						unsigned temp=(((((int)outBuff[circ%PING_PONG_BUFFS][i] + 32768))>>4))/2;
						temp*=volumeMap[MP3PlayerData.volume];
						temp>>=12;
						outBuff[circ%PING_PONG_BUFFS][i]=temp;
					}

					if(ret>0 && len>65)
					{
						pos+=len;
						LEDDisplay.setVumeter(input,FFT_LENGTH,1);
						MP3UiSetSongInfo(NULL,NULL,pos/1000,0,MP3PlayerData.volume,LEDDisplay.getEqualizer());
						len=0;
					}
					else if(ret<0)
						PRINTF("ERROR DECODING %d\n",ret);
					else if(ret  == 0)
					{
						float eqPoints[8]={0,0,0,0,0,0,0,0};
						MP3UiSetSongInfo(NULL,NULL,dur/1000,0,MP3PlayerData.volume,eqPoints);
						MP3DEC.unloadFile();
						SigGen.stop();
						if(MP3PlayerData.playMode==1 || MP3PlayerData.playMode==0)
						{
							char nextFile[256];
							getMP3AdjFile(1,nextFile);
							if(nextFile[0]==0)
							{
								if(MP3PlayerData.playMode==1)
								{
									getMP3AdjFile(1,nextFile);
									if(nextFile[0]!=(char)-1)
									{
										startSong(nextFile,MP3PlayerData.volume);
									}
								}
								else
								{
									getMP3AdjFile(1,nextFile);
									if(nextFile[0]!=(char)-1)
									{
										startSong(nextFile,MP3PlayerData.volume);
										SigGen.pause();
										MP3PlayerData.play=0;
									}
								}
							}
							else if(nextFile[0]!=(char)-1)
							{
								startSong(nextFile,MP3PlayerData.volume);
							}

						}
						else if(MP3PlayerData.playMode == 2)
						{
							char nextFile[256];
							if(nextFile[0]!=(char)-1)
							{
								getMP3AdjFile(1,nextFile);
							}
						}
						//printf("Next file is %s\n",nextFile);
						break;
					}

				}
				lastStatus=status;
			}

		}
		else if(MP3PlayerData.play==0)
		{
			SigGen.pause();
		}
		if(MP3PlayerData.offset != 0)
		{
			float eqPoints[8]={0,0,0,0,0,0,0,0};
			MP3UiSetSongInfo(NULL,NULL,dur/1000,0,MP3PlayerData.volume,eqPoints);
			MP3DEC.unloadFile();
			SigGen.stop();
			char nextFile[256];
			getMP3AdjFile(MP3PlayerData.offset,nextFile);
			if(nextFile[0]==0)
				getMP3AdjFile(MP3PlayerData.offset,nextFile);
			if(nextFile[0]!=(char)-1)
			{
				startSong(nextFile,MP3PlayerData.volume);
			}
			MP3PlayerData.offset = 0;
		}
	}
	else
	{
		SigGen.stop();
	}
}

MP3Player_ MP3Player={.init=init, .update=update};
