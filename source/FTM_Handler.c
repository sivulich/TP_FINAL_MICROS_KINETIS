/*
 * FTM_Handler.c
 *
 *  Created on: 13 feb. 2020
 *      Author: Fabian
 */


#include "fsl_device_registers.h"
#include "board.h"
#include "fsl_dspi_edma.h"
#include "fsl_edma.h"
#include "fsl_dmamux.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "lvgl/lvgl.h"
#include "fsl_ftm.h"
#include "pin_mux.h"
#include "fsl_gpio.h"
#include "FTM_Handler.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define NEO_NOF_PIXEL       64
#define NEO_NOF_PIXEL_1		19
#define NEO_NOF_BITS_PIXEL 24

#define BOARD_OUT_GPIO_PIN 22U
#define BOARD_OUT_GPIO_PORT GPIOB
#define ONED 1u << BOARD_OUT_GPIO_PIN

#define BOARD_FIRST_FTM_CHANNEL kFTM_Chnl_0
#define BOARD_SECOND_FTM_CHANNEL kFTM_Chnl_1
#define BOARD_THIRD_FTM_CHANNEL kFTM_Chnl_2

/* The Flextimer instance/channel used for board */
#define BOARD_FTM_BASEADDR FTM0
#define BOARD_FTM_CHANNEL kFTM_Chnl_0

/* Interrupt number and interrupt handler for the FTM instance used */
#define FTM_INTERRUPT_NUMBER_0 FTM0_IRQn

#define FTM_PIN1_HANDLER FTM0_IRQHandler

/* Interrupt to enable and flag to read; depends on the FTM channel used */
#define FTM_CHANNEL_0_INTERRUPT_ENABLE kFTM_Chnl0InterruptEnable
#define FTM_CHANNEL_1_INTERRUPT_ENABLE kFTM_Chnl1InterruptEnable
#define FTM_CHANNEL_2_INTERRUPT_ENABLE kFTM_Chnl2InterruptEnable

#define FTM_CHANNEL0_FLAG kFTM_Chnl0Flag
#define FTM_CHANNEL1_FLAG kFTM_Chnl1Flag
#define FTM_CHANNEL2_FLAG kFTM_Chnl2Flag

/* Get source clock for FTM driver */
#define FTM_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_CoreSysClk)


// DMA0
#define FTM_DMA DMA0
#define FTM_DMAMUX DMAMUX0

#define GPIO_PTOR_REG(base)  ((base)->PTOR)
#define BUFF_LENGTH 4U


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* FTM user callback */
void FTM_EDMA_Callback(edma_handle_t *handle, void *param, bool transferDone, uint32_t tcds);

/*******************************************************************************
 * Variables
 ******************************************************************************/
edma_handle_t g_EDMA_Handle[3];
volatile bool g_Transfer_Done = false;
edma_transfer_config_t transferConfig[3];

//edma_handle_t g_EDMA_Handle_reset;
//volatile bool g_Transfer_Done_reset = false;
//edma_transfer_config_t transferConfig_reset;

edma_tcd_t *tcdRegs;
uint32_t port5 = 1u << BOARD_OUT_GPIO_PIN;

//AT_NONCACHEABLE_SECTION_INIT(uint32_t srcAddr[BUFF_LENGTH])  = {0x01, 0x02, 0x03, 0x04};
//AT_NONCACHEABLE_SECTION_INIT(uint32_t destAddr[10000]) = {0x00};


void FTM_EDMA_Callback(edma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    if (transferDone)
    {
//    	DMAMUX_Deinit(EXAMPLE_DMAMUX);
//    	EDMA_Deinit(EXAMPLE_DMA);
    	FTM_Stop();
    	GPIO_PinWrite(GPIOB, 22, 0);
        g_Transfer_Done = true;
    }
}


