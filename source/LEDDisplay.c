/*
 * LEDDisplay.c
 *
 *  Created on: Feb 18, 2019
 *      Author: santi
 */


#include "LEDDisplay.h"
#include "LEDMatrix.h"
#define ARM_MATH_CM4 1
#define FFT_LENGTH 1024
#include "arm_math.h"
#include "arm_const_structs.h"

static uint8_t eqColors[10][3]={{0,10,0},
								{4,10,0},
								{8,10,0},
								{10,10,0},
								{14,10,0},
								{18,10,0},
								{20,10,0},
								{20,0,0},
								{40,0,0}};
static float eqPoints[4][8]={{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0}};
static int currEqualizer=0;
static arm_rfft_instance_f32 rfft_inst;
static arm_cfft_radix4_instance_f32 cfft_inst;

static void init()
{
	LEDMatrix.init();
	for(int i=0;i<8;i++)
		for(int j=0;j<8;j++)
			LEDMatrix.setPoint(i,j,0,0,0);
	LEDMatrix.update();
	arm_rfft_init_f32(&rfft_inst, &cfft_inst, 1024, 0, 1);
}

static void barVumeter()
{
	for(int i=0;i<8;i++)
		for(int j=0;j<8;j++)
		{
			if(eqPoints[(currEqualizer+1)%4][i]>15*(j+1))
				LEDMatrix.setPoint(j,i,eqColors[j][0],eqColors[j][1],eqColors[j][2]);
			else
				LEDMatrix.setPoint(j,i,0,0,0);
		}
}
static void circ4Vumeter()
{
	float points[4]={	eqPoints[(currEqualizer+1)%4][0]+eqPoints[(currEqualizer+1)%4][1],
						eqPoints[(currEqualizer+1)%4][2]+eqPoints[(currEqualizer+1)%4][3],
						eqPoints[(currEqualizer+1)%4][4]+eqPoints[(currEqualizer+1)%4][5],
						eqPoints[(currEqualizer+1)%4][6]+eqPoints[(currEqualizer+1)%4][7]};
	int multiplier=50;
	for(int i=0;i<4;i++)
	{
		for(int j=0;j<4;j++)
		{
			if(points[0]>multiplier*sqrt((3.5-i)*(3.5-i)+(3.5-j)*(3.5-j)))
			{
				int point=2*floor(sqrt((3.5-i)*(3.5-i)+(3.5-j)*(3.5-j)));
				LEDMatrix.setPoint(i,j,eqColors[point][0],eqColors[point][1],eqColors[point][2]);
			}
			else
				LEDMatrix.setPoint(i,j,0,0,0);
		}
	}
	for(int i=4;i<8;i++)
	{
		for(int j=0;j<4;j++)
		{
			if(points[1]>multiplier*sqrt((3.5-i)*(3.5-i)+(3.5-j)*(3.5-j)))
			{
				int point=2*floor(sqrt((3.5-i)*(3.5-i)+(3.5-j)*(3.5-j)));
				LEDMatrix.setPoint(i,j,eqColors[point][0],eqColors[point][1],eqColors[point][2]);
			}
			else
				LEDMatrix.setPoint(i,j,0,0,0);
		}
	}
	for(int i=4;i<8;i++)
	{
		for(int j=4;j<8;j++)
		{
			if(points[2]>multiplier*sqrt((3.5-i)*(3.5-i)+(3.5-j)*(3.5-j)))
			{
				int point=2*floor(sqrt((3.5-i)*(3.5-i)+(3.5-j)*(3.5-j)));
				LEDMatrix.setPoint(i,j,eqColors[point][0],eqColors[point][1],eqColors[point][2]);
			}
			else
				LEDMatrix.setPoint(i,j,0,0,0);
		}
	}
	for(int i=0;i<4;i++)
	{
		for(int j=4;j<8;j++)
		{
			if(points[3]>multiplier*sqrt((3.5-i)*(3.5-i)+(3.5-j)*(3.5-j)))
			{
				int point=2*floor(sqrt((3.5-i)*(3.5-i)+(3.5-j)*(3.5-j)));
				LEDMatrix.setPoint(i,j,eqColors[point][0],eqColors[point][1],eqColors[point][2]);
			}
			else
				LEDMatrix.setPoint(i,j,0,0,0);
		}
	}
}
static void corners1Vumeter()
{
	float points[1]={	eqPoints[(currEqualizer+1)%4][0]+eqPoints[(currEqualizer+1)%4][1]+
						eqPoints[(currEqualizer+1)%4][2]+eqPoints[(currEqualizer+1)%4][3]+
						eqPoints[(currEqualizer+1)%4][4]+eqPoints[(currEqualizer+1)%4][5]+
						eqPoints[(currEqualizer+1)%4][6]+eqPoints[(currEqualizer+1)%4][7]};
	int multiplier=100;
	for(int i=0;i<4;i++)
	{
		for(int j=0;j<4;j++)
		{
			if(points[0]>multiplier*sqrt((0-i)*(0-i)+(0-j)*(0-j)))
			{
				int point=2*floor(sqrt((0-i)*(0-i)+(0-j)*(0-j)));
				LEDMatrix.setPoint(i,j,eqColors[point][0],eqColors[point][1],eqColors[point][2]);
			}
			else
				LEDMatrix.setPoint(i,j,0,0,0);
		}
	}
	for(int i=4;i<8;i++)
	{
		for(int j=0;j<4;j++)
		{
			if(points[0]>multiplier*sqrt((7-i)*(7-i)+(0-j)*(0-j)))
			{
				int point=2*floor(sqrt((7-i)*(7-i)+(0-j)*(0-j)));
				LEDMatrix.setPoint(i,j,eqColors[point][0],eqColors[point][1],eqColors[point][2]);
			}
			else
				LEDMatrix.setPoint(i,j,0,0,0);
		}
	}
	for(int i=4;i<8;i++)
	{
		for(int j=4;j<8;j++)
		{
			if(points[0]>multiplier*sqrt((7-i)*(7-i)+(7-j)*(7-j)))
			{
				int point=2*floor(sqrt((7-i)*(7-i)+(7-j)*(7-j)));
				LEDMatrix.setPoint(i,j,eqColors[point][0],eqColors[point][1],eqColors[point][2]);
			}
			else
				LEDMatrix.setPoint(i,j,0,0,0);
		}
	}
	for(int i=0;i<4;i++)
	{
		for(int j=4;j<8;j++)
		{
			if(points[0]>multiplier*sqrt((0-i)*(0-i)+(7-j)*(7-j)))
			{
				int point=2*floor(sqrt((0-i)*(0-i)+(7-j)*(7-j)));
				LEDMatrix.setPoint(i,j,eqColors[point][0],eqColors[point][1],eqColors[point][2]);
			}
			else
				LEDMatrix.setPoint(i,j,0,0,0);
		}
	}
}
static void corners4Vumeter()
{
	float points[4]={	eqPoints[(currEqualizer+1)%4][0]+eqPoints[(currEqualizer+1)%4][1],
						eqPoints[(currEqualizer+1)%4][2]+eqPoints[(currEqualizer+1)%4][3],
						eqPoints[(currEqualizer+1)%4][4]+eqPoints[(currEqualizer+1)%4][5],
						eqPoints[(currEqualizer+1)%4][6]+eqPoints[(currEqualizer+1)%4][7]};
	int multiplier=50;
	for(int i=0;i<4;i++)
	{
		for(int j=0;j<4;j++)
		{
			if(points[0]>multiplier*sqrt((0-i)*(0-i)+(0-j)*(0-j)))
			{
				int point=2*floor(sqrt((0-i)*(0-i)+(0-j)*(0-j)));
				LEDMatrix.setPoint(i,j,eqColors[point][0],eqColors[point][1],eqColors[point][2]);
			}
			else
				LEDMatrix.setPoint(i,j,0,0,0);
		}
	}
	for(int i=4;i<8;i++)
	{
		for(int j=0;j<4;j++)
		{
			if(points[1]>multiplier*sqrt((7-i)*(7-i)+(0-j)*(0-j)))
			{
				int point=2*floor(sqrt((7-i)*(7-i)+(0-j)*(0-j)));
				LEDMatrix.setPoint(i,j,eqColors[point][0],eqColors[point][1],eqColors[point][2]);
			}
			else
				LEDMatrix.setPoint(i,j,0,0,0);
		}
	}
	for(int i=4;i<8;i++)
	{
		for(int j=4;j<8;j++)
		{
			if(points[2]>multiplier*sqrt((7-i)*(7-i)+(7-j)*(7-j)))
			{
				int point=2*floor(sqrt((7-i)*(7-i)+(7-j)*(7-j)));
				LEDMatrix.setPoint(i,j,eqColors[point][0],eqColors[point][1],eqColors[point][2]);
			}
			else
				LEDMatrix.setPoint(i,j,0,0,0);
		}
	}
	for(int i=0;i<4;i++)
	{
		for(int j=4;j<8;j++)
		{
			if(points[3]>multiplier*sqrt((0-i)*(0-i)+(7-j)*(7-j)))
			{
				int point=2*floor(sqrt((0-i)*(0-i)+(7-j)*(7-j)));
				LEDMatrix.setPoint(i,j,eqColors[point][0],eqColors[point][1],eqColors[point][2]);
			}
			else
				LEDMatrix.setPoint(i,j,0,0,0);
		}
	}
}

