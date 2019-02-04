#include "InputHandler.h"
#include "fsl_gpio.h"


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
}

bool InputHandlerRead(lv_indev_data_t * data)
{

	if(GPIO_PinRead(GPIOA,4)==0)
	{
		data->key=LV_GROUP_KEY_ENTER;
		state = LV_INDEV_STATE_PR;
	}
	else if(GPIO_PinRead(GPIOC,6)==0)
	{
		data->key=LV_GROUP_KEY_RIGHT;
		state = LV_INDEV_STATE_PR;
	}
	else
		state=LV_INDEV_STATE_REL;
	data->state = state;
    return false;       /*No more data to read so return false*/
}
