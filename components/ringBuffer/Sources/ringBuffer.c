/*
 * ringBuffer.c
 *
 *  Created on: 17.03.2021
 *      Author: dirk
 */

#include "stm32l0xx_hal.h"
#include "main.h"
#include "ringBuffer.h"


static int g_iLogDataStart;
static int g_iLogDataEnd;
static int g_iLogDataGlance; // used to run through the buffer without removing elements
static RB_DATA_T g_tLogData[APP_MAX_LOG_DATA_SETS];




void Rb_DataPut(RTC_TimeTypeDef* ptTime, uint32_t ulWeight){
	g_tLogData[g_iLogDataEnd].usTime=(ptTime->Hours<<8) + ptTime->Minutes;
	g_tLogData[g_iLogDataEnd].ulWeight=ulWeight;
	g_iLogDataEnd++;

	if(g_iLogDataEnd>=APP_MAX_LOG_DATA_SETS){
		g_iLogDataEnd=0; // return to start of ringbuffer
	}

	if(g_iLogDataStart==g_iLogDataEnd){
		g_iLogDataStart++;
		if(g_iLogDataStart>=APP_MAX_LOG_DATA_SETS){
			g_iLogDataStart=0; // return to start of ringbuffer
		}
	}
}



RB_DATA_T* Rb_DataGet(){
	RB_DATA_T* ptData;

	if(g_iLogDataStart == g_iLogDataEnd){
		ptData=0;
	}
	else {
		ptData= &g_tLogData[g_iLogDataStart];
		g_iLogDataStart++;
		if(g_iLogDataStart>=APP_MAX_LOG_DATA_SETS){
			g_iLogDataStart=0; // return to start of ringbuffer
		}
	}

	return ptData;
}


void Rb_StartGlance(void){
	g_iLogDataGlance=g_iLogDataStart;
}


RB_DATA_T* Rb_DataGlance(){
	RB_DATA_T* ptData;

	if(g_iLogDataGlance == g_iLogDataEnd){
		ptData=0;
	}
	else {
		ptData= &g_tLogData[g_iLogDataGlance];
		g_iLogDataGlance++;
		if(g_iLogDataGlance>=APP_MAX_LOG_DATA_SETS){
			g_iLogDataGlance=0; // return to start of ringbuffer
		}
	}

	return ptData;
}


void Rb_BufferClear(){
	g_iLogDataEnd=0;
	g_iLogDataStart=0;
}



int Rb_BufferInfo(){
	if(g_iLogDataStart<=g_iLogDataEnd){
		return g_iLogDataEnd-g_iLogDataStart;
	}
	else {
		return (APP_MAX_LOG_DATA_SETS-g_iLogDataStart + g_iLogDataEnd);
	}
}

