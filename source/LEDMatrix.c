/*
 * LEDMatrix.c
 *
 *  Created on: Feb 17, 2019
 *      Author: santi
 */
#include "LEDMatrix.h"
#include "SPI_handler.h"
#include "FTM_Handler.h"

#define LED_HEIGHT     	8
#define LED_WIDTH      	8
#define LED_BYTES      	3
#define BYTES_PER_BYTE 	12//8
#define ONE_CODE		0b111111110000
#define CERO_CODE		0b111100000000
#define OFFSET			80
#define DELAY			80
#define ARRAY_LENGTH	LED_HEIGHT*LED_WIDTH*LED_BYTES*BYTES_PER_BYTE+DELAY+OFFSET

#define GPIO_OUT		6U //22U - 16U

#define CERO_CERO_CODE 	0b000000000000111111110000
#define CERO_ONE_CODE 	0b111100000000111111110000
#define ONE_CERO_CODE 	0b000000000000111111111111
#define ONE_ONE_CODE 	0b111100000000111111111111


static uint8_t matrix[ARRAY_LENGTH];

static void init();

static void setPoint(int x,int y,uint8_t r, uint8_t g, uint8_t b);

static void update();


static void init()
{
	SPI_Handler_Init();
//	FTM_Handler_Init(matrix, ARRAY_LENGTH);
//	for(int i=0;i<8;i++)
//		for(int j=0;j<8;j++)
//			setPoint(i,j,10,10,0);
//	FTM_Start();
	update();
}

static void setPoint(int x,int y, uint8_t r, uint8_t g, uint8_t b)
{
//	int pos=(x+LED_WIDTH*y)*LED_BYTES*BYTES_PER_BYTE;
//	uint8_t* green = matrix + pos;
//	uint8_t* red = matrix + pos + BYTES_PER_BYTE;
//	uint8_t* blue = matrix + pos + 2*BYTES_PER_BYTE;
//
//	for (int i=0;i<BYTES_PER_BYTE;i++){
//		green[i] = ((g>>i)&1) << GPIO_OUT;
////		matrix[(x+y*LED_WIDTH)*LED_BYTES*BYTES_PER_BYTE+ i ] = ((g>>i)&1) << GPIO_OUT;
//	}
//	for (int i=0;i<BYTES_PER_BYTE;i++){
//		red[i] = ((r>>i)&1) << GPIO_OUT;
////		matrix[(x+y*LED_WIDTH)*LED_BYTES*BYTES_PER_BYTE+ BYTES_PER_BYTE + i ] = ((r>>i)&1) << GPIO_OUT;
//	}
//	for (int i=0;i<BYTES_PER_BYTE;i++){
//		blue[i] = ((b>>i)&1) << GPIO_OUT;
////		matrix[(x+y*LED_WIDTH)*LED_BYTES*BYTES_PER_BYTE+ 2*BYTES_PER_BYTE + i ] = ((b>>i)&1) << GPIO_OUT;
//	}
//	pos = 0;
	int pos=(x+LED_WIDTH*y)*LED_BYTES*BYTES_PER_BYTE+OFFSET;
	uint8_t* green = matrix+pos;

	uint8_t* red = matrix+pos+BYTES_PER_BYTE;
	uint8_t* blue = matrix+pos+2*BYTES_PER_BYTE;

	uint64_t colorL=0,colorH=0;
	for(int i=0;i<=3;i++)
	{
		colorH<<=24;
		colorH|=(colorL>>40);
		colorL<<=24;
		if(((g>>(2*i))&3)==0b00)
			colorL |= CERO_CERO_CODE;
		else if(((g>>(2*i))&3)==0b01)
			colorL |= CERO_ONE_CODE;
		else if(((g>>(2*i))&3)==0b10)
			colorL |= ONE_CERO_CODE;
		else if(((g>>(2*i))&3)==0b11)
			colorL |= ONE_ONE_CODE;
	}
	for(int i=0;i<8;i++)
	{
		green[i]=(colorL&0xFF);
		colorL>>=8;
	}
	for(int i=8;i<12;i++)
	{
		green[i]=(colorH&0xFF);
		colorH>>=8;
	}
	colorL=colorH=0;
	for(int i=0;i<=3;i++)
	{
		colorH<<=24;
		colorH|=(colorL>>40);
		colorL<<=24;
		if(((r>>(2*i))&3)==0b00)
			colorL |= CERO_CERO_CODE;
		else if(((r>>(2*i))&3)==0b01)
			colorL |= CERO_ONE_CODE;
		else if(((r>>(2*i))&3)==0b10)
			colorL |= ONE_CERO_CODE;
		else if(((r>>(2*i))&3)==0b11)
			colorL |= ONE_ONE_CODE;
	}
	for(int i=0;i<8;i++)
	{
		red[i]=(colorL&0xFF);
		colorL>>=8;
	}
	for(int i=8;i<12;i++)
	{
		red[i]=(colorH&0xFF);
		colorH>>=8;
	}
	colorL=colorH=0;
	for(int i=0;i<=3;i++)
	{
		colorH<<=24;
		colorH|=(colorL>>40);
		colorL<<=24;
		if(((b>>(2*i))&3)==0b00)
			colorL |= CERO_CERO_CODE;
		else if(((b>>(2*i))&3)==0b01)
			colorL |= CERO_ONE_CODE;
		else if(((b>>(2*i))&3)==0b10)
			colorL |= ONE_CERO_CODE;
		else if(((b>>(2*i))&3)==0b11)
			colorL |= ONE_ONE_CODE;
	}
	for(int i=0;i<8;i++)
	{
		blue[i]=(colorL&0xFF);
		colorL>>=8;
	}
	for(int i=8;i<12;i++)
	{
		blue[i]=(colorH&0xFF);
		colorH>>=8;
	}
	colorL=colorH=0;

}

static void update()
{
	SPI_Write_DMA(matrix,ARRAY_LENGTH,5);
//	FTM_Start();
}



struct LEDMatrix_ LEDMatrix={.init=init,.setPoint=setPoint,.update=update};
