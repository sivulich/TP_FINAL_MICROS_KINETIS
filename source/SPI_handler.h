#ifndef SPI_handler
#define	SPI_handler

#include <stdint.h>

/*#define DISP_SPI_MOSI 13
#define DISP_SPI_CLK  14
#define DISP_SPI_CS   5*/

extern volatile unsigned spiFlush;

void SPI_Handler_Init();

void SPI_Write_DMA(uint8_t* data , unsigned len,unsigned cs);

void SPI2_Write_DMA(uint8_t* data , unsigned len,unsigned cs);

void SPI_Write_Blocking(uint8_t* data , unsigned len,unsigned cs);

#endif


