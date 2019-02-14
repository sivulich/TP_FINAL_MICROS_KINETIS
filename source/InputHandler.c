#include "InputHandler.h"
#include "fsl_gpio.h"
#include "fsl_ftm.h"
#include "../lv_conf.h"

#define FORWARD_GPIO GPIOC,5
#define BACKWARD_GPIO GPIOC,0
#define PLAY_GPIO GPIOC,7
#define A_GPIO GPIOC,1
#define ENTER_GPIO GPIOC,8
#define B_GPIO GPIOC,9



static lv_indev_state_t state;
static int* play;
void InputHandlerInit(int* pl)
{
	//Initialize non-SPI GPIOs
	play=pl;
	gpio_pin_config_t config = {
	     kGPIO_DigitalInput,
	 	 0
	};

	ftm_config_t configFTM;
	FTM_GetDefaultConfig(&configFTM);
	FTM_Init(FTM2, &configFTM);

	ftm_phase_params_t paramsPhase;
	paramsPhase.enablePhaseFilter = 0;
	//paramsPhase.phaseFilterVal = xx;
	paramsPhase.phasePolarity = kFTM_QuadPhaseInvert;

	ftm_quad_decode_mode_t quadMode = kFTM_QuadPhaseEncode;
	FTM_SetupQuadDecode(FTM2,&paramsPhase,&paramsPhase,quadMode);
	//FTM2->MODE |= FTM_MODE_WPDIS_MASK;
	uint32_t startVal = 0;
	uint32_t overVal = 255;
	FTM_SetQuadDecoderModuloValue(FTM2, startVal, overVal);
	FTM_ClearQuadDecoderCounterValue(FTM2);

	GPIO_PinInit(GPIOA,4, &config);
	GPIO_PinInit(GPIOC,6, &config);

	GPIO_PinInit(FORWARD_GPIO, &config);		//fordward
	GPIO_PinInit(BACKWARD_GPIO, &config);		//backward
	GPIO_PinInit(PLAY_GPIO, &config);		//play/pause
	GPIO_PinInit(A_GPIO, &config);		//A
	GPIO_PinInit(ENTER_GPIO, &config);     //Enter
	GPIO_PinInit(B_GPIO, &config);		//B
}
static int lastEnc=0b11,newEnc,storeEnc=0b11,cnt=0;
static lv_indev_state_t encKey=LV_GROUP_KEY_ESC;
static int lastEncCnt=0;
#define ENC_LEFT 0b010010
#define ENC_RIGHT 0b100001
void InputUpdate()
{
	newEnc=FTM_GetQuadDecoderCounterValue(FTM2);
	if(newEnc!=lastEnc)
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
		lastEnc = newEnc;
	}

	/*newEnc=(GPIO_PinRead(A_GPIO)<<1) | GPIO_PinRead(B_GPIO);
	if(lastEnc!=newEnc && storeEnc!=0b010010 && storeEnc!=0b100001)
	{
		if(lastEncCnt>=4)
		{
			storeEnc=((storeEnc<<2)|newEnc)&0b111111;
			lastEnc=newEnc;
			lastEncCnt=0;
		}
		else
			lastEncCnt++;
	}
	else
		lastEncCnt=0;*/

}
static int playPressed=0;
bool InputHandlerRead(lv_indev_data_t * data)
{
	if(GPIO_PinRead(BACKWARD_GPIO)==0)
	{
		data->key = LV_GROUP_KEY_PREV;
		state=LV_INDEV_STATE_PR;
	}
	else if(GPIO_PinRead(FORWARD_GPIO)==0)
	{
		data->key=LV_GROUP_KEY_NEXT;
		state=LV_INDEV_STATE_PR;
	}
	else if(GPIO_PinRead(PLAY_GPIO)==0)
	{
		playPressed=1;
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
	else if(storeEnc==0b010010 && cnt==0)
	{
		data->key = LV_GROUP_KEY_LEFT;
		encKey=LV_GROUP_KEY_LEFT;
		state=LV_INDEV_STATE_PR;
		cnt++;
	}
	else if(storeEnc==0b100001 && cnt==0)
	{
		data->key=LV_GROUP_KEY_RIGHT;
		encKey=LV_GROUP_KEY_RIGHT;
		state=LV_INDEV_STATE_PR;
		cnt++;
	}
	else
	{
		if(playPressed==1)
		{
			*play^=1;
			playPressed=0;
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

	}
	data->state = state;
    return false;       /*No more data to read so return false*/
}
