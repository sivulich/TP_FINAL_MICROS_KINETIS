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

#define KINETIS_DAC_INSTANCE       DAC0
#define PIT_DATA_HANDLER PIT0_IRQHandler
#define PIT_IRQ_ID PIT0_IRQn
/* Get source clock for PIT driver */
#define PIT_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)


static int init();
static int setupSignal(short* buf1,short* buf2,unsigned len,unsigned fre);
static int start();
/*If returns 1, you should fill buff1, if it returns 2 you should fill buf2, if it returns 0 everthing is OK!*/
static int status();
static int pause();
static int stop();

static void writeDac(int dacValue)
{
	DAC_SetBufferValue(KINETIS_DAC_INSTANCE, 0U, ((uint16_t)(dacValue + 32768))>>4);
}



static int setUp=0;
static short* buffs[2];
static volatile unsigned buffLen,freq,currPos,currBuff;
static volatile int informed=0,setDac=0;

void PIT_DATA_HANDLER(void)
{
    /* Clear interrupt flag.*/
    PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, PIT_TFLG_TIF_MASK);
    setDac=1;
}
static int update()
{
	if(setUp==1 && setDac==1)
	{
		if(currPos==buffLen)
		{
			currPos=0;
			currBuff=(currBuff+1)%2;
		}
		if(currPos==0 && informed == 1)
			informed=0;
		writeDac((int)buffs[currBuff][currPos++]);
	}
	return 0;
}

static int init()
{
	dac_config_t dacConfigStruct;
	DAC_GetDefaultConfig(&dacConfigStruct);
	DAC_Init(KINETIS_DAC_INSTANCE, &dacConfigStruct);
	DAC_SetBufferReadPointer(KINETIS_DAC_INSTANCE, 0U);

	pit_config_t pitConfig;
	PIT_GetDefaultConfig(&pitConfig);
	pitConfig.enableRunInDebug=0;
	/* Init pit module */
	PIT_Init(PIT, &pitConfig);

	return 0;
}


static int setupSignal(short* buf1,short* buf2,unsigned len,unsigned fre)
{
	if(setUp==0)
	{
		buffs[0]=buf1;
		buffs[1]=buf2;
		buffLen=len;
		freq=fre;
		//SETUP PIT
		setUp=1;
		currBuff=0;
		currPos=0;
		informed = 1;
		/* Set timer period for channel 0 */
		PIT_SetTimerPeriod(PIT, kPIT_Chnl_0, USEC_TO_COUNT(1000000U/fre, PIT_SOURCE_CLOCK));
	}
	else
		return -1;
}

static int start()
{
	if(setUp==1)
	{
		/* Enable timer interrupts for channel 0 */
		PIT_EnableInterrupts(PIT, kPIT_Chnl_0, kPIT_TimerInterruptEnable);

		/* Enable at the NVIC */
		EnableIRQ(PIT_IRQ_ID);

		PIT_StartTimer(PIT, kPIT_Chnl_0);
		DAC_Enable(KINETIS_DAC_INSTANCE, true);
		return 0;
	}
	else
		return -1;
}

static int pause()
{
	if(setUp==1)
	{
		PIT_StopTimer(PIT,kPIT_Chnl_0);
		/* Enable at the NVIC */
		DisableIRQ(PIT_IRQ_ID);
		/* Enable timer interrupts for channel 0 */
		PIT_DisableInterrupts(PIT, kPIT_Chnl_0, kPIT_TimerInterruptEnable);
		DAC_Enable(KINETIS_DAC_INSTANCE, false);
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
		if(currBuff==0)
			return 2;
		else
			return 1;
	}
	return 0;
}


SigGen_ SigGen={.init=init,.setupSignal=setupSignal,.pause=pause,.start=start,.status=status,.stop=stop,.update=update};
