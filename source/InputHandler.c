#include "InputHandler.h"
#include "fsl_gpio.h"
#include "fsl_ftm.h"
#include "fsl_smc.h"
#include "fsl_llwu.h"
#include "fsl_debug_console.h"
#include "../lv_conf.h"
#include <math.h>
#include "MP3Ui.h"
#include "MP3PlayerData.h"

#define FORWARD_GPIO GPIOC,5
#define BACKWARD_GPIO GPIOC,0
#define PLAY_GPIO GPIOC,7
#define A_GPIO GPIOC,1
#define ENTER_GPIO GPIOC,8
#define B_GPIO GPIOC,9
#define SCREEN_GPIO GPIOB,9


static lv_indev_state_t state;
void InputHandlerInit()
{
	//Initialize non-SPI GPIOs
	gpio_pin_config_t config = {
	     kGPIO_DigitalInput,
	 	 0
	};

	ftm_config_t configFTM;
	FTM_GetDefaultConfig(&configFTM);
	FTM_Init(FTM2, &configFTM);

	ftm_phase_params_t paramsPhase;
	paramsPhase.enablePhaseFilter = 1;
	paramsPhase.phaseFilterVal = 4;
	paramsPhase.phasePolarity = kFTM_QuadPhaseInvert;

	ftm_quad_decode_mode_t quadMode = kFTM_QuadPhaseEncode;

	//FTM2->MODE |= FTM_MODE_WPDIS_MASK;
	uint32_t startVal = 0;
	uint32_t overVal = 255;
	FTM_SetQuadDecoderModuloValue(FTM2, startVal, overVal);
	FTM_ClearQuadDecoderCounterValue(FTM2);

	FTM_SetupQuadDecode(FTM2,&paramsPhase,&paramsPhase,quadMode);

	GPIO_PinInit(GPIOA,4, &config);
	GPIO_PinInit(GPIOC,6, &config);

	//GPIO_PinInit(FORWARD_GPIO, &config);		//fordward
	GPIO_PinInit(BACKWARD_GPIO, &config);		//backward
	GPIO_PinInit(PLAY_GPIO, &config);		//play/pause
	GPIO_PinInit(A_GPIO, &config);		//A
	GPIO_PinInit(ENTER_GPIO, &config);     //Enter
	GPIO_PinInit(B_GPIO, &config);		//B

	config.pinDirection = kGPIO_DigitalOutput;
	config.outputLogic = 1;
	GPIO_PinInit(SCREEN_GPIO, &config);
	GPIO_PinWrite(SCREEN_GPIO,1);
}
static int lastEnc=0b11,newEnc,storeEnc=0b11,cnt=0;
static unsigned long long pwrDownCnt=0;
static lv_indev_state_t encKey=LV_GROUP_KEY_ESC;
static int lastEncCnt=0;
#define ENC_RIGHT 0b010010
#define ENC_LEFT 0b100001

