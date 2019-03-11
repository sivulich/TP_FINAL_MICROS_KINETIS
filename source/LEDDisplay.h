/*
 * LEDDisplay.h
 *
 *  Created on: Feb 18, 2019
 *      Author: santi
 */

#ifndef LEDDISPLAY_H_
#define LEDDISPLAY_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
typedef struct LEDDisplay_ LEDDisplay_;


extern LEDDisplay_ LEDDisplay;

struct LEDDisplay_{
	void (*init)();

	void (*setVumeter)(short* input,unsigned len,int mode);

	float* (*getEqualizer)();
};
#ifdef __cplusplus
}
#endif

#endif /* LEDDISPLAY_H_ */