void FTM_Handler_Init(uint8_t* transmitBuf, uint8_t size)
{
	gpio_pin_config_t led_config = {
			kGPIO_DigitalOutput,
			0,
	};

	ftm_config_t ftmInfo;
	ftm_chnl_pwm_signal_param_t ftmParam[3];
	ftm_pwm_level_select_t pwmLevel = kFTM_HighTrue;


	ftmParam[0].chnlNumber            = (ftm_chnl_t)BOARD_FIRST_FTM_CHANNEL;
	ftmParam[0].level                 = pwmLevel;
	ftmParam[0].dutyCyclePercent      = 5U;
	ftmParam[0].firstEdgeDelayPercent = 0U;

	ftmParam[1].chnlNumber            = (ftm_chnl_t)BOARD_SECOND_FTM_CHANNEL;
	ftmParam[1].level                 = pwmLevel;
	ftmParam[1].dutyCyclePercent      = 40U;
	ftmParam[1].firstEdgeDelayPercent = 0U;

	ftmParam[2].chnlNumber            = (ftm_chnl_t)BOARD_THIRD_FTM_CHANNEL;
	ftmParam[2].level                 = pwmLevel;
	ftmParam[2].dutyCyclePercent      = 60U;
	ftmParam[2].firstEdgeDelayPercent = 0U;

	GPIO_PinInit(BOARD_OUT_GPIO_PORT, BOARD_OUT_GPIO_PIN, &led_config);

	GPIO_PortSet(BOARD_OUT_GPIO_PORT, 1u << BOARD_OUT_GPIO_PIN);
	/* Get default config of ftm_config_t*/
	FTM_GetDefaultConfig(&ftmInfo);

	/* Initialize FTM module */
	FTM_Init(BOARD_FTM_BASEADDR, &ftmInfo);

	FTM_SetupPwm(BOARD_FTM_BASEADDR, ftmParam, 3U, kFTM_EdgeAlignedPwm, 800000U, FTM_SOURCE_CLOCK); //240000U
	//FTM_SetupPwm(BOARD_FTM_BASEADDR_1, &ftmParam[1], 1U, kFTM_EdgeAlignedPwm, 24000U, FTM_SOURCE_CLOCK);

	/* Enable channel interrupt flag.*/
//	FTM_EnableInterrupts(BOARD_FTM_BASEADDR, FTM_CHANNEL_0_INTERRUPT_ENABLE);
//	FTM_EnableInterrupts(BOARD_FTM_BASEADDR, FTM_CHANNEL_1_INTERRUPT_ENABLE);
//	FTM_EnableInterrupts(BOARD_FTM_BASEADDR, FTM_CHANNEL_2_INTERRUPT_ENABLE);

//	FTM_EnableDmaTransfer(BOARD_FTM_BASEADDR, kFTM_Chnl_0, 1U);
	FTM_EnableDmaTransfer(BOARD_FTM_BASEADDR, kFTM_Chnl_1, 1U);
//	FTM_EnableDmaTransfer(BOARD_FTM_BASEADDR, kFTM_Chnl_2, 1U);

	/* Enable at the NVIC */
//	EnableIRQ(FTM_INTERRUPT_NUMBER_0);


	//DMAMUX Initialize
	DMAMUX_Init(FTM_DMAMUX);
	//DMA SOURCE 0
//	DMAMUX_SetSource(FTM_DMAMUX, 10, kDmaRequestMux0FTM0Channel0);
//	DMAMUX_EnableChannel(FTM_DMAMUX, 10);
	//DMA SOURCE 1
	DMAMUX_SetSource(FTM_DMAMUX, 11, kDmaRequestMux0FTM0Channel1);
	DMAMUX_EnableChannel(FTM_DMAMUX, 11);
	//DMA SOURCE 2
//	DMAMUX_SetSource(FTM_DMAMUX, 12, kDmaRequestMux0FTM0Channel2);
//	DMAMUX_EnableChannel(FTM_DMAMUX, 12);

	//DMA Initialize
	edma_config_t userConfig;
	EDMA_GetDefaultConfig(&userConfig);
	userConfig.enableRoundRobinArbitration=true;
	userConfig.enableDebugMode = false;
	EDMA_Init(FTM_DMA, &userConfig);
	//Chanel 0
//	EDMA_CreateHandle(&g_EDMA_Handle[0], FTM_DMA, 10);
	//Chanel 1
	EDMA_CreateHandle(&g_EDMA_Handle[1], FTM_DMA, 11);
	//Chanel 2
//	EDMA_CreateHandle(&g_EDMA_Handle[2], FTM_DMA, 12);


	EDMA_SetCallback(&g_EDMA_Handle[2], FTM_EDMA_Callback, NULL);

	uint8_t* auxPDOR = (uint8_t*)&GPIOB->PDOR;
	uint8_t* auxPSOR = (uint8_t*)&GPIOB->PSOR;
	uint8_t* auxPCOR = (uint8_t*)&GPIOB->PCOR;
	//Chanel 0
//	EDMA_PrepareTransfer(&transferConfig[0], &port5, 1U, (uint32_t *)(auxPSOR), 1U,
//						   1U, size, kEDMA_PeripheralToMemory);
	//Chanel 1
	EDMA_PrepareTransfer(&transferConfig[1], (uint32_t *)transmitBuf, 1U, (uint32_t *)(auxPDOR+2), 1U,
								1U, size, kEDMA_MemoryToPeripheral);
	//Chanel 2
//	EDMA_PrepareTransfer(&transferConfig[2], &port5, 4U, (uint32_t *)(auxPCOR), 4U,
//							   4U, 4*size, kEDMA_PeripheralToMemory);
	//Chanel 0
//	EDMA_SubmitTransfer(&g_EDMA_Handle[0], &transferConfig[0]);
	//Chanel 1
	EDMA_SubmitTransfer(&g_EDMA_Handle[1], &transferConfig[1]);
	//Chanel 2
//	EDMA_SubmitTransfer(&g_EDMA_Handle[2], &transferConfig[2]);

//	tcdRegs            = (edma_tcd_t *)(uint32_t)&g_EDMA_Handle[0].base->TCD[g_EDMA_Handle[0].channel];
//	EDMA_TcdDisableInterrupts(tcdRegs, kEDMA_MajorInterruptEnable);
	tcdRegs            = (edma_tcd_t *)(uint32_t)&g_EDMA_Handle[1].base->TCD[g_EDMA_Handle[1].channel];
	EDMA_TcdDisableInterrupts(tcdRegs, kEDMA_MajorInterruptEnable);

//	EDMA_StartTransfer(&g_EDMA_Handle[0]);
//	EDMA_StartTransfer(&g_EDMA_Handle[1]);
//	EDMA_StartTransfer(&g_EDMA_Handle[2]);

}

