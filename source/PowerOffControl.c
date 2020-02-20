/*
 * PowerOffControl.c
 *
 *  Created on: 5 mar. 2019
 *      Author: Maxi
 */
#include "PowerOffControl.h"
#include "fsl_pmc.h"
#include "fsl_smc.h"
#include "fsl_llwu.h"
#include "fsl_gpio.h"
#include "fsl_port.h"
#include "fsl_dac.h"
#include "fsl_sim.h"

#define SCREEN_GPIO 		GPIOD,6
//#define KEYPAD_GND_GPIO		GPIOE,25		//Esto es solo para la multiperforada
#define PWR_BUTTON			LLWU,4

//GPIOs para apagar VCC en modulos externos al MCU
//#define KEYPAD_POWER_GPIO	GPIOE,24		//esto cambio a GPIOA,6
#define ILI_CTRL			GPIOC,19
#define LED_CTRL			GPIOC,18
#define OUT_CTRL			GPIOA,10
//#define BT_CTRL				GPIOD,7

static void init()
{
	gpio_pin_config_t config = {
		 kGPIO_DigitalOutput,
		 0
	};
	config.pinDirection = kGPIO_DigitalOutput;
	config.outputLogic = 1;
	GPIO_PinInit(SCREEN_GPIO, &config);
	GPIO_PinWrite(SCREEN_GPIO,1);
	GPIO_PinInit(ILI_CTRL, &config);
	GPIO_PinWrite(ILI_CTRL,0);
	GPIO_PinInit(LED_CTRL, &config);
	GPIO_PinWrite(LED_CTRL,0);
	GPIO_PinInit(OUT_CTRL, &config);
	GPIO_PinWrite(OUT_CTRL,0);

}

static void recover()
{
//	SMC_SetPowerModeProtection(SMC, kSMC_AllowPowerModeVlls);
//	if(LLWU_GetExternalWakeupPinFlag(PWR_BUTTON))
//	{
//		LLWU_ClearExternalWakeupPinFlag(PWR_BUTTON);
//	}
	if(PMC_GetPeriphIOIsolationFlag(PMC))
	{
		PMC_ClearPeriphIOIsolationFlag(PMC);
	}
}

