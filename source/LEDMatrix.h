/*
 * LedMatrix.h
 *
 *  Created on: Feb 17, 2019
 *      Author: santi
 */

#ifndef LEDMATRIX_H_
#define LEDMATRIX_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
typedef struct LEDMatrix_ LEDMatrix_;


extern LEDMatrix_ LEDMatrix;

struct LEDMatrix_{
	void (*init)();

	void (*setPoint)(int x,int y,uint8_t r, uint8_t g, uint8_t b);

	void (*update)();
};
#ifdef __cplusplus
}
#endif

#endif /* LEDMATRIX_H_ */
