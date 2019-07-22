/*
 * PowerOffControl.h
 *
 *  Created on: 5 mar. 2019
 *      Author: Maxi
 */

#ifndef POWEROFFCONTROL_H_
#define POWEROFFCONTROL_H_

typedef struct POWEROFF_ POWEROFF_;

extern POWEROFF_ POWEROFF;

struct POWEROFF_
{
	/**
	* @brief			Put the MCU in Low power mode
	*/
	void(*powerOff)();

	/**
	* @brief 			Recover MCU from Low power mode
	*/
	void(*recover)();

	/**
	* @brief 			Init power control module
	*/
	void(*init)();
};
#endif /* POWEROFFCONTROL_H_ */