void BOARD_DeInitPins(void)
{
    /* PORTA0 (pin 50) is configured as JTAG_TCLK */
    PORT_SetPinMux(PORTA, 0U, kPORT_PinDisabledOrAnalog);
    /* PORTA10 (pin 62) is configured as PTA10 */
    PORT_SetPinMux(PORTA, 10U, kPORT_PinDisabledOrAnalog);
    /* PORTA11 (pin 63) is configured as I2C2_SDA */
    PORT_SetPinMux(PORTA, 11U, kPORT_PinDisabledOrAnalog);
    /* PORTA12 (pin 64) is configured as I2C2_SCL */
    PORT_SetPinMux(PORTA, 12U, kPORT_PinDisabledOrAnalog);
    /* PORTA13 (pin 65) is configured as PTA13, LLWU_P4 */
//    PORT_SetPinMux(PORTA, 13U, kPORT_PinDisabledOrAnalog);
    /* PORTA2 (pin 52) is configured as JTAG_TDO */
    PORT_SetPinMux(PORTA, 2U, kPORT_PinDisabledOrAnalog);
    /* PORTA3 (pin 53) is configured as JTAG_TMS */
    PORT_SetPinMux(PORTA, 3U, kPORT_PinDisabledOrAnalog);
    PORT_SetPinMux(PORTA, 4U, kPORT_PinDisabledOrAnalog);
    /* PORTA8 (pin 60) is configured as FTM1_QD_PHA */
    PORT_SetPinMux(PORTA, 8U, kPORT_PinDisabledOrAnalog);
    /* PORTA9 (pin 61) is configured as FTM1_QD_PHB */
    PORT_SetPinMux(PORTA, 9U, kPORT_PinDisabledOrAnalog);
    /* PORTB18 (pin 97) is configured as FTM2_QD_PHA */
    PORT_SetPinMux(PORTB, 18U, kPORT_PinDisabledOrAnalog);
    /* PORTB19 (pin 98) is configured as FTM2_QD_PHB */
    PORT_SetPinMux(PORTB, 19U, kPORT_PinDisabledOrAnalog);
    /* PORTB20 (pin 99) is configured as PTB20 */
    PORT_SetPinMux(PORTB, 20U, kPORT_PinDisabledOrAnalog);
    /* PORTB22 (pin 101) is configured as SPI2_SOUT */
    PORT_SetPinMux(PORTB, 22U, kPORT_PinDisabledOrAnalog);
    /* PORTB4 (pin 85) is configured as PTB4 */
    PORT_SetPinMux(PORTB, 4U, kPORT_PinDisabledOrAnalog);
    /* PORTB5 (pin 86) is configured as PTB5 */
    PORT_SetPinMux(PORTB, 5U, kPORT_PinDisabledOrAnalog);
    /* PORTB6 (pin 87) is configured as PTB6 */
    PORT_SetPinMux(PORTB, 6U, kPORT_PinDisabledOrAnalog);
    /* PORTB9 (pin 90) is configured as PTB9 */
    PORT_SetPinMux(PORTB, 9U, kPORT_PinDisabledOrAnalog);
    /* PORTC0 (pin 103) is configured as PTC0 */
    PORT_SetPinMux(PORTC, 0U, kPORT_PinDisabledOrAnalog);
    /* PORTC1 (pin 104) is configured as PTC1 */
    PORT_SetPinMux(PORTC, 1U, kPORT_PinDisabledOrAnalog);
    /* PORTC11 (pin 116) is configured as I2S0_RXD1 */
    PORT_SetPinMux(PORTC, 11U, kPORT_PinDisabledOrAnalog);
    /* PORTC12 (pin 117) is configured as PTC12 */
    PORT_SetPinMux(PORTC, 12U, kPORT_PinDisabledOrAnalog);
    /* PORTC13 (pin 118) is configured as PTC13 */
    PORT_SetPinMux(PORTC, 13U, kPORT_PinDisabledOrAnalog);
    /* PORTC14 (pin 119) is configured as PTC14 */
    PORT_SetPinMux(PORTC, 14U, kPORT_PinDisabledOrAnalog);
    /* PORTC15 (pin 120) is configured as PTC15 */
    PORT_SetPinMux(PORTC, 15U, kPORT_PinDisabledOrAnalog);
    /* PORTC18 (pin 125) is configured as PTC18 */
    PORT_SetPinMux(PORTC, 18U, kPORT_PinDisabledOrAnalog);
    /* PORTC19 (pin 126) is configured as PTC19 */
    PORT_SetPinMux(PORTC, 19U, kPORT_PinDisabledOrAnalog);
    /* PORTC2 (pin 105) is configured as PTC2 */
    PORT_SetPinMux(PORTC, 2U, kPORT_PinDisabledOrAnalog);
    /* PORTC3 (pin 106) is configured as PTC3 */
    PORT_SetPinMux(PORTC, 3U, kPORT_PinDisabledOrAnalog);
    /* PORTC5 (pin 110) is configured as I2S0_RXD0 */
    PORT_SetPinMux(PORTC, 5U, kPORT_PinDisabledOrAnalog);
    /* PORTC6 (pin 111) is configured as I2S0_MCLK */
    PORT_SetPinMux(PORTC, 6U, kPORT_PinDisabledOrAnalog);
    /* PORTC7 (pin 112) is configured as I2S0_RX_FS */
    PORT_SetPinMux(PORTC, 7U, kPORT_PinDisabledOrAnalog);
    /* PORTD0 (pin 127) is configured as SPI0_PCS0 */
    PORT_SetPinMux(PORTD, 0U, kPORT_PinDisabledOrAnalog);
    /* PORTD1 (pin 128) is configured as SPI0_SCK */
    PORT_SetPinMux(PORTD, 1U, kPORT_PinDisabledOrAnalog);
    /* PORTD2 (pin 129) is configured as SPI0_SOUT */
    PORT_SetPinMux(PORTD, 2U, kPORT_PinDisabledOrAnalog);
    /* PORTD3 (pin 130) is configured as SPI0_SIN */
    PORT_SetPinMux(PORTD, 3U, kPORT_PinDisabledOrAnalog);
    /* PORTD4 (pin 131) is configured as PTD4 */
    PORT_SetPinMux(PORTD, 4U, kPORT_PinDisabledOrAnalog);
    /* PORTD5 (pin 132) is configured as PTD5 */
    PORT_SetPinMux(PORTD, 5U, kPORT_PinDisabledOrAnalog);
    /* PORTD6 (pin 133) is configured as PTD6 */
    PORT_SetPinMux(PORTD, 6U, kPORT_PinDisabledOrAnalog);
    /* PORTE0 (pin 1) is configured as SDHC0_D1 */
    PORT_SetPinMux(PORTE, 0U, kPORT_PinDisabledOrAnalog);
    /* PORTE1 (pin 2) is configured as SDHC0_D0 */
    PORT_SetPinMux(PORTE, 1U, kPORT_PinDisabledOrAnalog);
    /* PORTE2 (pin 3) is configured as SDHC0_DCLK */
    PORT_SetPinMux(PORTE, 2U, kPORT_PinDisabledOrAnalog);
    /* PORTE24 (pin 45) is configured as PTE24 */
    PORT_SetPinMux(PORTE, 24U, kPORT_PinDisabledOrAnalog);
    /* PORTE25 (pin 46) is configured as PTE25 */
    PORT_SetPinMux(PORTE, 25U, kPORT_PinDisabledOrAnalog);
    /* PORTE3 (pin 4) is configured as SDHC0_CMD */
    PORT_SetPinMux(PORTE, 3U, kPORT_PinDisabledOrAnalog);
    PORT_SetPinMux(PORTE, 4U, kPORT_PinDisabledOrAnalog);
    /* PORTE5 (pin 8) is configured as SDHC0_D2 */
    PORT_SetPinMux(PORTE, 5U, kPORT_PinDisabledOrAnalog);
    /* PORTE6 (pin 9) is configured as PTE6 */
    PORT_SetPinMux(PORTE, 6U, kPORT_PinDisabledOrAnalog);
}

