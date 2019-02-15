#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include "../lvgl/lvgl.h"


void InputHandlerInit(int* pl,int* cs,int* vol);

bool InputHandlerRead(lv_indev_data_t * data);


#endif