static void circ1Vumeter()
{
	float points[1]={	eqPoints[(currEqualizer+1)%4][0]+eqPoints[(currEqualizer+1)%4][1]+
						eqPoints[(currEqualizer+1)%4][2]+eqPoints[(currEqualizer+1)%4][3]+
						eqPoints[(currEqualizer+1)%4][4]+eqPoints[(currEqualizer+1)%4][5]+
						eqPoints[(currEqualizer+1)%4][6]+eqPoints[(currEqualizer+1)%4][7]};
	int multiplier=100;
	for(int i=0;i<8;i++)
	{
		for(int j=0;j<8;j++)
		{
			if(points[0]>multiplier*sqrt((3.5-i)*(3.5-i)+(3.5-j)*(3.5-j)))
			{
				int point=2*floor(sqrt((3.5-i)*(3.5-i)+(3.5-j)*(3.5-j)));
				LEDMatrix.setPoint(i,j,eqColors[point][0],eqColors[point][1],eqColors[point][2]);
			}
			else
				LEDMatrix.setPoint(i,j,0,0,0);
		}
	}
}
static void centerVumeter()
{
	int multiplier=20;
	for(int i=0;i<8;i++)
	{
		for(int j=0;j<8;j++)
		{
			if(eqPoints[(currEqualizer+1)%4][i]>multiplier*sqrt((3.5-j)*(3.5-j)))
			{
				int point=2*floor(sqrt((3.5-j)*(3.5-j)))+1;
				LEDMatrix.setPoint(i,j,eqColors[point][0],eqColors[point][1],eqColors[point][2]);
			}
			else
				LEDMatrix.setPoint(i,j,0,0,0);
		}
	}
}


static void setVumeter(short* in,unsigned outputSamps,int mode)
{
	if(mode==0)
		barVumeter();
	else if(mode==1)
		centerVumeter();
	else if(mode==2)
		corners1Vumeter();
	else if(mode==3)
		corners4Vumeter();
	else if(mode==4)
		circ1Vumeter();
	else if(mode==5)
		circ4Vumeter();

	unsigned len = FFT_LENGTH;
	LEDMatrix.update();
	float output[2*1152], input[1152];
	if(outputSamps>1152)
	{
		for(int i = 0; i < outputSamps/2; i++)
			input[i]=((int)in[2*i]/2+in[2*i+1]/2)*1.0/32768;
	}
	else
		for(int i = 0; i < outputSamps; i++)
			input[i]=(((int)in[i]))*1.0/32768;
	arm_rfft_f32(&rfft_inst, input, output);
	arm_cmplx_mag_f32(output, input, len);
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
}
static float* getEqualizer()
{
	return eqPoints[currEqualizer];
}

LEDDisplay_ LEDDisplay = {.init=init,.setVumeter=setVumeter,.getEqualizer=getEqualizer};
