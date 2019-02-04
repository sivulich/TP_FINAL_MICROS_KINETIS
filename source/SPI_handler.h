#ifndef SPI_handler
#define	SPI_handler

#include <stdint.h>

#define DISP_SPI_MOSI 13
#define DISP_SPI_CLK  14
#define DISP_SPI_CS   5

void SPI_Handler_Init();

void SPI_Write(uint8_t* data , uint16_t len);

#endif


