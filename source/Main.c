#include <stdio.h>
#include <string.h>
#include "fsl_sd.h"
//#include "fsl_debug_console.h"
#include "board.h"

#include "pin_mux.h"
#include "clock_config.h"

#include "MP3UI.h"
#include "lvgl/lvgl.h"
#include "ili9341.h"
#include "InputHandler.h"
#include "MP3Player.h"
#include "LEDMatrix.h"
#include "PowerOffControl.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/
//Drivers for input and display
static lv_indev_drv_t kb_drv;
static lv_indev_t * kb_indev;
static lv_disp_drv_t disp;



/*******************************************************************************/

#define ARM_MATH_CM4	1
#include "arm_math.h"
#define F2Q15(x)  ((q15_t)((float32_t)x * 16384UL))
int main(void)
{

	POWEROFF.recover();

	/*Inicialización de la placa*/
    BOARD_InitPins();
    BOARD_BootClockRUN();
 //   BOARD_InitDebugConsole();

	/*inicialización de littlevgl*/
	lv_init();

	/*Inicialización de la pantalla*/
	ili9431_init();
	//Registramos la pantalla como output
	lv_disp_drv_init(&disp);
	disp.disp_flush = ili9431_flush;
	//disp.disp_fill = ili9431_fill;
	disp.disp_map = ili9431_map;
	lv_disp_drv_register(&disp);

	/*Inicialización de los inputs*/
	InputHandlerInit();
	//Registramos los inputs como un keypad
	kb_drv.type = LV_INDEV_TYPE_KEYPAD;
	kb_drv.read = InputHandlerRead;
	kb_indev = lv_indev_drv_register(&kb_drv);

	/*Creamos la interfaz de usuario*/
	MP3UiCreate(&kb_drv);

	/*Creamos el reproductor*/
	MP3Player.init();

	gpio_pin_config_t config = {
		 kGPIO_DigitalOutput,
		 0
	};
	GPIO_PinInit(GPIOA,2, &config);
	GPIO_PinInit(GPIOC,16, &config);


	/*float in[500];
	q15_t i[500];
	for(int i = 0; i < 500; i++)
	{
		in[i] = (arm_sin_f32(2*PI*21000.00*i/48000));
	}
	q15_t out[500];
	q15_t coeff[6];// b0, b1, b2, a1, a2
	//q15_t c[5];
	q15_t varState[4];//x_1, x_2, y_1, y_2;

	coeff[0] = F2Q15(0.1215975);
	coeff[1] = 0;
	coeff[2] = F2Q15(0.0751514);
	coeff[3] = F2Q15(0.1215975);
	coeff[4] = F2Q15(0.9095030);
	coeff[5] = F2Q15(-0.2278495);
	//arm_float_to_q15(coeff,c,5);
	arm_float_to_q15(in,i,500);
	varState[0] = varState[1] = varState[2] = varState[3] = 0;
	arm_biquad_casd_df1_inst_q15 f;
	arm_biquad_cascade_df1_init_q15(&f,1,coeff,varState,1);
	arm_biquad_cascade_df1_q15(&f,i,out,500);*/

	while (1)
	{
		GPIO_PinWrite(GPIOA,2,1);
		MP3Player.update();
		GPIO_PinWrite(GPIOA,2,0);
		GPIO_PinWrite(GPIOC,16,1);
		lv_task_handler();
		GPIO_PinWrite(GPIOC,16,0);
	}

	//printf("Thanks for using MP3\n");
	//GETCHAR();
}
