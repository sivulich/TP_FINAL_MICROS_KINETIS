/*
 * MP3Equalizer.c
 *
 *  Created on: Mar 8, 2019
 *      Author: Santi
 */

#include "MP3Equalizer.h"
#include <math.h>

#define M_PI 3.1415926535897932384626433832795

static int freqs;
static int a0, a1, a2, b0, b1, b2;

void init(int fs)
{
	freqs = fs;
}

void equalize(short* input, short* output, int len)
{

}

static void calcCoeffs(float f, float g, float q) {
	float A, omega, cs, sn, alpha;
	A = pow(10, g / 40.0f);
	omega = (2 * M_PI * f) / freqs;
	sn = sin(omega);
	cs = cos(omega);
	alpha = sn / (2.0 * q);
	b0 = 1 + (alpha * A);
	b1 = -2 * cs;
	b2 = 1 - (alpha * A);
	a0 = 1 + (alpha / (float) A);
	a1 = -2 * cs;
	a2 = 1 - (alpha / (float) A);
}

MP3Equalizer_ MP3Equalizer = {.init = init, .equalize = equalize};

