/*
 * SigGen.c
 *
 *  Created on: Feb 9, 2019
 *      Author: Santi
 */

#include "SigGen.h"
#include "board.h"
#include "fsl_pit.h"
#include "fsl_dac.h"
#include "fsl_edma.h"
#include "fsl_dmamux.h"
#include "fsl_sai.h"
#include "fsl_sai_edma.h"

#define K64_DAC0			DAC1
#define PIT_L_DATA_HANDLER 	PIT3_IRQHandler
#define PIT_L_IRQ_ID 		PIT3_IRQn
#define PIT_L_CHANNEL 		kPIT_Chnl_3
#define CHANNEL_L_DMA 		3
#define PIT_L_DMASOURCE		kDmaRequestMux0AlwaysOn63

#define K64_DAC1			DAC0
#define PIT_R_DATA_HANDLER 	PIT2_IRQHandler
#define PIT_R_IRQ_ID 		PIT2_IRQn
#define PIT_R_CHANNEL 		kPIT_Chnl_2
#define CHANNEL_R_DMA 		2
#define PIT_R_DMASOURCE		kDmaRequestMux0AlwaysOn62

/* Get source clock for PIT driver */
#define PIT_SOURCE_CLOCK 	CLOCK_GetFreq(kCLOCK_BusClk)
#define SIGEN_DMA 			DMA0
#define SIGEN_DMAMUX 		DMAMUX0

//SAI
#define K64_I2S				I2S0
#define I2S_DMA_SOURCE  	kDmaRequestMux0I2S0Tx
#define I2S_CHANNEL_DMA		8


static int init();
static int setupSignal(unsigned short** buL,unsigned short** buR,int qnt,unsigned len,unsigned fre);
static int start();
/*If returns 1, you should fill buff1, if it returns 2 you should fill buf2, if it returns 0 everthing is OK!*/
static int status();
static int pause();
static int stop();


static int setUp=0,pauseState=0,buffQnt;
static unsigned short** buffsL, **buffsR;
static volatile unsigned buffLen,freq,currPos,currBuffL, currBuffR;
static volatile int informed=0,setDac=0;

void PIT_L_DATA_HANDLER(void)
{
    /* Clear interrupt flag.*/
    PIT_ClearStatusFlags(PIT, PIT_L_CHANNEL, PIT_TFLG_TIF_MASK);
    //setDac=1;
}

void PIT_R_DATA_HANDLER(void)
{
    /* Clear interrupt flag.*/
    PIT_ClearStatusFlags(PIT, PIT_R_CHANNEL, PIT_TFLG_TIF_MASK);
    //setDac=1;
}


static edma_handle_t g_EDMA_HandleL;
static edma_transfer_config_t transferLConfig;

static edma_handle_t g_EDMA_HandleR;
static edma_transfer_config_t transferRConfig;

static edma_config_t userConfig;

static volatile bool g_Transfer_L_Done = false;
static volatile bool g_Transfer_R_Done = false;

//SAI
sai_edma_handle_t *saiEDMAHandle;
static edma_handle_t g_EDMA_HandleSAI;
sai_transfer_t sai_transfer;
uint32_t dataSAI_L;
uint32_t dataSAI_R;

/* User callback function for EDMA transfer. */
void EDMA_CallbackL(edma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    if (transferDone)
    {
    	g_Transfer_L_Done = true;
        if(pauseState==0)
        {
        	currBuffL=(currBuffL+1)%buffQnt;
        	informed=0;
        	/*for(int i = 0; i < 1152; i++)
			{
				monoBuffer[i] = buffsL[currBuff][i]/2 + buffsR[currBuff][i]/2;
			}*/
			EDMA_PrepareTransfer(&transferLConfig, buffsL[currBuffL], 2, K64_DAC0->DAT, 2,
					2, buffLen*2, kEDMA_MemoryToPeripheral);
			EDMA_SubmitTransfer(&g_EDMA_HandleL, &transferLConfig);
			EDMA_StartTransfer(&g_EDMA_HandleL);
        }
    }
}

void EDMA_CallbackR(edma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    if (transferDone)
    {
    	g_Transfer_R_Done = true;
        if(pauseState==0)
        {
        	currBuffR=(currBuffR+1)%buffQnt;
        	informed=0;
        	/*for(int i = 0; i < 1152; i++)
			{
				monoBuffer[i] = buffsL[currBuff][i]/2 + buffsR[currBuff][i]/2;
			}*/
			EDMA_PrepareTransfer(&transferRConfig, buffsR[currBuffR], 2, K64_DAC1->DAT, 2,
					2, buffLen*2, kEDMA_MemoryToPeripheral);
			EDMA_SubmitTransfer(&g_EDMA_HandleR, &transferRConfig);
			EDMA_StartTransfer(&g_EDMA_HandleR);
        }
    }
}

void EDMA_I2S_Callback(I2S_Type *base, sai_edma_handle_t *handle, status_t status, void *userData){

}

