#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include "../lvgl/lvgl.h"


void InputHandlerInit();

bool InputHandlerRead(lv_indev_data_t * data);
bool encoder_read(lv_indev_data_t* data);


#endif
