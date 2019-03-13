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
#define FILTER_COEFFS		6
#define FILTER_STATE_VARS	4
#define FILTER_STAGES		2

typedef struct
{
	q15_t coeff[FILTER_COEFFS * FILTER_STAGES];// b0, b1, b2, a1, a2
	q15_t varState[FILTER_STATE_VARS * FILTER_STAGES];//x_1, x_2, y_1, y_2;
}filter_t;

//static q15_t stateVarL[4];
//static q15_t stateVarR[4];
float equalizerCoeffs[13][3][12]=
{{{1.000000,0.000000,-1.958033,0.962414,1.936966,-0.942663,1.000000,0.000000,-1.978253,0.979540,1.975057,-0.976035},{1.000000,0.000000,-1.653665,0.750907,1.499003,-0.641085,1.000000,0.000000,-1.889238,0.902590,1.895311,-0.903755},{1.000000,0.000000,-0.910486,0.483322,0.599556,-0.344679,1.000000,0.000000,-1.654973,0.749584,1.693067,-0.752860}},
{{1.000000,0.000000,-1.955164,0.959711,1.936966,-0.942663,1.000000,0.000000,-1.977635,0.978873,1.975057,-0.976035},{1.000000,0.000000,-1.631877,0.734653,1.499003,-0.641085,1.000000,0.000000,-1.889348,0.901838,1.895311,-0.903755},{1.000000,0.000000,-0.864172,0.459630,0.599556,-0.344679,1.000000,0.000000,-1.659425,0.747851,1.693067,-0.752860}},
{{1.000000,0.000000,-1.952065,0.956796,1.936966,-0.942663,1.000000,0.000000,-1.977041,0.978228,1.975057,-0.976035},{1.000000,0.000000,-1.608572,0.717543,1.499003,-0.641085,1.000000,0.000000,-1.889808,0.901447,1.895311,-0.903755},{1.000000,0.000000,-0.815607,0.435899,0.599556,-0.344679,1.000000,0.000000,-1.664643,0.746992,1.693067,-0.752860}},
{{1.000000,0.000000,-1.948718,0.953651,1.936966,-0.942663,1.000000,0.000000,-1.976478,0.977614,1.975057,-0.976035},{1.000000,0.000000,-1.583685,0.699593,1.499003,-0.641085,1.000000,0.000000,-1.890633,0.901436,1.895311,-0.903755},{1.000000,0.000000,-0.764805,0.412323,0.599556,-0.344679,1.000000,0.000000,-1.670636,0.747052,1.693067,-0.752860}},
{{1.000000,0.000000,-1.945100,0.950261,1.936966,-0.942663,1.000000,0.000000,-1.975954,0.977038,1.975057,-0.976035},{1.000000,0.000000,-1.557157,0.680834,1.499003,-0.641085,1.000000,0.000000,-1.891829,0.901818,1.895311,-0.903755},{1.000000,0.000000,-0.711808,0.389111,0.599556,-0.344679,1.000000,0.000000,-1.677395,0.748054,1.693067,-0.752860}},
{{1.000000,0.000000,-1.941191,0.946604,1.936966,-0.942663,1.000000,0.000000,-1.975477,0.976508,1.975057,-0.976035},{1.000000,0.000000,-1.528942,0.661312,1.499003,-0.641085,1.000000,0.000000,-1.893393,0.902594,1.895311,-0.903755},{1.000000,0.000000,-0.656690,0.366487,0.599556,-0.344679,1.000000,0.000000,-1.684890,0.749998,1.693067,-0.752860}},
{{1.000000,0.000000,0.000000,0.000000,-0.000000,-0.000000,1.000000,0.000000,0.000000,0.000000,-0.000000,-0.000000},{1.000000,0.000000,0.000000,0.000000,-0.000000,-0.000000,1.000000,0.000000,0.000000,0.000000,-0.000000,-0.000000},{1.000000,0.000000,0.000000,0.000000,-0.000000,-0.000000,1.000000,0.000000,0.000000,0.000000,-0.000000,-0.000000}},
{{1.000000,0.000000,-1.932401,0.938416,1.936966,-0.942663,1.000000,0.000000,-1.974703,0.975627,1.975057,-0.976035},{1.000000,0.000000,-1.467313,0.620228,1.499003,-0.641085,1.000000,0.000000,-1.897560,0.905282,1.895311,-0.903755},{1.000000,0.000000,-0.540542,0.323912,0.599556,-0.344679,1.000000,0.000000,-1.701858,0.756595,1.693067,-0.752860}},
{{1.000000,0.000000,-1.927472,0.933845,1.936966,-0.942663,1.000000,0.000000,-1.974424,0.975293,1.975057,-0.976035},{1.000000,0.000000,-1.433857,0.598824,1.499003,-0.641085,1.000000,0.000000,-1.900107,0.907147,1.895311,-0.903755},{1.000000,0.000000,-0.479809,0.304399,0.599556,-0.344679,1.000000,0.000000,-1.711173,0.761136,1.693067,-0.752860}},
{{1.000000,0.000000,-1.922152,0.928930,1.936966,-0.942663,1.000000,0.000000,-1.974229,0.975044,1.975057,-0.976035},{1.000000,0.000000,-1.398627,0.576967,1.499003,-0.641085,1.000000,0.000000,-1.902915,0.909312,1.895311,-0.903755},{1.000000,0.000000,-0.417541,0.286336,0.599556,-0.344679,1.000000,0.000000,-1.720916,0.766397,1.693067,-0.752860}},
{{1.000000,0.000000,-1.916417,0.923651,1.936966,-0.942663,1.000000,0.000000,-1.974124,0.974885,1.975057,-0.976035},{1.000000,0.000000,-1.361622,0.554757,1.499003,-0.641085,1.000000,0.000000,-1.905938,0.911736,1.895311,-0.903755},{1.000000,0.000000,-0.353937,0.269894,0.599556,-0.344679,1.000000,0.000000,-1.730982,0.772283,1.693067,-0.752860}},
{{1.000000,0.000000,-1.910241,0.917993,1.936966,-0.942663,1.000000,0.000000,-1.974117,0.974825,1.975057,-0.976035},{1.000000,0.000000,-1.322848,0.532295,1.499003,-0.641085,1.000000,0.000000,-1.909132,0.914374,1.895311,-0.903755},{1.000000,0.000000,-0.289206,0.255216,0.599556,-0.344679,1.000000,0.000000,-1.741268,0.778692,1.693067,-0.752860}},
{{1.000000,0.000000,-1.903600,0.911939,1.936966,-0.942663,1.000000,0.000000,-1.974211,0.974867,1.975057,-0.976035},{1.000000,0.000000,-1.282313,0.509688,1.499003,-0.641085,1.000000,0.000000,-1.912450,0.917180,1.895311,-0.903755},{1.000000,0.000000,-0.223562,0.242415,0.599556,-0.344679,1.000000,0.000000,-1.751674,0.785519,1.693067,-0.752860}}
};