static int init()
{
	dac_config_t dacConfigStruct;
	DAC_GetDefaultConfig(&dacConfigStruct);
	//dacConfigStruct.referenceVoltageSource=kDAC_ReferenceVoltageSourceVref1;
	DAC_Init(K64_DAC0, &dacConfigStruct);
	DAC_SetBufferReadPointer(K64_DAC0, 0U);

	DAC_Init(K64_DAC1, &dacConfigStruct);		//para el mcu de 144
	DAC_SetBufferReadPointer(K64_DAC1, 0U);	//para el mcu de 144

	sai_config_t saiConfig;
	saiConfig.masterSlave = kSAI_Slave;
	saiConfig.bclkSource = kSAI_BclkSourceBusclk;
	saiConfig.mclkOutputEnable = false;
	SAI_TxGetDefaultConfig(&saiConfig);
	SAI_TxInit(K64_I2S, &saiConfig);
	SAI_TxSetBitClockPolarity(K64_I2S, kSAI_PolarityActiveLow);
	SAI_TxSetDataOrder(K64_I2S, kSAI_DataMSB);

	// This has to change when setUp signal
	sai_transfer_format_t format;
	format.channel = kSAI_Channel0Mask | kSAI_Channel1Mask;
	format.stereo = kSAI_Stereo;
	format.sampleRate_Hz = 0U; //comes from MP3
	format.bitWidth = kSAI_WordWidth24bits; //kSAI_WordWidth32bits
	uint32_t mclk = 0U;//TBD
	uint32_t bclk = 0U;//TBD
	SAI_TxSetFormat(K64_I2S, &format, mclk, bclk);
	dataSAI_L = SAI_TxGetDataRegisterAddress(K64_I2S, 0);
	dataSAI_R = SAI_TxGetDataRegisterAddress(K64_I2S, 1);

	pit_config_t pitConfig;
	PIT_GetDefaultConfig(&pitConfig);
	pitConfig.enableRunInDebug=0;
	/* Init pit module */
	PIT_Init(PIT, &pitConfig);

	DMAMUX_Init(SIGEN_DMAMUX);
	DMAMUX_DisableChannel(SIGEN_DMAMUX, CHANNEL_L_DMA);
	DMAMUX_SetSource(SIGEN_DMAMUX, CHANNEL_L_DMA, PIT_L_DMASOURCE);
	DMAMUX_EnablePeriodTrigger(SIGEN_DMAMUX,CHANNEL_L_DMA);

	DMAMUX_DisableChannel(SIGEN_DMAMUX, CHANNEL_R_DMA);
	DMAMUX_SetSource(SIGEN_DMAMUX, CHANNEL_R_DMA, PIT_R_DMASOURCE);
	DMAMUX_EnablePeriodTrigger(SIGEN_DMAMUX,CHANNEL_R_DMA);

	//SAI
	DMAMUX_DisableChannel(SIGEN_DMAMUX, I2S_CHANNEL_DMA);
	DMAMUX_SetSource(SIGEN_DMAMUX, I2S_CHANNEL_DMA, I2S_DMA_SOURCE);
	DMAMUX_EnableChannel(SIGEN_DMAMUX, I2S_CHANNEL_DMA);

	//DMAMUX_EnableChannel(SIGEN_DMAMUX, CHANNEL_DMA);
	/* Configure EDMA one shot transfer */
	/*
	 * userConfig.enableRoundRobinArbitration = false;
	 * userConfig.enableHaltOnError = true;
	 * userConfig.enableContinuousLinkMode = false;
	 * userConfig.enableDebugMode = false;
	 */
	EDMA_GetDefaultConfig(&userConfig);
	userConfig.enableRoundRobinArbitration=true;
	userConfig.enableDebugMode = false;
	EDMA_Init(SIGEN_DMA, &userConfig);

	EDMA_CreateHandle(&g_EDMA_HandleL, SIGEN_DMA, CHANNEL_L_DMA);
	EDMA_SetCallback(&g_EDMA_HandleL, EDMA_CallbackL, NULL);

	EDMA_CreateHandle(&g_EDMA_HandleR, SIGEN_DMA, CHANNEL_R_DMA);
	EDMA_SetCallback(&g_EDMA_HandleR, EDMA_CallbackR, NULL);

	EDMA_CreateHandle(&g_EDMA_HandleSAI, SIGEN_DMA, I2S_CHANNEL_DMA);
	SAI_TransferTxCreateHandleEDMA(K64_I2S, saiEDMAHandle, EDMA_I2S_Callback, NULL, &g_EDMA_HandleSAI);
	/*edma_channel_Preemption_config_t priorityConfig;
	priorityConfig.channelPriority=0;
	priorityConfig.enableChannelPreemption=0;
	priorityConfig.enablePreemptAbility=1;
	EDMA_SetChannelPreemptionConfig(SIGEN_DMA,CHANNEL_DMA,&priorityConfig);*/

	DAC_Enable(K64_DAC0, true);
	DAC_Enable(K64_DAC1, true);	//para el mcu de 144

	SAI_TxEnable(I2S0, true);
	return 0;
}


