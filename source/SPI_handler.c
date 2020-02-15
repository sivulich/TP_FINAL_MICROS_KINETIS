#include "fsl_device_registers.h"
//#include "fsl_debug_console.h"
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
#define DSPI_MASTER_BASEADDR SPI0
#define DSPI_MASTER_DMA_MUX_BASEADDR DMAMUX
#define DSPI_MASTER_DMA_BASEADDR DMA0
#define DSPI_MASTER_DMA_RX_REQUEST_SOURCE kDmaRequestMux0SPI0Rx
#define DSPI_MASTER_DMA_TX_REQUEST_SOURCE kDmaRequestMux0SPI0Tx

//#define DSPI_MASTER2_BASEADDR SPI2
//#define DSPI_MASTER2_DMA_TRX_REQUEST_SOURCE kDmaRequestMux0SPI2

#define DSPI_MASTER_CLK_SRC DSPI0_CLK_SRC
#define DSPI_MASTER_CLK_FREQ CLOCK_GetFreq(DSPI0_CLK_SRC)
#define DSPI_MASTER_PCS_FOR_INIT kDSPI_Pcs0
#define DSPI_MASTER_PCS_FOR_TRANSFER kDSPI_MasterPcs0
#define DSPI_DEALY_COUNT 0xfffffU
#define TRANSFER_SIZE  (240*320*3)         /* Transfer dataSize */
#define TRANSFER_BAUDRATE 10000000U /* Transfer baudrate - 10M */
#define BUFFER_LEN (200) //193 are needed to write all the memory

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* DSPI user callback */
void DSPI_MasterUserCallback(SPI_Type *base, dspi_master_edma_handle_t *handle, status_t status, void *userData);
//void DSPI_Master2UserCallback(SPI_Type *base, dspi_master_edma_handle_t *handle, status_t status, void *userData);

/*******************************************************************************
 * Variables
 ******************************************************************************/


// SPI0 DMA variables -> LCD Display
dspi_master_edma_handle_t g_dspi_edma_m_handle;
edma_handle_t dspiEdmaMasterRxRegToRxDataHandle;

#if (!(defined(FSL_FEATURE_DSPI_HAS_GASKET) && FSL_FEATURE_DSPI_HAS_GASKET))
edma_handle_t dspiEdmaMasterTxDataToIntermediaryHandle;
#endif
edma_handle_t dspiEdmaMasterIntermediaryToTxRegHandle;

dspi_transfer_t masterXfer;
volatile bool isTransferCompleted = false;
//volatile bool isFinished = false;
volatile unsigned messageLen=0;
volatile uint8_t* messageData;
volatile unsigned spiFlush=0;

// SPI2 DMA variables -> LED Display
//dspi_master_edma_handle_t g_dspi2_edma_m_handle;
//edma_handle_t dspiEdmaMaster2RxRegToRxDataHandle;
//edma_handle_t dspiEdmaMaster2TxDataToIntermediaryHandle;
//edma_handle_t dspiEdmaMaster2IntermediaryToTxRegHandle;
//
//volatile bool isTransfer2Completed = false;
//dspi_transfer_t master2Xfer;
////volatile bool isFinished2 = false;
//volatile unsigned message2Len=0;
//volatile uint8_t* message2Data;
//volatile unsigned spi2Flush=0;


// Common variables
//uint32_t i;
dspi_master_config_t masterConfig;
//dspi_master_handle_t spiHandle;
dspi_master_config_t masterConfig;
dspi_transfer_t xfer;
volatile unsigned chipSelect=0;

