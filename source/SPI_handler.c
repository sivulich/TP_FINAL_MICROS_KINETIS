#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_dspi.h"
#include "board.h"
#include "fsl_dspi_edma.h"
#include "fsl_edma.h"
#include "fsl_dmamux.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "SPI_handler.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_DSPI_MASTER_BASEADDR SPI0
#define EXAMPLE_DSPI_MASTER_DMA_MUX_BASEADDR DMAMUX
#define EXAMPLE_DSPI_MASTER_DMA_BASEADDR DMA0
#define EXAMPLE_DSPI_MASTER_DMA_RX_REQUEST_SOURCE kDmaRequestMux0SPI0Rx
#define EXAMPLE_DSPI_MASTER_DMA_TX_REQUEST_SOURCE kDmaRequestMux0SPI0Tx
#define DSPI_MASTER_CLK_SRC DSPI0_CLK_SRC
#define DSPI_MASTER_CLK_FREQ CLOCK_GetFreq(DSPI0_CLK_SRC)
#define EXAMPLE_DSPI_MASTER_PCS_FOR_INIT kDSPI_Pcs0
#define EXAMPLE_DSPI_MASTER_PCS_FOR_TRANSFER kDSPI_MasterPcs0
#define EXAMPLE_DSPI_DEALY_COUNT 0xfffffU
#define TRANSFER_SIZE 240*320*3        /* Transfer dataSize */
#define TRANSFER_BAUDRATE 20000000U /* Transfer baudrate - 20M */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* DSPI user callback */
//void DSPI_MasterUserCallback(SPI_Type *base, dspi_master_edma_handle_t *handle, status_t status, void *userData);

/*******************************************************************************
 * Variables
 ******************************************************************************/

dspi_master_edma_handle_t g_dspi_edma_m_handle;


uint32_t i;
dspi_master_config_t masterConfig;
dspi_transfer_t masterXfer;

#define BUFFER_LEN (200) //193 are needed to write all the memory
dspi_master_handle_t spiHandle;
dspi_master_config_t masterConfig;
dspi_transfer_t xfer;

volatile bool isFinished = false;
//static  uint8_t sendData[BUFFER_LEN];
uint8_t receiveBuff[BUFFER_LEN];





void SPI_Handler_Init()
{

	uint32_t srcClock_Hz;
	dspi_master_config_t masterConfig;


	DSPI_MasterGetDefaultConfig(&masterConfig);
	masterConfig.whichCtar = kDSPI_Ctar0;
	masterConfig.ctarConfig.baudRate = TRANSFER_BAUDRATE;
	masterConfig.ctarConfig.bitsPerFrame = 8;
	masterConfig.ctarConfig.cpol = kDSPI_ClockPolarityActiveHigh;
	masterConfig.ctarConfig.cpha = kDSPI_ClockPhaseFirstEdge;
	masterConfig.ctarConfig.direction = kDSPI_MsbFirst;
	masterConfig.ctarConfig.pcsToSckDelayInNanoSec = 1000000000U / TRANSFER_BAUDRATE;
	masterConfig.ctarConfig.lastSckToPcsDelayInNanoSec = 1000000000U / TRANSFER_BAUDRATE;
	masterConfig.ctarConfig.betweenTransferDelayInNanoSec = 1000000000U / TRANSFER_BAUDRATE;

	masterConfig.whichPcs = EXAMPLE_DSPI_MASTER_PCS_FOR_INIT;
	masterConfig.pcsActiveHighOrLow = kDSPI_PcsActiveLow;

	masterConfig.enableContinuousSCK = false;
	masterConfig.enableRxFifoOverWrite = false;
	masterConfig.enableModifiedTimingFormat = false;
	masterConfig.samplePoint = kDSPI_SckToSin0Clock;
	srcClock_Hz = DSPI_MASTER_CLK_FREQ;
	DSPI_MasterInit(EXAMPLE_DSPI_MASTER_BASEADDR, &masterConfig, srcClock_Hz);

}


void SPI_Write(uint8_t* data , uint16_t len)
{

	if (len == 0) return;           //no need to send anything
	status_t ret;
	masterXfer.txData = data;
    masterXfer.dataSize = len;
    masterXfer.rxData = NULL;
	masterXfer.configFlags = kDSPI_MasterCtar0 | EXAMPLE_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
	ret=DSPI_MasterTransferBlocking(EXAMPLE_DSPI_MASTER_BASEADDR, &masterXfer);
	if (kStatus_Success != ret)
		PRINTF("There is error when start DSPI_MasterTransferEDMA \r\n ");
	//while(g_dspi_edma_m_handle.state==kDSPI_Busy);
}