static int setupSignal(unsigned short** buL, unsigned short** buR, int qnt,unsigned len,unsigned fre)
{
	if(setUp==0)
	{
		buffsL=buL;
		buffsR=buR;
		buffLen=len;
		buffQnt=qnt;
		freq=fre;
		//SETUP PIT
		setUp=1;
		currBuffL=0;
		currBuffR=0;
		currPos=0;
		informed = 1;
		pauseState=1;
		/* Set timer period for channel 0 */
		PIT_SetTimerPeriod(PIT, PIT_L_CHANNEL, USEC_TO_COUNT(1000000U/fre, PIT_SOURCE_CLOCK)+1);
		PIT_SetTimerPeriod(PIT, PIT_R_CHANNEL, USEC_TO_COUNT(1000000U/fre, PIT_SOURCE_CLOCK)+1);

		/*for(int i = 0; i < 1152; i++)
		{
			monoBuffer[i] = buffsL[currBuff][i]/2 + buffsR[currBuff][i]/2;
		}*/
		EDMA_PrepareTransfer(&transferLConfig, buffsL[currBuffL], 2, K64_DAC0->DAT, 2,
								 2, buffLen*2, kEDMA_MemoryToPeripheral);

		//para el mcu de 144
		EDMA_PrepareTransfer(&transferRConfig, buffsR[currBuffR], 2, K64_DAC1->DAT, 2,
										 2, buffLen*2, kEDMA_MemoryToPeripheral);

		EDMA_SubmitTransfer(&g_EDMA_HandleL, &transferLConfig);
		EDMA_SubmitTransfer(&g_EDMA_HandleR, &transferRConfig);
		return 0;
	}
	else
		return -1;
}

static int start()
{
	if(setUp==1 && pauseState==1)
	{
//		DAC_Enable(K64_DAC0, true);
//		DAC_Enable(K64_DAC1, true);

		/* Enable timer interrupts for channel 0 */
		PIT_EnableInterrupts(PIT, PIT_L_CHANNEL, kPIT_TimerInterruptEnable);
		PIT_EnableInterrupts(PIT, PIT_R_CHANNEL, kPIT_TimerInterruptEnable);

		/* Enable at the NVIC */
		EnableIRQ(PIT_L_IRQ_ID);
		EnableIRQ(PIT_R_IRQ_ID);
		PIT_StartTimer(PIT, PIT_L_CHANNEL);
		PIT_StartTimer(PIT, PIT_R_CHANNEL);

		//DAC_Enable(KINETIS_DAC_INSTANCE, true);
		EDMA_StartTransfer(&g_EDMA_HandleL);
		EDMA_StartTransfer(&g_EDMA_HandleR);
		DMAMUX_EnableChannel(SIGEN_DMAMUX, CHANNEL_L_DMA);
		DMAMUX_EnableChannel(SIGEN_DMAMUX, CHANNEL_R_DMA);
		pauseState=0;
		return 0;
	}
	else
		return -1;
}

static int pause()
{
	if(setUp==1 && pauseState==0)
	{
		DMAMUX_DisableChannel(SIGEN_DMAMUX, CHANNEL_L_DMA);
		DMAMUX_DisableChannel(SIGEN_DMAMUX, CHANNEL_R_DMA);
		PIT_StopTimer(PIT,PIT_L_CHANNEL);
		PIT_StopTimer(PIT,PIT_R_CHANNEL);
		/* Enable at the NVIC */
		DisableIRQ(PIT_L_IRQ_ID);
		DisableIRQ(PIT_R_IRQ_ID);
		/* Enable timer interrupts for channel 0 */
		PIT_DisableInterrupts(PIT, PIT_L_CHANNEL, kPIT_TimerInterruptEnable);
		PIT_DisableInterrupts(PIT, PIT_R_CHANNEL, kPIT_TimerInterruptEnable);

//		DAC_Enable(K64_DAC0, false);
//		DAC_Enable(K64_DAC1, false);
		pauseState=1;
		return 0;
	}
	else
		return -1;
}
static int stop()
{
	if(setUp==1)
	{
		pause();
		setUp=0;
	}
	return 0;
}

/*If returns 1, you should fill buff1, if it returns 2 you should fill buf2, if it returns 0 everthing is OK!*/

static int status()
{
	if(informed==0)
	{
		informed=1;
		int currBuff = ( (currBuffL > currBuffR) || (currBuffL == 0 && currBuffR == (buffQnt-1)) )? currBuffL : currBuffR;
		return ((currBuff-1+buffQnt)%buffQnt)+1;
	}
	return 0;
}


SigGen_ SigGen={.init=init,.setupSignal=setupSignal,.pause=pause,.start=start,.status=status,.stop=stop};
