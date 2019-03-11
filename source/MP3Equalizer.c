/*
 * MP3Equalizer.c
 *
 *  Created on: Mar 8, 2019
 *      Author: Santi
 */

#include "MP3Equalizer.h"
#include "MP3PlayerData.h"
#include <math.h>

#define M_PI 3.1415926535897932384626433832795

typedef struct
{
	int a0, a1, a2, b0, b1, b2;
	int x_0, x_1, x_2, y_0, y_1, y_2;
}filter_t;

static filter_t filterBandsL[EQ_BANDS];
static filter_t filterBandsR[EQ_BANDS];
//static int gainsVal[EQ_BANDS];
static float freqBands[EQ_BANDS] = {134,774,4000};
static float qFactor[EQ_BANDS] = {1,1,1};
static int fs;

filter_t calcCoeffs(float f, float g, float q);

void setGains(int* gains)
{
	for(int i = 0; i < EQ_BANDS; i++)
	{
		filterBandsL[i] = calcCoeffs(freqBands[i], gains[i], qFactor[i]);
		filterBandsR[i] = filterBandsL[i];
	}
}

void init(int fsample)
{
	fs = fsample;
	int g[EQ_BANDS] = {0,0,0};
	setGains(g);
}

void equalize(short* input, short* output, int len, int dir)
{
	float *in;
	float *out = (float*)output;

	for(int i = 0; i < len; i++)
	{
		in[i] = (((int)input[i]))*1.0/32768;
		//out[i] = (((int)input[i]))*1.0/32768;
	}

	for(int i = 0 ; i < EQ_BANDS; i++)
	{
		for(int j = 0; j < len; j++)
		{
			if(i == 0)
				out[j] = 0;

			if(dir == 0)
			{
				filterBandsL[i].x_0 = in[j];

				filterBandsL[i].y_0 = (filterBandsL[i].b0 * filterBandsL[i].x_0 + filterBandsL[i].b1 * filterBandsL[i].x_1 + filterBandsL[i].b2 * filterBandsL[i].x_2 - filterBandsL[i].a1 * filterBandsL[i].y_1 - filterBandsL[i].a2 * filterBandsL[i].y_2) / filterBandsL[i].a0;
				filterBandsL[i].x_2 = filterBandsL[i].x_1;
				filterBandsL[i].x_1 = filterBandsL[i].x_0;
				filterBandsL[i].y_2 = filterBandsL[i].y_1;
				filterBandsL[i].y_1 = filterBandsL[i].y_0;

				float temp = filterBandsL[i].y_0 / EQ_BANDS;
				output[j] += (short)temp;
			}
			else
			{
				filterBandsR[i].x_0 = in[j];

				filterBandsR[i].y_0 = (filterBandsR[i].b0 * filterBandsR[i].x_0 + filterBandsR[i].b1 * filterBandsR[i].x_1 + filterBandsR[i].b2 * filterBandsR[i].x_2 - filterBandsR[i].a1 * filterBandsR[i].y_1 - filterBandsR[i].a2 * filterBandsR[i].y_2) / filterBandsR[i].a0;
				filterBandsR[i].x_2 = filterBandsR[i].x_1;
				filterBandsR[i].x_1 = filterBandsR[i].x_0;
				filterBandsR[i].y_2 = filterBandsR[i].y_1;
				filterBandsR[i].y_1 = filterBandsR[i].y_0;

				float temp = filterBandsR[i].y_0 / EQ_BANDS;
				output[j] += (short)temp;
			}
		}
	}
}

filter_t calcCoeffs(float f, float g, float q)
{
	filter_t filter;
	float A, omega, cs, sn, alpha;
	A = pow(10, g / 40.0f);
	omega = (2 * M_PI * f) / fs;
	sn = sin(omega);
	cs = cos(omega);
	alpha = sn / (2.0 * q);
	filter.b0 = 1 + (alpha * A);
	filter.b1 = -2 * cs;
	filter.b2 = 1 - (alpha * A);
	filter.a0 = 1 + (alpha / (float) A);
	filter.a1 = -2 * cs;
	filter.a2 = 1 - (alpha / (float) A);
	filter.y_0 = filter.y_1 = filter.y_2 = filter.x_0 = filter.x_1 = filter.x_2 = 0;
	return filter;
}

MP3Equalizer_ MP3Equalizer = {.init = init, .equalize = equalize, .setGains = setGains};