static void powerOff()
{
	//recover();
	SIM_SetUsbVoltRegulatorEnableMode(kSIM_UsbVoltRegEnableInAllModes);
	GPIO_PinWrite(SCREEN_GPIO,0);
	GPIO_PinWrite(ILI_CTRL,1);
	GPIO_PinWrite(LED_CTRL,1);
	GPIO_PinWrite(OUT_CTRL,1);

	BOARD_DeInitPins();
	DAC_Deinit(DAC0);
	DAC_Deinit(DAC1);
	LLWU_ClearExternalWakeupPinFlag(PWR_BUTTON);
	LLWU_SetExternalWakeupPinMode(PWR_BUTTON,kLLWU_ExternalPinFallingEdge);
	//llwu_external_pin_filter_mode_t filterConfig;
	//filterConfig.pinIndex=9;
	//filterConfig.filterMode=kLLWU_PinFilterFallingEdge;
	//LLWU_SetPinFilterMode(LLWU,20,filterConfig);
	SMC_SetPowerModeProtection(SMC, kSMC_AllowPowerModeVlls);
	smc_power_mode_vlls_config_t config;
//	config.enablePorDetectInVlls0=false;
	config.subMode = kSMC_StopSub0;
	SMC_PreEnterStopModes();
	SMC_SetPowerModeVlls(SMC,&config);
	SMC_PostExitStopModes();
	//PRINTF("Si estas leyendo esto no me dormi un carajo. =)\n");
}

POWEROFF_ POWEROFF = { .recover = recover, .powerOff = powerOff, .init=init};

