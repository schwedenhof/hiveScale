/*
 * uartHandler.c
 *
 *  Created on: 17.03.2021
 *      Author: dirk
 */
#include <string.h>
#include "main.h"
#include "uartHandler.h"

extern APP_DATA_T g_tAppData;






void HAL_UART_RxCpltCallback (UART_HandleTypeDef * huart){
	int i=0;
	HAL_StatusTypeDef tRes= HAL_OK;

	if(huart==g_tAppData.tUart[APP_LPUART1].phUart){
		tRes=HAL_TIM_Base_Stop_IT(g_tAppData.phtim21); // stop timeout timer
		__HAL_TIM_SET_COUNTER(g_tAppData.phtim21,0);
		if(tRes!=HAL_OK){
			Error_Handler();
		}
		tRes=HAL_TIM_Base_Start_IT(g_tAppData.phtim21); // restart timeout timer
		if(tRes!=HAL_OK){
			Error_Handler();
		}
	}


	for (i=0;i<2;i++){
		if(huart==g_tAppData.tUart[i].phUart){

			// check if last received character is a new line (as end of message)
			if(g_tAppData.tUart[i].abRxBuffer[g_tAppData.tUart[i].iRxIdx]==(uint8_t)APP_NL){
				// in case of SIM800 UART, we additionally check for termination message
				if(huart==g_tAppData.tUart[APP_LPUART1].phUart){

					// check for various messages
					if(g_tAppData.tUart[i].iRxIdx>5){ //at least 8 characters received (check for >6 because iRxIdx not incremented yet)
						if(!strncmp((char*)&g_tAppData.tUart[i].abRxBuffer[g_tAppData.tUart[i].iRxIdx-7],"DST: 0\r\n",8)){
							g_tAppData.tUart[i].fRxMsg=UART_RX_DST; // frame termination "DST: 0\r\n" was received
							tRes=HAL_TIM_Base_Stop_IT(g_tAppData.phtim21); // stop timeout timer
							__HAL_TIM_SET_COUNTER(g_tAppData.phtim21,0);

						}
					}
					if(g_tAppData.tUart[i].iRxIdx>5){ //at least 7 characters received (check for >5 because iRxIdx not incremented yet)
						if(!strncmp((char*)&g_tAppData.tUart[i].abRxBuffer[g_tAppData.tUart[i].iRxIdx-6],"ERROR\r\n",7)){
							g_tAppData.tUart[i].fRxMsg=UART_RX_ERROR; // frame termination "ERROR\r\n" was received
							tRes=HAL_TIM_Base_Stop_IT(g_tAppData.phtim21); // stop timeout timer
							__HAL_TIM_SET_COUNTER(g_tAppData.phtim21,0);

						}
					}
					if(g_tAppData.tUart[i].iRxIdx>2){ //at least 4 characters received (check for >2 because iRxIdx not incremented yet)
						if(!strncmp((char*)&g_tAppData.tUart[i].abRxBuffer[g_tAppData.tUart[i].iRxIdx-3],"OK\r\n",4)){
							g_tAppData.tUart[i].fRxMsg=UART_RX_OK; // frame termination "OK\r\n" was received
							tRes=HAL_TIM_Base_Stop_IT(g_tAppData.phtim21); // stop timeout timer
							__HAL_TIM_SET_COUNTER(g_tAppData.phtim21,0);
						}
					}
				}
				else {
					g_tAppData.tUart[i].fRxMsg=UART_RX_NL; // OK, NL was received
				}
			}


			if(g_tAppData.tUart[i].iRxIdx < UART_RX_BUFFER_LEN){
				// if end of buffer not reached yet, increment index
				g_tAppData.tUart[i].iRxIdx++; // index points to next receive buffer byte and indicates correct number of already received bytes
			}

			if(g_tAppData.tUart[i].iRxIdx == UART_RX_BUFFER_LEN) {
				// if buffer full now, flush anyway, even if no OK
				g_tAppData.tUart[i].fRxMsg=UART_RX_BUFFER_FULL; // buffer full
			}


			if(g_tAppData.tUart[i].fRxMsg==UART_RX_ONGOING){
				// if message not complete yet, start waiting for next byte
				HAL_UART_Receive_IT(g_tAppData.tUart[i].phUart, &g_tAppData.tUart[i].abRxBuffer[g_tAppData.tUart[i].iRxIdx], 1);
			}
		}
	}

}



int Uh_UartReset(int eUart){
	HAL_UART_AbortReceive(g_tAppData.tUart[eUart].phUart); // abort possible ongoing receive in order to start new one with new buffer pointer
	g_tAppData.tUart[eUart].iRxIdx=0;
	g_tAppData.tUart[eUart].iDataLen=0;
	g_tAppData.tUart[eUart].fRxMsg=UART_RX_ONGOING;
	HAL_UART_Receive_IT(g_tAppData.tUart[eUart].phUart, &g_tAppData.tUart[eUart].abRxBuffer[0], 1);
return APP_OK;
}


/*
 *
 *
 * */
int Uh_UartRxWait(int eUart, uint32_t ulTimeout){
	HAL_StatusTypeDef eStat=HAL_OK;
	uint32_t ulTickStart, ulTickNow, ulTickDelta=0;
	ulTickStart=HAL_GetTick();
	int iRet=0;

	while( (UART_RX_ONGOING==g_tAppData.tUart[eUart].fRxMsg) & (ulTickDelta<ulTimeout)){
		ulTickNow=HAL_GetTick();
		if(ulTickNow>=ulTickStart){
			ulTickDelta=ulTickNow-ulTickStart;
		}
		else {
			ulTickDelta=ulTickNow+(0xFFFFFFFF-ulTickStart);
		}
	}

	if(UART_RX_ONGOING != g_tAppData.tUart[eUart].fRxMsg){
		iRet=g_tAppData.tUart[eUart].fRxMsg;
		g_tAppData.tUart[eUart].iDataLen=g_tAppData.tUart[eUart].iRxIdx;
		memcpy(g_tAppData.tUart[eUart].abDataBuffer, g_tAppData.tUart[eUart].abRxBuffer, g_tAppData.tUart[eUart].iDataLen);
		g_tAppData.tUart[eUart].fRxMsg=UART_RX_ONGOING;
		g_tAppData.tUart[eUart].iRxIdx=0;
		HAL_UART_AbortReceive(g_tAppData.tUart[eUart].phUart); // abort possible ongoing receive in order to start new one with new buffer pointer
		eStat=HAL_UART_Receive_IT(g_tAppData.tUart[eUart].phUart, &g_tAppData.tUart[eUart].abRxBuffer[0], 1);
		if(eStat==HAL_BUSY){
			g_tAppData.iErrorCnt++;
		}

	}
	else if(ulTimeout>0){
		iRet=UART_RX_TIMEOUT;
		HAL_UART_Receive_IT(g_tAppData.tUart[eUart].phUart, &g_tAppData.tUart[eUart].abRxBuffer[g_tAppData.tUart[eUart].iRxIdx], 1);
	}
	else {
		iRet=UART_RX_ONGOING;
	}

	return iRet;
}

