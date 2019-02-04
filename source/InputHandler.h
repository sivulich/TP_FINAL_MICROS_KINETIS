#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include "../lvgl/lvgl.h"


void InputHandlerInit(void);

bool InputHandlerRead(lv_indev_data_t * data);

#endif
