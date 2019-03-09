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
#include "fsl_debug_console.h"	//Just for debug
#include "fsl_gpio.h"

static void recover()
{
	if(PMC_GetPeriphIOIsolationFlag(PMC))
	{
		PMC_ClearPeriphIOIsolationFlag(PMC);
	}
}

static void powerOff()
{
	LLWU_ClearExternalWakeupPinFlag(LLWU,9);
	LLWU_SetExternalWakeupPinMode(LLWU, 9,  kLLWU_ExternalPinAnyEdge);
	//llwu_external_pin_filter_mode_t filterConfig;
	//filterConfig.pinIndex=9;
	//filterConfig.filterMode=kLLWU_PinFilterFallingEdge;
	//LLWU_SetPinFilterMode(LLWU,20,filterConfig);
	SMC_SetPowerModeProtection(SMC, kSMC_AllowPowerModeVlls);
	smc_power_mode_vlls_config_t config;
	config.enablePorDetectInVlls0=true;
	config.subMode = kSMC_StopSub0;
	SMC_PreEnterStopModes();
	SMC_SetPowerModeVlls(SMC,&config);
	SMC_PostExitStopModes();
	PRINTF("Si estas leyendo esto no me dormi un carajo. =)\n");
}

POWEROFF_ POWEROFF = { .recover = recover, .powerOff = powerOff};

