#include "InputHandler.h"
#include "fsl_gpio.h"
#include "fsl_ftm.h"
#include "PowerOffControl.h"
#include "../lv_conf.h"
#include <math.h>
#include "MP3Ui.h"
#include "MP3PlayerData.h"

//Keypad GPIOs
#define FORWARD_GPIO	GPIOB,6		//cambio a GPIOB,4
#define BACKWARD_GPIO	GPIOB,4		//cambio a GPIOB,6
#define PLAY_GPIO		GPIOB,5		//cambio a GPIOB,5
#define ENTER_GPIO		GPIOB,20		//cambio a GPIOB,20
#define ONOFF_GPIO		GPIOA,13		//nuevo, para el segundo encoder

//Para el modulo Bluetooth(no existe mas)
//#define VOLP_BT			GPIOD,14
//#define VOLN_BT			GPIOD,13
//#define NEXT_BT			GPIOD,12
//#define PREV_BT			GPIOD,11
//#define PLAY_BT			GPIOD,10
//#define AUX_DET			GPIOD,15

//#define A_GPIO GPIOC,1
//#define B_GPIO GPIOC,9
//#define SCREEN_GPIO GPIOB,9

//FTM modulos
#define ENC_FTM			FTM2
#define VOL_FTM			FTM1
#define ENCODER_MAX_VAL 255


//#define ENC_RIGHT 0b010010
//#define ENC_LEFT 0b100001
//static lv_indev_state_t state;
void InputHandlerInit()
{
	//Initialize non-SPI GPIOs
	gpio_pin_config_t config = {
	     kGPIO_DigitalInput,
	 	 0
	};

	ftm_config_t configFTM;
	FTM_GetDefaultConfig(&configFTM);
	FTM_Init(ENC_FTM, &configFTM);
	FTM_Init(VOL_FTM, &configFTM);
	//FTM_Init(VOL_FTM, &configFTM);

	ftm_phase_params_t paramsPhase;
	paramsPhase.enablePhaseFilter = 1;
	paramsPhase.phaseFilterVal = 4;
	paramsPhase.phasePolarity = kFTM_QuadPhaseInvert;

	ftm_quad_decode_mode_t quadMode = kFTM_QuadPhaseEncode;

	uint32_t startVal = 0;
	uint32_t overVal = ENCODER_MAX_VAL;
	FTM_SetQuadDecoderModuloValue(ENC_FTM, startVal, overVal);
	FTM_ClearQuadDecoderCounterValue(ENC_FTM);
	FTM_SetQuadDecoderModuloValue(VOL_FTM, startVal, overVal);
	FTM_ClearQuadDecoderCounterValue(VOL_FTM);

	FTM_SetupQuadDecode(ENC_FTM,&paramsPhase,&paramsPhase,quadMode);
	FTM_SetupQuadDecode(VOL_FTM,&paramsPhase,&paramsPhase,quadMode);

//	GPIO_PinInit(GPIOA,4, &config);
//	GPIO_PinInit(GPIOC,6, &config);

	//GPIO_PinInit(FORWARD_GPIO, &config);		//fordward
	GPIO_PinInit(BACKWARD_GPIO, &config);		//backward
	GPIO_PinInit(PLAY_GPIO, &config);		//play/pause
	GPIO_PinInit(ENTER_GPIO, &config);     //Enter
	//GPIO_PinInit(A_GPIO, &config);		//A
	//GPIO_PinInit(B_GPIO, &config);		//B
}
//static int lastEnc = 0b11, newEnc, storeEnc = 0b11, cnt = 0, lastEncCnt = 0;
static int lastEnc = 0b11, newEnc;
static int lastVolEnc = 0b11, newVolEnc, volEncDiff;
static unsigned long long pwrDownCnt = 0;
//static lv_indev_state_t encKey=LV_GROUP_KEY_ESC;

