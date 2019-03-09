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

#include "fsl_debug_console.h"	//para el PRINTF

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

/*Volume MAP*/
#define MAX_VOLUME 30
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
static int init(int* p,int* v);
static void update();

static int * pl,*vol;
static int init(int* p,int* v)
{
	pl=p;
	vol=v;
	/*Inicializamos los ping pong buffers*/
	for(int i=0;i<PING_PONG_BUFFS;i++)
		outBuff[i]=buff+i*MP3_BUFFER_SIZE/2;

	/*Inicializamos el decodificador MP3 y el generador de señales*/
	MP3DEC.init();
	SigGen.init();
	LEDDisplay.init();
	return 0;
}


static void update()
{
	int play=*pl;
	int volume=*vol;
	//Si se selecciono un archivo nos devolvera el nombre, si no hay SD -1 y si no se selecciono nada 0
	char* file = getMP3file();
	if(file!=(char*)-1)
	{
		//Si se selecciono un archivo
		if(file!=NULL)
		{
			//Reiniciamos el decodificador MP3
			MP3DEC.unloadFile();
			MP3DEC.loadFile(file);

			//Obtenemos el header extraemos la información y lo salteamos
			dur = 0;
			while(dur<=0)
				dur=MP3DEC.decode(outBuff[0],&buffLen);
			char* songTitle = (char*)MP3DEC.getMP3Info("TIT2",&len), *songArtist = (char*)MP3DEC.getMP3Info("TPE1",&len);
			if(!strcmp(songTitle,"")) songTitle = file;
			//if(!strcmp(songArtist,"")) songArtist = "Song Artist";
			MP3UiSetSongInfo(songTitle,songArtist,dur/1000,1,volume,NULL);
			while(len<=0)
				len=MP3DEC.decode(outBuff[0],&buffLen);
			MP3FrameInfo finfo=MP3DEC.getFrameInfo();

			//Reinicamos el generador de señales y lo configuramos para esta canción
			SigGen.stop();
			SigGen.setupSignal(outBuff,PING_PONG_BUFFS,finfo.outputSamps,finfo.samprate*finfo.nChans);
			len=0;
			pos=0;
			*pl=1;
			play=1;
			lastStatus=1;

			//Preparamos los ping pong buffers
			for(int i=0;i<PING_PONG_BUFFS;i++)
			{
				len+=MP3DEC.decode(outBuff[i],&buffLen);
				for(int j=0;j<buffLen;j++)
				{
					unsigned temp=(((((int)outBuff[i][j] + 32768))>>4))/2;
					temp*=volumeMap[volume];
					temp>>=12;
					outBuff[i][j]=temp;
				}
			}


			//Arrancamos la señal
			SigGen.start();
		}
		if(play==1 && MP3DEC.onFile()==1)
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
						temp*=volumeMap[volume];
						temp>>=12;
						outBuff[circ%PING_PONG_BUFFS][i]=temp;
					}



					if(ret>0 && len>65)
					{
						pos+=len;
						LEDDisplay.setVumeter(input,FFT_LENGTH,1);
						MP3UiSetSongInfo(NULL,NULL,pos/1000,0,volume,LEDDisplay.getEqualizer());

						len=0;
					}
					else if(ret<0)
						PRINTF("ERROR DECODING %d\n",ret);
					else if(ret  == 0)
					{
						PRINTF("Finished Decoding File!\n");
						float eqPoints[8]={0,0,0,0,0,0,0,0};
						MP3UiSetSongInfo(NULL,NULL,dur/1000,0,volume,eqPoints);
						MP3DEC.unloadFile();
						SigGen.stop();
						break;
					}

				}
				lastStatus=status;
			}

		}
		else if(play==0)
		{
			SigGen.pause();
		}
	}
	else
	{
		SigGen.stop();
	}
}

MP3Player_ MP3Player={.init=init, .update=update};
