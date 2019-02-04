/*
 * MP3Decoder.c
 *
 *  Created on: Feb 3, 2019
 *      Author: santi
 */
#include "MP3Decoder.h"
#include "ff.h"
#include "./helix/pub/mp3dec.h"
#include "fsl_debug_console.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


static HMP3Decoder dec;
static MP3FrameInfo info;
static FIL fp;
static int loadedFile=0;

//Decode info
static unsigned char *temp,mp3file[MP3_BUFFER_SIZE];
static int sz, frames = 0, skipped = 0,lastPos=MP3_BUFFER_SIZE,bufSize=MP3_BUFFER_SIZE,duration;
static UINT br;
static int headerSize;



static void printInfo(MP3FrameInfo info)
{
	PRINTF("\nDecoded a frame with:\nBitrate: %d\nBitsPerSample: %d\nLayer: %d\nNchans: %d\nOutSamp: %d\nSampRate: %d\nVersion: %d\n",
		info.bitrate,
		info.bitsPerSample,
		info.layer,
		info.nChans,
		info.outputSamps,
		info.samprate,
		info.version
	);
}

int onFile()
{
	return loadedFile;
}

unsigned char* getMP3Info(const char* tag,int* size)
{
	int i = 0, len = 0;
	char found = 0;
	for (i = 0; i < (headerSize - 4); i++)
	{
		if (memcmp(mp3file + i, tag, 4) == 0)
		{
			//checkear 4 bytes de size, 2 bytes de flag, 1 byte de encoding
			//leer contenido hasta /0
			i += 4;
			if (*(mp3file + i) != 0)
				break;
			i += 4 + 2;
			switch (*(mp3file + i))
			{
			case 0: case 3:
				//printf("UTF-8\n");
				found = 1;
				break;
			case 1: case 2:
				found = 1;
				break;
			default:
				break;
			}
			i += 1;
			break;
		}
	}
	if (found)
		while (*(mp3file + i + len) != 0)
			len++;
	*size = len;
	//devolver contenido
	return (mp3file + i);
}

static void unloadFile()
{
	if(loadedFile==0)
		return;
	f_close(&fp);
	loadedFile=0;
	headerSize=0;
	sz=0, frames = 0, skipped = 0,lastPos=MP3_BUFFER_SIZE,bufSize=MP3_BUFFER_SIZE,duration=0;
	return;
}

static int decode(short * out,unsigned* len)
{
	if(sz > 0)
	{
		memcpy(mp3file,mp3file+lastPos,MP3_BUFFER_SIZE-lastPos);
		f_read(&fp,mp3file+MP3_BUFFER_SIZE-lastPos,lastPos,&br);
		//Mover a load File
		int off = MP3FindSyncWord(mp3file, sz);
		if(headerSize==0)
			headerSize=off;
		else
			headerSize=0;
		if (off == -1)
			return -1;
		sz -= off;
		temp=mp3file+off;
		bufSize=MP3_BUFFER_SIZE-off;
		int code = MP3Decode(dec, &temp, &bufSize, out, 0);
		lastPos=MP3_BUFFER_SIZE-bufSize;
		sz-=lastPos;
		if (code == 0)
		{
			MP3GetLastFrameInfo(dec, &info);
			duration = ((double)(sz+lastPos)) / info.bitrate;
			frames++;
			*len=1152*info.nChans;
		}
		else
			return code;
	}
	else
		return 0;
	return duration;
}

static int loadFile(const char* fileName)
{
	if(loadedFile==1)
		return 1;
	f_open(&fp, fileName, FA_READ);

	if (fp.err)
	{
		PRINTF("Couldnt open file\nPress any key to exit\n");
		return -1;
	}
	loadedFile=1;
	sz = f_size(&fp);
	/*f_read(&fp,mp3file+MP3_BUFFER_SIZE-lastPos,lastPos,&br);
	//Mover a load File
	int off = MP3FindSyncWord(mp3file, sz);
	headerSize=off;
	lastPos=0;*/
	return 0;
}

static void init()
{
	dec = MP3InitDecoder();
	loadedFile=0;
	headerSize = 0;
	sz=0, frames = 0, skipped = 0,lastPos=MP3_BUFFER_SIZE,bufSize=MP3_BUFFER_SIZE,duration=0;
}

struct MP3DEC_ MP3DEC={.init=init,.decode=decode,.loadFile=loadFile,.unloadFile=unloadFile,.getMP3Info=getMP3Info,.onFile=onFile};
