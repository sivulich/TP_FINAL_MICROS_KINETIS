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
#include "lvgl/lvgl.h"
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
#define TRANSFER_SIZE  (240*320*3)         /* Transfer dataSize */
#define TRANSFER_BAUDRATE 10000000U /* Transfer baudrate - 50M */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* DSPI user callback */
void DSPI_MasterUserCallback(SPI_Type *base, dspi_master_edma_handle_t *handle, status_t status, void *userData);

/*******************************************************************************
 * Variables
 ******************************************************************************/


//dspi_master_handle_t g_m_handle;

dspi_master_edma_handle_t g_dspi_edma_m_handle;
edma_handle_t dspiEdmaMasterRxRegToRxDataHandle;

#if (!(defined(FSL_FEATURE_DSPI_HAS_GASKET) && FSL_FEATURE_DSPI_HAS_GASKET))
edma_handle_t dspiEdmaMasterTxDataToIntermediaryHandle;
#endif
edma_handle_t dspiEdmaMasterIntermediaryToTxRegHandle;

volatile bool isTransferCompleted = false;

uint32_t i;
dspi_master_config_t masterConfig;
dspi_transfer_t masterXfer;

#define BUFFER_LEN (200) //193 are needed to write all the memory
dspi_master_handle_t spiHandle;
dspi_master_config_t masterConfig;
dspi_transfer_t xfer;


volatile bool isFinished = false;
volatile unsigned messageLen=0;
volatile uint8_t* messageData;
volatile unsigned spiFlush=0;
void DSPI_MasterUserCallback(SPI_Type *base, dspi_master_edma_handle_t *handle, status_t status, void *userData)
{
    if (status == kStatus_Success)
    {
        //PRINTF("This is DSPI master edma transfer completed callback. \r\n\r\n");
    }
    //while(g_dspi_edma_m_handle.state==kDSPI_Busy);
    if(messageLen>32000)
    {
    	while(g_dspi_edma_m_handle.state==kDSPI_Busy);
    	masterXfer.txData = messageData;
    	masterXfer.dataSize = 32000;
        masterXfer.rxData = NULL;
    	masterXfer.configFlags = kDSPI_MasterCtar0 | EXAMPLE_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
    	DSPI_MasterTransferEDMA(EXAMPLE_DSPI_MASTER_BASEADDR, &g_dspi_edma_m_handle, &masterXfer);
    	messageData+=32000;
    	messageLen-=32000;
    }
    else if(messageLen>0)
    {
    	while(g_dspi_edma_m_handle.state==kDSPI_Busy);
		masterXfer.txData = messageData;
		masterXfer.dataSize = messageLen;
		masterXfer.rxData = NULL;
		masterXfer.configFlags = kDSPI_MasterCtar0 | EXAMPLE_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
		DSPI_MasterTransferEDMA(EXAMPLE_DSPI_MASTER_BASEADDR, &g_dspi_edma_m_handle, &masterXfer);
		messageLen=0;
    }
    else if(messageLen == 0 )
    {
    	isTransferCompleted = true;
    	if(spiFlush==1)
    	{
    		lv_flush_ready();
    		spiFlush=0;
    	}

    }

    //lv_vdb_flush();
    //lv_flush_ready();
}





void SPI_Handler_Init()
{

	uint32_t srcClock_Hz;
	dspi_master_config_t masterConfig;


	edma_config_t userConfig;
	uint32_t masterRxChannel, masterTxChannel;
	masterRxChannel = 0U;
	masterTxChannel = 1U;
	uint32_t masterIntermediaryChannel;
	masterIntermediaryChannel = 2U;
	DMAMUX_Init(EXAMPLE_DSPI_MASTER_DMA_MUX_BASEADDR);
	DMAMUX_SetSource(EXAMPLE_DSPI_MASTER_DMA_MUX_BASEADDR, masterRxChannel,
					 (uint8_t)EXAMPLE_DSPI_MASTER_DMA_RX_REQUEST_SOURCE);
	DMAMUX_EnableChannel(EXAMPLE_DSPI_MASTER_DMA_MUX_BASEADDR, masterRxChannel);
	DMAMUX_SetSource(EXAMPLE_DSPI_MASTER_DMA_MUX_BASEADDR, masterTxChannel,
					 (uint8_t)EXAMPLE_DSPI_MASTER_DMA_TX_REQUEST_SOURCE);
	DMAMUX_EnableChannel(EXAMPLE_DSPI_MASTER_DMA_MUX_BASEADDR, masterTxChannel);


	EDMA_GetDefaultConfig(&userConfig);

	EDMA_Init(EXAMPLE_DSPI_MASTER_DMA_BASEADDR, &userConfig);


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


	memset(&(dspiEdmaMasterRxRegToRxDataHandle), 0, sizeof(dspiEdmaMasterRxRegToRxDataHandle));
	memset(&(dspiEdmaMasterTxDataToIntermediaryHandle), 0, sizeof(dspiEdmaMasterTxDataToIntermediaryHandle));
	memset(&(dspiEdmaMasterIntermediaryToTxRegHandle), 0, sizeof(dspiEdmaMasterIntermediaryToTxRegHandle));
	EDMA_CreateHandle(&(dspiEdmaMasterRxRegToRxDataHandle), EXAMPLE_DSPI_MASTER_DMA_BASEADDR, masterRxChannel);
	EDMA_CreateHandle(&(dspiEdmaMasterTxDataToIntermediaryHandle), EXAMPLE_DSPI_MASTER_DMA_BASEADDR,
					  masterIntermediaryChannel);
	EDMA_CreateHandle(&(dspiEdmaMasterIntermediaryToTxRegHandle), EXAMPLE_DSPI_MASTER_DMA_BASEADDR, masterTxChannel);
	DSPI_MasterTransferCreateHandleEDMA(EXAMPLE_DSPI_MASTER_BASEADDR, &g_dspi_edma_m_handle, DSPI_MasterUserCallback,
										NULL, &dspiEdmaMasterRxRegToRxDataHandle,
										&dspiEdmaMasterTxDataToIntermediaryHandle,
										&dspiEdmaMasterIntermediaryToTxRegHandle);

	isTransferCompleted = true;

}


void SPI_Write_DMA(uint8_t* data , unsigned len)
{

	if (len == 0) return;           //no need to send anything
	while(g_dspi_edma_m_handle.state==kDSPI_Busy);
	messageLen = len;
	messageData = data;
	if(len>32000)
	{
		len=32000;
		messageLen-=32000;
		messageData+=32000;
	}
	else
	{
		messageLen=0;
	}
	masterXfer.txData = data;
	masterXfer.dataSize = len;
    masterXfer.rxData = NULL;
	masterXfer.configFlags = kDSPI_MasterCtar0 | EXAMPLE_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
	status_t ret = DSPI_MasterTransferEDMA(EXAMPLE_DSPI_MASTER_BASEADDR, &g_dspi_edma_m_handle, &masterXfer);
	isTransferCompleted = false;
	if (kStatus_Success != ret )
		PRINTF("There is error when start DSPI_MasterTransferEDMA \r\n ");
	//while(isTransferCompleted == false)
	//while(g_dspi_edma_m_handle.state==kDSPI_Busy);
}

void SPI_Write_Blocking(uint8_t* data , unsigned len)
{
	SPI_Write_DMA(data ,len);
	while(isTransferCompleted == false);
	while(g_dspi_edma_m_handle.state==kDSPI_Busy);
}