static int playPressed=0, offsetPressed = 0;//,changeModePressed=0;
/*bool InputHandlerRead(lv_indev_data_t * data)
{
	newEnc=FTM_GetQuadDecoderCounterValue(FTM2);
	if(GPIO_PinRead(BACKWARD_GPIO)==0)
	{
		if( MP3PlayerData.currentScreen==MAIN_SCREEN ||
		    MP3PlayerData.currentScreen==PLAY_SCREEN ||
			MP3PlayerData.currentScreen==FILE_SCREEN1||
			MP3PlayerData.currentScreen==FILE_SCREEN0)
		{
			offsetPressed = -1;
		}
		else
		{
			data->key = LV_GROUP_KEY_PREV;
			state=LV_INDEV_STATE_PR;
			//if(MP3PlayerData.currentScreen!=EQ_SCREEN)
		}
	}
	else if(GPIO_PinRead(FORWARD_GPIO)==0)
	{
		if( MP3PlayerData.currentScreen==MAIN_SCREEN ||
		    MP3PlayerData.currentScreen==PLAY_SCREEN ||
			MP3PlayerData.currentScreen==FILE_SCREEN1||
			MP3PlayerData.currentScreen==FILE_SCREEN0 )
		{
			offsetPressed = 1;
		}
		else
		{
			data->key = LV_GROUP_KEY_NEXT;
			state=LV_INDEV_STATE_PR;
			//if(MP3PlayerData.currentScreen!=EQ_SCREEN)
			//	offsetPressed = 1;
		}
	}
	else if(GPIO_PinRead(PLAY_GPIO)==0)
	{
		playPressed=1;
		pwrDownCnt++;
		if(pwrDownCnt>=40)
			POWEROFF.powerOff();
	}
	else if(GPIO_PinRead(GPIOA,4)==0)
	{
		changeModePressed=-1;
	}
	else if(GPIO_PinRead(GPIOC,6)==0)
	{
		changeModePressed=1;
	}
	else if(GPIO_PinRead(ENTER_GPIO)==0)
	{
		data->key=LV_GROUP_KEY_ENTER;

		state=LV_INDEV_STATE_PR;
	}
	else if(newEnc!=lastEnc && abs(newEnc-lastEnc)>=2)
	{
		if(lastEnc > (ENCODER_MAX_VAL+1)/2)
		{
			if( (newEnc > (lastEnc-(ENCODER_MAX_VAL+1)/2)) && newEnc<lastEnc)
				storeEnc=ENC_LEFT;
			else
				storeEnc=ENC_RIGHT;
		}
		else
		{
			if( (newEnc < (lastEnc+(ENCODER_MAX_VAL+1)/2)) && newEnc>lastEnc)
				storeEnc=ENC_RIGHT;
			else
				storeEnc=ENC_LEFT;
		}

		lastEncCnt=0;
		lastEnc = newEnc;

	}
	else if(storeEnc==ENC_RIGHT && cnt==0)
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
	else if(storeEnc==ENC_LEFT && cnt==0)
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
		if(playPressed == 1)
		{
			MP3PlayerData.play^=1;
			playPressed=0;
		}
		if(offsetPressed != 0)
		{
			MP3PlayerData.offset = offsetPressed;
			offsetPressed = 0;
		}
		if(changeModePressed!=0)
		{
			MP3PlayerData.vumeterMode=(VUMETERS_MODES+MP3PlayerData.vumeterMode+changeModePressed)%VUMETERS_MODES;
			changeModePressed=0;
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
    return false;
}*/

int tempEnconderDiff=0;
bool encoder_read(lv_indev_data_t*data){
	if(tempEnconderDiff!=0)
	{
		data->enc_diff=tempEnconderDiff;
		tempEnconderDiff=0;
		return false;
	}
	newEnc=FTM_GetQuadDecoderCounterValue(ENC_FTM);
	if(newEnc!=lastEnc && abs(newEnc-lastEnc)>=2)
	{
		if(lastEnc > (ENCODER_MAX_VAL+1)/2)
		{
			if( (newEnc > (lastEnc-(ENCODER_MAX_VAL+1)/2)) && newEnc<lastEnc)
				tempEnconderDiff=1;
			else
				tempEnconderDiff=-1;
		}
		else
		{
			if( (newEnc < (lastEnc+(ENCODER_MAX_VAL+1)/2)) && newEnc>lastEnc)
				tempEnconderDiff=-1;
			else
				tempEnconderDiff=1;
		}
		data->enc_diff=tempEnconderDiff;
		//lastEncCnt=0;
		lastEnc = newEnc;
	}
	newVolEnc=FTM_GetQuadDecoderCounterValue(VOL_FTM);
	if(newVolEnc!=lastVolEnc && abs(newVolEnc-lastVolEnc)>=2)
	{
		if(lastVolEnc > (ENCODER_MAX_VAL+1)/2)
		{
			if( (newVolEnc > (lastVolEnc-(ENCODER_MAX_VAL+1)/2)) && newVolEnc<lastVolEnc)
				volEncDiff=1;
			else
				volEncDiff=-1;
		}
		else
		{
			if( (newVolEnc < (lastVolEnc+(ENCODER_MAX_VAL+1)/2)) && newVolEnc>lastVolEnc)
				volEncDiff=-1;
			else
				volEncDiff=1;
		}
		if(MP3PlayerData.volume+volEncDiff<=MAX_VOLUME && MP3PlayerData.volume+volEncDiff>=MIN_VOLUME)
			MP3PlayerData.volume+=volEncDiff;
		// data->volEnc_diff=tempEnconderDiff;
		//lastEncCnt=0;
		lastVolEnc = newVolEnc;
	}


	if(GPIO_PinRead(ENTER_GPIO)==0){
		data->state = LV_INDEV_STATE_PR;
	}
	else if(GPIO_PinRead(BACKWARD_GPIO)==0)
	{
		offsetPressed = -1;
	}
	else if(GPIO_PinRead(FORWARD_GPIO)==0)
	{
		offsetPressed = 1;
	}
	else if(GPIO_PinRead(PLAY_GPIO)==0)
	{
		playPressed=1;
		pwrDownCnt++;
		if(pwrDownCnt>=40)
			POWEROFF.powerOff();
	}
	else{
		pwrDownCnt=0;
		if(playPressed == 1)
		{
			MP3PlayerData.play^=1;
			playPressed=0;
		}
		if(offsetPressed != 0)
		{
			MP3PlayerData.offset = offsetPressed;
			offsetPressed = 0;
		}
		data->state = LV_INDEV_STATE_REL;
	}

	return false; /*No buffering so no more data read*/
}
