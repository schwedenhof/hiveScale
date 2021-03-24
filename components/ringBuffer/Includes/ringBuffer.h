/*
 * ringBuffer.h
 *
 *  Created on: 17.03.2021
 *      Author: dirk
 */

#ifndef RINGBUFFER_INCLUDES_RINGBUFFER_H_
#define RINGBUFFER_INCLUDES_RINGBUFFER_H_

#include <stdint.h>


typedef struct RB_DATA_Ttag{
	uint16_t usTime;
	uint32_t ulWeight;
} RB_DATA_T;


void Rb_DataPut(RTC_TimeTypeDef* ptTime, uint32_t ulWeight);
void Rb_StartGlance(void);
RB_DATA_T* Rb_DataGet(void);
RB_DATA_T* Rb_DataGlance(void);
int Rb_BufferInfo();
void Rb_BufferClear();



#endif /* RINGBUFFER_INCLUDES_RINGBUFFER_H_ */