static int playPressed=0, offsetPressed = 0;
bool InputHandlerRead(lv_indev_data_t * data)
{
	newEnc=FTM_GetQuadDecoderCounterValue(FTM2);
	if(GPIO_PinRead(BACKWARD_GPIO)==0)
	{
		data->key = LV_GROUP_KEY_PREV;
		state=LV_INDEV_STATE_PR;
		if(MP3PlayerData.currentScreen!=EQ_SCREEN)
			offsetPressed = -1;
		//GPIO_PinWrite(SCREEN_GPIO,0);
	}
	else if(GPIO_PinRead(FORWARD_GPIO)==0)
	{
		data->key=LV_GROUP_KEY_NEXT;
		state=LV_INDEV_STATE_PR;
		if(MP3PlayerData.currentScreen!=EQ_SCREEN)
			offsetPressed = 1;
		//GPIO_PinWrite(SCREEN_GPIO,1);
	}
	else if(GPIO_PinRead(PLAY_GPIO)==0)
	{
		playPressed=1;
		pwrDownCnt++;
		if(pwrDownCnt>=40)
		{

			smc_power_mode_vlls_config_t config;
			config.enablePorDetectInVlls0=true;
			config.subMode = kSMC_StopSub3;
			LLWU_SetExternalWakeupPinMode(LLWU, 9,  kLLWU_ExternalPinAnyEdge);
			LLWU_ClearExternalWakeupPinFlag(LLWU,9);
			//llwu_external_pin_filter_mode_t filterConfig;
			//filterConfig.pinIndex=9;
			//filterConfig.filterMode=kLLWU_PinFilterFallingEdge;
			//LLWU_SetPinFilterMode(LLWU,20,filterConfig);
			SMC_PreEnterStopModes();
			SMC_SetPowerModeVlls(SMC,&config);
			//PRINTF("Si estas leyendo esto no me dormi un carajo. =)\n");
			//SMC_PostExitStopModes();

		}
	}
	/*else if(GPIO_PinRead(GPIOA,4)==0)
	{
		data->key = LV_GROUP_KEY_LEFT;
		state=LV_INDEV_STATE_PR;
	}
	else if(GPIO_PinRead(GPIOC,6)==0)
	{
		data->key=LV_GROUP_KEY_RIGHT;

		state=LV_INDEV_STATE_PR;
	}*/
	else if(GPIO_PinRead(ENTER_GPIO)==0)
	{
		data->key=LV_GROUP_KEY_ENTER;

		state=LV_INDEV_STATE_PR;
	}
	else if(newEnc!=lastEnc && abs(newEnc-lastEnc)>=2)
	{
		if(lastEnc > 128)
		{
			if( (newEnc > (lastEnc-128)) && newEnc<lastEnc)
				storeEnc=ENC_LEFT;
			else
				storeEnc=ENC_RIGHT;
		}
		else
		{
			if( (newEnc < (lastEnc+128)) && newEnc>lastEnc)
				storeEnc=ENC_RIGHT;
			else
				storeEnc=ENC_LEFT;
		}

		lastEncCnt=0;
		lastEnc = newEnc;

	}
	else if(storeEnc==0b010010 && cnt==0)
	{
		if(MP3PlayerData.currentScreen==PLAY_SCREEN)
		{
			storeEnc=0b111111;
			if(MP3PlayerData.volume>0)
				MP3PlayerData.volume=MP3PlayerData.volume-1;
			data->key=LV_GROUP_KEY_ESC;
			data->state=LV_INDEV_STATE_REL;
			return false;
		}
		else if(MP3PlayerData.currentScreen==MAIN_SCREEN)
			data->key=LV_GROUP_KEY_PREV;
		else
			data->key = LV_GROUP_KEY_LEFT;
		encKey=data->key;
		state=LV_INDEV_STATE_PR;
		cnt++;
	}
	else if(storeEnc==0b100001 && cnt==0)
	{
		if(MP3PlayerData.currentScreen==PLAY_SCREEN)
		{
			storeEnc=0b111111;
			if(MP3PlayerData.volume<MAX_VOLUME)
				MP3PlayerData.volume=MP3PlayerData.volume+1;
			data->key=LV_GROUP_KEY_ESC;
			data->state=LV_INDEV_STATE_REL;
			return false;
		}
		else if(MP3PlayerData.currentScreen==MAIN_SCREEN)
			data->key=LV_GROUP_KEY_NEXT;
		else
			data->key = LV_GROUP_KEY_RIGHT;
		encKey=data->key;
		state=LV_INDEV_STATE_PR;
		cnt++;
	}
	else
	{
		if(playPressed==1)
		{
			MP3PlayerData.play^=1;
			playPressed=0;
		}
		if(offsetPressed != 0)
		{
			MP3PlayerData.offset = offsetPressed;
			offsetPressed = 0;
		}
		if(encKey!=LV_GROUP_KEY_ESC && cnt<2)
		{
			data->key=encKey;
			storeEnc=0b111111;
			state=LV_INDEV_STATE_PR;
			cnt++;
		}
		else
		{
			state=LV_INDEV_STATE_REL;
			encKey=LV_GROUP_KEY_ESC;
			cnt=0;
		}
		pwrDownCnt=0;
	}
	data->state = state;
    return false;       /*No more data to read so return false*/
}