static filter_t filterBandsL[EQ_BANDS];
static filter_t filterBandsR[EQ_BANDS];
static arm_biquad_casd_df1_inst_q15 filterL[EQ_BANDS];
static arm_biquad_casd_df1_inst_q15 filterR[EQ_BANDS];

static float freqBands[EQ_BANDS] = {134,774,4000};
static float qFactor[EQ_BANDS] = {0.05,1,0.9};
static int fs;

filter_t calcCoeffs(float f, float g, float q);

void setGains(int* gains)
{
	for(int i = 0; i < EQ_BANDS; i++)
	{
		//q15_t lastVars[4];
		filterBandsL[i] = calcCoeffs(freqBands[i], gains[i], qFactor[i]);
		/*lastVars[0]=filterBandsL[i].varState[0];
		lastVars[1]=filterBandsL[i].varState[1];
		lastVars[2]=filterBandsL[i].varState[2];
		lastVars[3]=filterBandsL[i].varState[3];*/
		arm_biquad_cascade_df1_init_q15(filterL+i,2,filterBandsL[i].coeff,filterBandsL[i].varState,3);
		/*filterBandsL[i].varState[0]=lastVars[0];
		filterBandsL[i].varState[1]=lastVars[1];
		filterBandsL[i].varState[2]=lastVars[2];
		filterBandsL[i].varState[3]=lastVars[3];*/
		filterBandsR[i] = filterBandsL[i];
		/*lastVars[0]=filterBandsR[i].varState[0];
		lastVars[1]=filterBandsR[i].varState[1];
		lastVars[2]=filterBandsR[i].varState[2];
		lastVars[3]=filterBandsR[i].varState[3];*/
		arm_biquad_cascade_df1_init_q15(filterR+i,2,filterBandsR[i].coeff,filterBandsR[i].varState,3);
		/*filterBandsR[i].varState[0]=lastVars[0];
		filterBandsR[i].varState[1]=lastVars[1];
		filterBandsR[i].varState[2]=lastVars[2];
		filterBandsR[i].varState[3]=lastVars[3];*/
	}
}

void init(int fsample)
{
	fs = fsample;
	setGains(MP3PlayerData.equalizeBands);
}

void equalize(unsigned short* input,unsigned short* output, int len, int dir)
{
	//float outputTemp[1152];
	if(dir == 0)
	{
		arm_biquad_cascade_df1_q15(filterL,(q15_t*)input,(q15_t*)output,len);
		//arm_biquad_cascade_df1_q15(filterL,(q15_t*)output,(q15_t*)input,len);
		arm_biquad_cascade_df1_q15(filterL+1,(q15_t*)output,(q15_t*)input,len);
		//arm_biquad_cascade_df1_q15(filterL+1,(q15_t*)output,(q15_t*)input,len);
		arm_biquad_cascade_df1_q15(filterL+2,(q15_t*)input,(q15_t*)output,len);
		//arm_biquad_cascade_df1_q15(filterL+2,(q15_t*)output,(q15_t*)input,len);
	}
	else
	{
		arm_biquad_cascade_df1_q15(filterR,(q15_t*)input,(q15_t*)output,len);
		//arm_biquad_cascade_df1_q15(filterR,(q15_t*)output,(q15_t*)input,len);
		arm_biquad_cascade_df1_q15(filterR+1,(q15_t*)output,(q15_t*)input,len);
		//arm_biquad_cascade_df1_q15(filterR+1,(q15_t*)output,(q15_t*)input,len);
		arm_biquad_cascade_df1_q15(filterR+2,(q15_t*)input,(q15_t*)output,len);
		//arm_biquad_cascade_df1_q15(filterR+2,(q15_t*)output,(q15_t*)input,len);
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

	int gain=g+EQ_MAX_DB,band=0;
	for(int i=0;i<EQ_BANDS;i++)
		if(f==freqBands[i])
		{	band=i;break;}

	for(int i=0;i<(FILTER_COEFFS * FILTER_STAGES);i++)
		filter.coeff[i] = F2Q15(equalizerCoeffs[gain][band][i]);


	//filter.varState[0] = filter.varState[1] = filter.varState[2] = filter.varState[3] = 0;
	return filter;
}

MP3Equalizer_ MP3Equalizer = {.init = init, .equalize = equalize, .setGains = setGains};

