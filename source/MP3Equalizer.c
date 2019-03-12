/*
 * MP3Equalizer.c
 *
 *  Created on: Mar 8, 2019
 *      Author: Santi
 */

#include "MP3Equalizer.h"
#include "MP3PlayerData.h"
#include <math.h>
#define ARM_MATH_CM4 1
#include "arm_math.h"
#include "arm_const_structs.h"

#define F2Q15(x)  ((q15_t)((float32_t)x * 16384UL))

#define M_PI 3.1415926535897932384626433832795

typedef struct
{
	q15_t coeff[6];// b0, b1, b2, a1, a2
	q15_t varState[4];//x_1, x_2, y_1, y_2;
}filter_t;

//static q15_t stateVarL[4];
//static q15_t stateVarR[4];

static filter_t filterBandsL[EQ_BANDS];
static filter_t filterBandsR[EQ_BANDS];
static arm_biquad_casd_df1_inst_q15 filterL[EQ_BANDS];
static arm_biquad_casd_df1_inst_q15 filterR[EQ_BANDS];

static float freqBands[EQ_BANDS] = {134,774,4000};
static float qFactor[EQ_BANDS] = {0.1,1,1};
static int fs;

filter_t calcCoeffs(float f, float g, float q);

void setGains(int* gains)
{
	for(int i = 0; i < EQ_BANDS; i++)
	{
		filterBandsL[i] = calcCoeffs(freqBands[i], gains[i], qFactor[i]);
		arm_biquad_cascade_df1_init_q15(filterL+i,1,filterBandsL[i].coeff,filterBandsL[i].varState,1);
		filterBandsR[i] = filterBandsL[i];
		arm_biquad_cascade_df1_init_q15(filterR+i,1,filterBandsR[i].coeff,filterBandsR[i].varState,1);
	}
}

void init(int fsample)
{
	fs = fsample;
	int g[EQ_BANDS] = {0,0,0};
	setGains(g);
}

void equalize(unsigned short* input,unsigned short* output, int len, int dir)
{
	//float outputTemp[1152];
	if(dir == 0)
	{
		arm_biquad_cascade_df1_q15(filterL,(q15_t*)input,(q15_t*)output,len);
		arm_biquad_cascade_df1_q15(filterL+1,(q15_t*)output,(q15_t*)input,len);
		arm_biquad_cascade_df1_q15(filterL+2,(q15_t*)input,(q15_t*)output,len);
	}
	else
	{
		arm_biquad_cascade_df1_q15(filterR,(q15_t*)input,(q15_t*)output,len);
		arm_biquad_cascade_df1_q15(filterR+1,(q15_t*)output,(q15_t*)input,len);
		arm_biquad_cascade_df1_q15(filterR+2,(q15_t*)input,(q15_t*)output,len);
	}
	//for(int i=0;i<1152;i++)
	//{
	//	output[i]=(outputTemp[i]);
	//}

	/*float in[1];
	//float *out = (float*)output;

	for(int i = 0; i < len; i++)
	{
		in[i] = (((int)input[i]))*1.0/32768;
		//out[i] = (((int)input[i]))*1.0/32768;
	}*/
	/*
	for(int i = 0 ; i < EQ_BANDS; i++)
	{
		for(int j = 0; j < len; j++)
		{
			if(dir == 0)
			{
				filterBandsL[i].x_0 = input[j];

				filterBandsL[i].y_0 = (filterBandsL[i].b0 * filterBandsL[i].x_0 + filterBandsL[i].b1 * filterBandsL[i].x_1 + filterBandsL[i].b2 * filterBandsL[i].x_2 - filterBandsL[i].a1 * filterBandsL[i].y_1 - filterBandsL[i].a2 * filterBandsL[i].y_2) / filterBandsL[i].a0;
				filterBandsL[i].x_2 = filterBandsL[i].x_1;
				filterBandsL[i].x_1 = filterBandsL[i].x_0;
				filterBandsL[i].y_2 = filterBandsL[i].y_1;
				filterBandsL[i].y_1 = filterBandsL[i].y_0;
				if(filterBandsL[i].y_0>65535)
					input[j] += 65535 / EQ_BANDS;
				else if(filterBandsL[i].y_0>0)
					input[j] += filterBandsL[i].y_0 / EQ_BANDS;
			}
			else
			{
				filterBandsR[i].x_0 = input[j];

				filterBandsR[i].y_0 = (filterBandsR[i].b0 * filterBandsR[i].x_0 + filterBandsR[i].b1 * filterBandsR[i].x_1 + filterBandsR[i].b2 * filterBandsR[i].x_2 - filterBandsR[i].a1 * filterBandsR[i].y_1 - filterBandsR[i].a2 * filterBandsR[i].y_2) / filterBandsR[i].a0;
				filterBandsR[i].x_2 = filterBandsR[i].x_1;
				filterBandsR[i].x_1 = filterBandsR[i].x_0;
				filterBandsR[i].y_2 = filterBandsR[i].y_1;
				filterBandsR[i].y_1 = filterBandsR[i].y_0;

				if(filterBandsR[i].y_0>65535)
					input[j] += 65535 / EQ_BANDS;
				else if(filterBandsR[i].y_0>0)
					input[j] += filterBandsR[i].y_0 / EQ_BANDS;
			}
		}
	}*/
}

filter_t calcCoeffs(float f, float g, float q)
{
	filter_t filter;
	float A, omega, cs, sn, alpha;
	A = pow(10, (g-EQ_MAX_DB) / 20.0f);
	omega = (2 * M_PI * f) / fs;
	sn = sin(omega);
	cs = cos(omega);
	alpha = sn / (2.0 * q);
	filter.coeff[0] = F2Q15((1 + (alpha * A))/(1 + (alpha /  A)));			//b_0
	filter.coeff[1] = 0;
	filter.coeff[2] = F2Q15(-2 * cs/(1 + (alpha /  A)));					//b_1
	filter.coeff[3] = F2Q15((1 - (alpha * A))/(1 + (alpha /  A)));			//b_2
	//filter.a0 = 1 + (alpha / (float) A);
	filter.coeff[4] = F2Q15(2 * cs/(1 + (alpha /  A)));					//a_1
	filter.coeff[5] = F2Q15(-(1 - (alpha / (float) A))/(1 + (alpha /  A)));	//a_2
/*
	filter.coeff[0] = F2Q15(0.1215975);			//b_0
	filter.coeff[1] = 0;
	filter.coeff[2] = F2Q15(0.0751514);					//b_1
	filter.coeff[3] = F2Q15(0.1215975);			//b_2
	//filter.a0 = 1 + (alpha / (float) A);
	filter.coeff[4] = F2Q15(0.9095030);					//a_1
	filter.coeff[5] = F2Q15(-0.2278495);	//a_2*/

	//filter.varState[0] = filter.varState[1] = filter.varState[2] = filter.varState[3] = 0;
	return filter;
}

MP3Equalizer_ MP3Equalizer = {.init = init, .equalize = equalize, .setGains = setGains};

