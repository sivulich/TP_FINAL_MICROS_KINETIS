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
#include "LEDMatrix.h"

#define ARM_MATH_CM4 1

#include "arm_math.h"
#include "arm_const_structs.h"
#include "fsl_debug_console.h"	//para el PRINTF

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define PING_PONG_BUFFS 10
/* buffer size (in byte) for read/write operations */
#define BUFFER_SIZE (100U)

#define FFT_LENGTH 1024
uint8_t eqColors[8][3]={{0,10,0},
						{0,10,0},
						{0,10,0},
						{10,10,0},
						{10,10,0},
						{10,10,0},
						{20,10,0},
						{20,0,0}};




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
static int init(int* p,int* v);
static void update();
static arm_rfft_instance_f32 rfft_inst;
static arm_cfft_radix4_instance_f32 cfft_inst;
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
	LEDMatrix.init();
	for(int i=0;i<8;i++)
		for(int j=0;j<8;j++)
			LEDMatrix.setPoint(i,j,0,0,0);
	LEDMatrix.update();
	arm_rfft_init_f32(&rfft_inst, &cfft_inst, FFT_LENGTH, 0, 1);
	return 0;
}
float eqPoints[4][8]={{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0}};
int currEqualizer=0;

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
			MP3UiSetSongInfo((char*)MP3DEC.getMP3Info("TIT2",&len),(char*)MP3DEC.getMP3Info("TPE1",&len),dur/1000,1,volume,NULL);
			while(len<=0 )
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
					float32_t input[1152], output[2*1152];
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
						for(int i=0;i<8;i++)
							for(int j=0;j<8;j++)
							{
								if(eqPoints[(currEqualizer+1)%4][i]>15*(j+1))
									LEDMatrix.setPoint(i,7-j,eqColors[j][0],eqColors[j][1],eqColors[j][2]);
								else
									LEDMatrix.setPoint(i,7-j,0,0,0);
							}
						LEDMatrix.update();

						arm_rfft_f32(&rfft_inst, input, output);
						arm_cmplx_mag_f32(output, input, FFT_LENGTH);
						for(int i=0;i<8;i++)
							eqPoints[currEqualizer][i]=0;
						for(int i=0;i<2;i++)
							eqPoints[currEqualizer][0]+=(float)input[i]*2.0/2;
						for(int i=2;i<5;i++)
							eqPoints[currEqualizer][1]+=(float)input[i]*8.0/3;
						for(int i=5;i<10;i++)
							eqPoints[currEqualizer][2]+=(float)input[i]*8.0/5;
						for(int i=10;i<23;i++)
							eqPoints[currEqualizer][3]+=(float)input[i]*8.0/13;
						for(int i=23;i<49;i++)
							eqPoints[currEqualizer][4]+=(float)input[i]*8.0/26;
						for(int i=49;i<108;i++)
							eqPoints[currEqualizer][5]+=(float)input[i]*8.0/61;
						for(int i=108;i<235;i++)
							eqPoints[currEqualizer][6]+=(float)input[i]*8.0/127;
						for(int i=235;i<512;i++)
							eqPoints[currEqualizer][7]+=(float)input[i]*8.0/277;


						currEqualizer=(currEqualizer+1)%4;
						MP3UiSetSongInfo(NULL,NULL,pos/1000,0,volume,eqPoints[currEqualizer]);

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
