#include "InputHandler.h"
#include "fsl_gpio.h"
#include "../lv_conf.h"

static lv_indev_state_t state;

void InputHandlerInit(void)
{
	//Initialize non-SPI GPIOs
	gpio_pin_config_t config = {
	     kGPIO_DigitalInput,
	 	 0
	};

	GPIO_PinInit(GPIOA,4, &config);
	GPIO_PinInit(GPIOC,6, &config);

	GPIO_PinInit(GPIOC,0, &config);		//fordward
	GPIO_PinInit(GPIOC,5, &config);		//back
	GPIO_PinInit(GPIOC,7, &config);		//play/pause
	GPIO_PinInit(GPIOC,9, &config);		//A
	GPIO_PinInit(GPIOC,8, &config);		//B
}
static int lastEnc=0b11,newEnc,storeEnc,cnt=0;;
static lv_indev_state_t encKey=LV_GROUP_KEY_ESC;
static int lastEncCnt=0;
void InputUpdate()
{
	newEnc=(GPIO_PinRead(GPIOC,9)<<1) | GPIO_PinRead(GPIOC,8);
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
		lastEncCnt=0;
	//lastEnc=newEnc;

}
bool InputHandlerRead(lv_indev_data_t * data)
{
	newEnc=(GPIO_PinRead(GPIOC,9)<<1) | GPIO_PinRead(GPIOC,8);
	if(GPIO_PinRead(GPIOC,5)==0)
	{
		data->key = LV_GROUP_KEY_PREV;
		state=LV_INDEV_STATE_PR;
	}
	else if(GPIO_PinRead(GPIOC,0)==0)
	{
		data->key=LV_GROUP_KEY_NEXT;
		state=LV_INDEV_STATE_PR;
	}
	else if(GPIO_PinRead(GPIOA,4)==0)
	{
		data->key = LV_GROUP_KEY_LEFT;
		state=LV_INDEV_STATE_PR;
	}
	else if(GPIO_PinRead(GPIOC,6)==0)
	{
		data->key=LV_GROUP_KEY_RIGHT;

		state=LV_INDEV_STATE_PR;
	}
	else if(GPIO_PinRead(GPIOC,7)==0)
	{
		data->key=LV_GROUP_KEY_ENTER;

		state=LV_INDEV_STATE_PR;
	}
	//else if(newEnc!=0b11 && cnt==0) //ESTO ANDABA PERO MEDIO CABEZA Y SE EQUIVOCA
	//{
		/*if(last==0b11 && new==0b01)
		{
			data->key = LV_GROUP_KEY_LEFT;
			encKey=LV_GROUP_KEY_LEFT;
			state=LV_INDEV_STATE_PR;
		}
		else if(last==0b11 && new==0b10)
		{
			data->key=LV_GROUP_KEY_RIGHT;
			encKey=LV_GROUP_KEY_RIGHT;
			state=LV_INDEV_STATE_PR;
		}
		else
		{
			data->key=encKey;
			state=LV_INDEV_STATE_PR;
		}*/
		//if(lastEnc!=newEnc)
		//{
			//storeEnc=((storeEnc<<2)|newEnc)&0b111111;
	else if(storeEnc==0b010010 && cnt==0)
	{
		data->key = LV_GROUP_KEY_LEFT;
		encKey=LV_GROUP_KEY_LEFT;
		state=LV_INDEV_STATE_PR;
		cnt++;
	}
	else if(storeEnc==0b100001 &&cnt==0)
	{
		data->key=LV_GROUP_KEY_RIGHT;
		encKey=LV_GROUP_KEY_RIGHT;
		state=LV_INDEV_STATE_PR;
		cnt++;
	}
		//}

	//}
	else
	{
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
