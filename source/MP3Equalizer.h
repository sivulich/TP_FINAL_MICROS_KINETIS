/*
 * MP3Equalizer.h
 *
 *  Created on: Mar 8, 2019
 *      Author: Santi
 */

#ifndef MP3EQUALIZER_H_
#define MP3EQUALIZER_H_

typedef struct MP3Equalizer_ MP3Equalizer_;

extern MP3Equalizer_ MP3Equalizer;

struct MP3Equalizer_{
	void (*init)(int fs);

	void (*equalize)(unsigned short* input,unsigned short* output, int len, int dir);

	void(*setGains)(int* gains);

};

#endif /* MP3EQUALIZER_H_ */