void DSPI_MasterUserCallback(SPI_Type *base, dspi_master_edma_handle_t *handle, status_t status, void *userData)
{

	int MAX_LEN = 32000;
	dspi_transfer_t xtfer = masterXfer;
	if (status == kStatus_Success)
    {
        //PRINTF("This is DSPI master edma transfer completed callback. \r\n\r\n");
    }
    //while(g_dspi_edma_m_handle.state==kDSPI_Busy);
    if(messageLen>MAX_LEN)
    {
    	while(g_dspi_edma_m_handle.state==kDSPI_Busy);
    	xtfer.txData =(uint8_t*) messageData;
    	xtfer.dataSize = MAX_LEN;
    	xtfer.rxData = NULL;
    	xtfer.configFlags = kDSPI_MasterCtar0 |  (chipSelect << DSPI_MASTER_PCS_SHIFT) | kDSPI_MasterPcsContinuous;
    	DSPI_MasterTransferEDMA(base, &g_dspi_edma_m_handle, &xtfer);
    	messageData+=MAX_LEN;
    	messageLen-=MAX_LEN;
    }
    else if(messageLen>0)
    {
    	while(g_dspi_edma_m_handle.state==kDSPI_Busy){};
    	xtfer.txData = (uint8_t*)messageData;
    	xtfer.dataSize = messageLen;
    	xtfer.rxData = NULL;
    	xtfer.configFlags = kDSPI_MasterCtar0 |  (chipSelect << DSPI_MASTER_PCS_SHIFT) | kDSPI_MasterPcsContinuous;
		DSPI_MasterTransferEDMA(base, &g_dspi_edma_m_handle, &xtfer);
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

//void DSPI_Master2UserCallback(SPI_Type *base, dspi_master_edma_handle_t *handle, status_t status, void *userData)
//{
//
//	int MAX_LEN = 500;
//	dspi_transfer_t xtfer = master2Xfer;
//	if (status == kStatus_Success)
//    {
//        //PRINTF("This is DSPI master edma transfer completed callback. \r\n\r\n");
//    }
//    //while(g_dspi_edma_m_handle.state==kDSPI_Busy);
//    if(message2Len>MAX_LEN)
//    {
//    	while(g_dspi2_edma_m_handle.state==kDSPI_Busy);
//    	xtfer.txData =(uint8_t*) message2Data;
//    	xtfer.dataSize = MAX_LEN;
//    	xtfer.rxData = NULL;
//    	xtfer.configFlags = kDSPI_MasterCtar0 |  (0 << DSPI_MASTER_PCS_SHIFT) | kDSPI_MasterPcsContinuous;
//    	DSPI_MasterTransferEDMA(base, &g_dspi2_edma_m_handle, &xtfer);
//    	message2Data+=MAX_LEN;
//    	message2Len-=MAX_LEN;
//    }
//    else if(message2Len>0)
//    {
//    	while(g_dspi2_edma_m_handle.state==kDSPI_Busy){};
//    	xtfer.txData = (uint8_t*)message2Data;
//    	xtfer.dataSize = message2Len;
//    	xtfer.rxData = NULL;
//    	xtfer.configFlags = kDSPI_MasterCtar0 |  (0 << DSPI_MASTER_PCS_SHIFT) | kDSPI_MasterPcsContinuous;
//		DSPI_MasterTransferEDMA(base, &g_dspi2_edma_m_handle, &xtfer);
//		message2Len=0;
//    }
//    else if(message2Len == 0 )
//    {
//
//    	isTransfer2Completed = true;
//    	if(spi2Flush==1)
//    	{
//    		lv_flush_ready();
//    		spi2Flush=0;
//    	}
//
//    }
//
//    //lv_vdb_flush();
//    //lv_flush_ready();
//}



void SPI_Handler_Init()
{

	uint32_t srcClock_Hz;
	dspi_master_config_t masterConfig;

	edma_config_t userConfig;
	uint32_t masterRxChannel, masterTxChannel, master2RxChannel, master2TxChannel;
	masterRxChannel = 4U;
	masterTxChannel = 5U;
	uint32_t masterIntermediaryChannel, master2IntermediaryChannel;
	masterIntermediaryChannel = 6U;

	// For LED Display
//	master2RxChannel = 7U;
//	master2IntermediaryChannel = 8U;
//	master2TxChannel = 9U;

	//DMAMUX Init
	DMAMUX_Init(DSPI_MASTER_DMA_MUX_BASEADDR);
	DMAMUX_SetSource(DSPI_MASTER_DMA_MUX_BASEADDR, masterRxChannel,
					 (uint8_t)DSPI_MASTER_DMA_RX_REQUEST_SOURCE);
	DMAMUX_EnableChannel(DSPI_MASTER_DMA_MUX_BASEADDR, masterRxChannel);

	DMAMUX_SetSource(DSPI_MASTER_DMA_MUX_BASEADDR, masterTxChannel,
					 (uint8_t)DSPI_MASTER_DMA_TX_REQUEST_SOURCE);
	DMAMUX_EnableChannel(DSPI_MASTER_DMA_MUX_BASEADDR, masterTxChannel);

//	DMAMUX_SetSource(DSPI_MASTER_DMA_MUX_BASEADDR, master2RxChannel,
//						 (uint8_t)DSPI_MASTER2_DMA_TRX_REQUEST_SOURCE);
//	DMAMUX_EnableChannel(DSPI_MASTER_DMA_MUX_BASEADDR, master2RxChannel);


	EDMA_GetDefaultConfig(&userConfig);
	userConfig.enableRoundRobinArbitration=true;
	userConfig.enableDebugMode = false;
	EDMA_Init(DSPI_MASTER_DMA_BASEADDR, &userConfig);
	/*edma_channel_Preemption_config_t priorityConfig;
	priorityConfig.channelPriority=1;
	priorityConfig.enableChannelPreemption=1;
	priorityConfig.enablePreemptAbility=0;
	EDMA_SetChannelPreemptionConfig(EXAMPLE_DSPI_MASTER_DMA_BASEADDR,masterTxChannel,&priorityConfig);*/
	//EDMA_SetBandWidth(EXAMPLE_DSPI_MASTER_DMA_BASEADDR,masterTxChannel,kEDMA_BandwidthStall8Cycle);
	//EDMA_SetBandWidth(EXAMPLE_DSPI_MASTER_DMA_BASEADDR,masterRxChannel,kEDMA_BandwidthStall8Cycle);

	// SPI Init for SPI0 and SPI2
	DSPI_MasterGetDefaultConfig(&masterConfig);
	masterConfig.whichCtar = kDSPI_Ctar0;
	masterConfig.ctarConfig.baudRate = TRANSFER_BAUDRATE;
	masterConfig.ctarConfig.bitsPerFrame = 8;
	masterConfig.ctarConfig.cpol = kDSPI_ClockPolarityActiveHigh;
	masterConfig.ctarConfig.cpha = kDSPI_ClockPhaseFirstEdge;
	masterConfig.ctarConfig.direction = kDSPI_MsbFirst;
	masterConfig.ctarConfig.pcsToSckDelayInNanoSec =0;// 1000000000U / TRANSFER_BAUDRATE;
	masterConfig.ctarConfig.lastSckToPcsDelayInNanoSec =0;// 1000000000U / TRANSFER_BAUDRATE;
	masterConfig.ctarConfig.betweenTransferDelayInNanoSec =0;// 1000000000U / TRANSFER_BAUDRATE;

	masterConfig.whichPcs = DSPI_MASTER_PCS_FOR_INIT;
	masterConfig.pcsActiveHighOrLow = kDSPI_PcsActiveLow;

	masterConfig.enableContinuousSCK = false;
	masterConfig.enableRxFifoOverWrite = false;
	masterConfig.enableModifiedTimingFormat = false;
	masterConfig.samplePoint = kDSPI_SckToSin0Clock;
	srcClock_Hz = DSPI_MASTER_CLK_FREQ;
	DSPI_MasterInit(DSPI_MASTER_BASEADDR, &masterConfig, srcClock_Hz);
//	DSPI_MasterInit(DSPI_MASTER2_BASEADDR, &masterConfig, srcClock_Hz);

	//memset(&(dspiEdmaMasterRxRegToRxDataHandle), 0, sizeof(dspiEdmaMasterRxRegToRxDataHandle));
	//memset(&(dspiEdmaMasterTxDataToIntermediaryHandle), 0, sizeof(dspiEdmaMasterTxDataToIntermediaryHandle));
	//memset(&(dspiEdmaMasterIntermediaryToTxRegHandle), 0, sizeof(dspiEdmaMasterIntermediaryToTxRegHandle));
	EDMA_CreateHandle(&(dspiEdmaMasterRxRegToRxDataHandle), DSPI_MASTER_DMA_BASEADDR, masterRxChannel);
	EDMA_CreateHandle(&(dspiEdmaMasterTxDataToIntermediaryHandle), DSPI_MASTER_DMA_BASEADDR, masterIntermediaryChannel);
	EDMA_CreateHandle(&(dspiEdmaMasterIntermediaryToTxRegHandle), DSPI_MASTER_DMA_BASEADDR, masterTxChannel);
	DSPI_MasterTransferCreateHandleEDMA(DSPI_MASTER_BASEADDR, &g_dspi_edma_m_handle, DSPI_MasterUserCallback,
										NULL, &dspiEdmaMasterRxRegToRxDataHandle,
										&dspiEdmaMasterTxDataToIntermediaryHandle,
										&dspiEdmaMasterIntermediaryToTxRegHandle);

//	SPI for LED display
//	EDMA_CreateHandle(&(dspiEdmaMaster2RxRegToRxDataHandle), DSPI_MASTER_DMA_BASEADDR, master2RxChannel);
//	EDMA_CreateHandle(&(dspiEdmaMaster2TxDataToIntermediaryHandle), DSPI_MASTER_DMA_BASEADDR, master2IntermediaryChannel);
//	EDMA_CreateHandle(&(dspiEdmaMaster2IntermediaryToTxRegHandle), DSPI_MASTER_DMA_BASEADDR, master2TxChannel);
//	DSPI_MasterTransferCreateHandleEDMA(DSPI_MASTER2_BASEADDR, &g_dspi2_edma_m_handle, DSPI_Master2UserCallback,
//											NULL, &dspiEdmaMaster2RxRegToRxDataHandle,
//											&dspiEdmaMaster2TxDataToIntermediaryHandle,
//											&dspiEdmaMaster2IntermediaryToTxRegHandle);

	isTransferCompleted = true;
//	isTransfer2Completed = true;

}


void SPI_Write_DMA(uint8_t* data , unsigned len,unsigned cs)
{

	if (len == 0) return;           //no need to send anything
	while(isTransferCompleted == false);
	dspi_master_edma_handle_t spi_handle = g_dspi_edma_m_handle;
	dspi_transfer_t xtfer = masterXfer;
//	while(g_dspi2_edma_m_handle.state==kDSPI_Busy);
	while(g_dspi_edma_m_handle.state==kDSPI_Busy);
	messageLen = len;
	messageData = data;
	int MAX_LEN = 32000;

	if(len>MAX_LEN)
	{
		len=MAX_LEN;
		messageLen-=MAX_LEN;
		messageData+=MAX_LEN;
	}
	else
	{
		messageLen=0;
	}
	xtfer.txData = data;
	xtfer.dataSize = len;
	xtfer.rxData = NULL;
	xtfer.configFlags = kDSPI_MasterCtar0 | (cs << DSPI_MASTER_PCS_SHIFT) | kDSPI_MasterPcsContinuous;
    chipSelect=cs;
    status_t ret;
    ret = DSPI_MasterTransferEDMA(DSPI_MASTER_BASEADDR, &g_dspi_edma_m_handle, &xtfer);
	isTransferCompleted = false;
	if (kStatus_Success != ret )
		while(1);//Assert on Error, wait for reset
		//PRINTF("There is error when start DSPI_MasterTransferEDMA \r\n ");
	//while(isTransferCompleted == false)
	//while(g_dspi_edma_m_handle.state==kDSPI_Busy);
}


//void SPI2_Write_DMA(uint8_t* data , unsigned len,unsigned cs)
//{
//
//	if (len == 0) return;           //no need to send anything
//	while(isTransfer2Completed == false);
//	dspi_master_edma_handle_t spi_handle = g_dspi2_edma_m_handle;
//	dspi_transfer_t xtfer = master2Xfer;
//	while(g_dspi2_edma_m_handle.state==kDSPI_Busy);
//	while(g_dspi_edma_m_handle.state==kDSPI_Busy);
//	message2Len = len;
//	message2Data = data;
//	int MAX_LEN = 500;
//
//	if(len>MAX_LEN)
//	{
//		len=MAX_LEN;
//		message2Len-=MAX_LEN;
//		message2Data+=MAX_LEN;
//	}
//	else
//	{
//		message2Len=0;
//	}
//	xtfer.txData = data;
//	xtfer.dataSize = len;
//	xtfer.rxData = NULL;
//	xtfer.configFlags = kDSPI_MasterCtar0 | (0 << DSPI_MASTER_PCS_SHIFT) | kDSPI_MasterPcsContinuous;
////    chipSelect=cs;
//    status_t ret;
//	ret = DSPI_MasterTransferEDMA(DSPI_MASTER2_BASEADDR, &g_dspi2_edma_m_handle, &xtfer);
//	isTransfer2Completed = false;
//	if (kStatus_Success != ret )
//		while(1);//Assert on Error, wait for reset
//		//PRINTF("There is error when start DSPI_MasterTransferEDMA \r\n ");
//	//while(isTransferCompleted == false)
//	//while(g_dspi_edma_m_handle.state==kDSPI_Busy);
//}

void SPI_Write_Blocking(uint8_t* data , unsigned len,unsigned cs)
{
	if (len == 0) return;           //no need to send anything
	messageLen = len;
	messageData = data;
	dspi_transfer_t xtfer = masterXfer;

	int MAX_LEN = 32000;

	if(len>MAX_LEN)
	{
		len=MAX_LEN;
		messageLen-=MAX_LEN;
		messageData+=MAX_LEN;
	}
	else
	{
		messageLen=0;
	}
	xtfer.txData = data;
	xtfer.dataSize = len;
	xtfer.rxData = NULL;
	xtfer.configFlags = kDSPI_MasterCtar0 | (cs << DSPI_MASTER_PCS_SHIFT) | kDSPI_MasterPcsContinuous;
	status_t ret;
	ret = DSPI_MasterTransferBlocking(DSPI_MASTER_BASEADDR, &xtfer);
	isTransferCompleted = true;
	if (kStatus_Success != ret )
		while(1); //Assert on error wait for reset
		//PRINTF("There is error when start DSPI_MasterTransferEDMA \r\n ");
}

