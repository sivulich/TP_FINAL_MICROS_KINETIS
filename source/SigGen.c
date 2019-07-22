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

#define KINETIS_DAC_INSTANCE       DAC0
#define PIT_DATA_HANDLER PIT3_IRQHandler
#define PIT_IRQ_ID PIT3_IRQn
#define PIT_CHANNEL kPIT_Chnl_3
/* Get source clock for PIT driver */
#define PIT_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)
#define EXAMPLE_DMA DMA0
#define CHANNEL_DMA 3
#define EXAMPLE_DMAMUX DMAMUX0



static int init();
static int setupSignal(unsigned short** buL,unsigned short** buR,int qnt,unsigned len,unsigned fre);
static int start();
/*If returns 1, you should fill buff1, if it returns 2 you should fill buf2, if it returns 0 everthing is OK!*/
static int status();
static int pause();
static int stop();


static int setUp=0,pauseState=0,buffQnt;
static unsigned short** buffsL, **buffsR;
static volatile unsigned buffLen,freq,currPos,currBuff;
static volatile int informed=0,setDac=0;

void PIT_DATA_HANDLER(void)
{
    /* Clear interrupt flag.*/
    PIT_ClearStatusFlags(PIT, PIT_CHANNEL, PIT_TFLG_TIF_MASK);
    //setDac=1;
}


static edma_handle_t g_EDMA_Handle;
static edma_transfer_config_t transferConfig;
static edma_config_t userConfig;
static volatile bool g_Transfer_Done = false;
/* User callback function for EDMA transfer. */
void EDMA_Callback(edma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    if (transferDone)
    {
        g_Transfer_Done = true;
        if(pauseState==0)
        {
        	currBuff=(currBuff+1)%buffQnt;
        	informed=0;
        	/*for(int i = 0; i < 1152; i++)
			{
				monoBuffer[i] = buffsL[currBuff][i]/2 + buffsR[currBuff][i]/2;
			}*/
        	EDMA_PrepareTransfer(&transferConfig, buffsL[currBuff], 2, DAC0->DAT, 2,
        									 2, buffLen*2, kEDMA_MemoryToPeripheral);
        	EDMA_SubmitTransfer(&g_EDMA_Handle, &transferConfig);
        	EDMA_StartTransfer(&g_EDMA_Handle);

        }
    }
}

static int init()
{
	dac_config_t dacConfigStruct;
	DAC_GetDefaultConfig(&dacConfigStruct);
	//dacConfigStruct.referenceVoltageSource=kDAC_ReferenceVoltageSourceVref1;
	DAC_Init(KINETIS_DAC_INSTANCE, &dacConfigStruct);
	DAC_SetBufferReadPointer(KINETIS_DAC_INSTANCE, 0U);

	pit_config_t pitConfig;
	PIT_GetDefaultConfig(&pitConfig);
	pitConfig.enableRunInDebug=0;
	/* Init pit module */
	PIT_Init(PIT, &pitConfig);

	DMAMUX_Init(EXAMPLE_DMAMUX);
	DMAMUX_DisableChannel(DMAMUX0, CHANNEL_DMA);
	DMAMUX_SetSource(EXAMPLE_DMAMUX, CHANNEL_DMA,kDmaRequestMux0AlwaysOn63);
	DMAMUX_EnablePeriodTrigger(EXAMPLE_DMAMUX,CHANNEL_DMA);
	//DMAMUX_EnableChannel(EXAMPLE_DMAMUX, CHANNEL_DMA);
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
	EDMA_Init(EXAMPLE_DMA, &userConfig);
	EDMA_CreateHandle(&g_EDMA_Handle, EXAMPLE_DMA, CHANNEL_DMA);
	EDMA_SetCallback(&g_EDMA_Handle, EDMA_Callback, NULL);
	/*edma_channel_Preemption_config_t priorityConfig;
	priorityConfig.channelPriority=0;
	priorityConfig.enableChannelPreemption=0;
	priorityConfig.enablePreemptAbility=1;
	EDMA_SetChannelPreemptionConfig(EXAMPLE_DMA,CHANNEL_DMA,&priorityConfig);*/
	DAC_Enable(KINETIS_DAC_INSTANCE, true);
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
		currBuff=0;
		currPos=0;
		informed = 1;
		pauseState=1;
		/* Set timer period for channel 0 */
		PIT_SetTimerPeriod(PIT, PIT_CHANNEL, USEC_TO_COUNT(1000000U/fre, PIT_SOURCE_CLOCK)+1);

		/*for(int i = 0; i < 1152; i++)
		{
			monoBuffer[i] = buffsL[currBuff][i]/2 + buffsR[currBuff][i]/2;
		}*/
		EDMA_PrepareTransfer(&transferConfig, buffsL[currBuff], 2, DAC0->DAT, 2,
								 2, buffLen*2, kEDMA_MemoryToPeripheral);
		EDMA_SubmitTransfer(&g_EDMA_Handle, &transferConfig);
		return 0;
	}
	else
		return -1;
}

static int start()
{
	if(setUp==1 && pauseState==1)
	{
		/* Enable timer interrupts for channel 0 */
		PIT_EnableInterrupts(PIT, PIT_CHANNEL, kPIT_TimerInterruptEnable);

		/* Enable at the NVIC */
		EnableIRQ(PIT_IRQ_ID);
		PIT_StartTimer(PIT, PIT_CHANNEL);
		//DAC_Enable(KINETIS_DAC_INSTANCE, true);
		EDMA_StartTransfer(&g_EDMA_Handle);
		DMAMUX_EnableChannel(EXAMPLE_DMAMUX, CHANNEL_DMA);
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
		DMAMUX_DisableChannel(EXAMPLE_DMAMUX, CHANNEL_DMA);
		PIT_StopTimer(PIT,PIT_CHANNEL);
		/* Enable at the NVIC */
		DisableIRQ(PIT_IRQ_ID);
		/* Enable timer interrupts for channel 0 */
		PIT_DisableInterrupts(PIT, PIT_CHANNEL, kPIT_TimerInterruptEnable);
		//DAC_Enable(KINETIS_DAC_INSTANCE, false);
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
		return ((currBuff-1+buffQnt)%buffQnt)+1;
	}
	return 0;
}


SigGen_ SigGen={.init=init,.setupSignal=setupSignal,.pause=pause,.start=start,.status=status,.stop=stop};