void FTM_PIN1_HANDLER(void)
{

	GPIO_PortToggle(BOARD_OUT_GPIO_PORT, 1u << BOARD_OUT_GPIO_PIN);

	//BOARD_OUT_GPIO_PORT->PTOR =1u << BOARD_OUT_GPIO_PIN;
//    if ((FTM_GetStatusFlags(BOARD_FTM_BASEADDR) & FTM_CHANNEL0_FLAG) == FTM_CHANNEL0_FLAG)
//    {
//        /* Clear interrupt flag.*/
//        FTM_ClearStatusFlags(BOARD_FTM_BASEADDR, FTM_CHANNEL0_FLAG);
//    }

    if ((FTM_GetStatusFlags(BOARD_FTM_BASEADDR) & FTM_CHANNEL1_FLAG) == FTM_CHANNEL1_FLAG)
      {
          /* Clear interrupt flag.*/
          FTM_ClearStatusFlags(BOARD_FTM_BASEADDR, FTM_CHANNEL1_FLAG);
    }
//    if ((FTM_GetStatusFlags(BOARD_FTM_BASEADDR) & FTM_CHANNEL2_FLAG) == FTM_CHANNEL2_FLAG)
//      {
//          /* Clear interrupt flag.*/
//          FTM_ClearStatusFlags(BOARD_FTM_BASEADDR, FTM_CHANNEL2_FLAG);
//      }
    __DSB();
}

void FTM_Start(){
	/* Enable channel interrupt flag.*/
//	FTM_EnableInterrupts(BOARD_FTM_BASEADDR, FTM_CHANNEL_0_INTERRUPT_ENABLE);
	FTM_EnableInterrupts(BOARD_FTM_BASEADDR, FTM_CHANNEL_1_INTERRUPT_ENABLE);
//	FTM_EnableInterrupts(BOARD_FTM_BASEADDR, FTM_CHANNEL_2_INTERRUPT_ENABLE);


	/* Enable at the NVIC */
	EnableIRQ(FTM_INTERRUPT_NUMBER_0);

	FTM_StartTimer(BOARD_FTM_BASEADDR, kFTM_SystemClock);
	EDMA_StartTransfer(&g_EDMA_Handle[1]);
	}

void FTM_Stop(){
	FTM_StopTimer(BOARD_FTM_BASEADDR);
}
