/*
 * FTM_Handler.h
 *
 *  Created on: 13 feb. 2020
 *      Author: Fabian
 */

#ifndef FTM_HANDLER_H_
#define FTM_HANDLER_H_

void FTM_Handler_Init(uint8_t* transmitBuf, uint8_t size);
void FTM_PIN1_HANDLER(void);
void FTM_Start();
void FTM_Stop();

#endif /* FTM_HANDLER_H_ */
