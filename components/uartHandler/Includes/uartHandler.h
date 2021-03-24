/*
 * uartHandler.h
 *
 *  Created on: 17.03.2021
 *      Author: dirk
 */

#ifndef UARTHANDLER_INCLUDES_UARTHANDLER_H_
#define UARTHANDLER_INCLUDES_UARTHANDLER_H_

#include <stdint.h>

enum {
	UART_RX_ONGOING = 0,
	UART_RX_NL,
	UART_RX_OK,
	UART_RX_ERROR,
	UART_RX_DST,              // string "DST: 0" received (SIM800 message at end of startup)
	UART_RX_BUFFER_FULL,
	UART_RX_ENDOFFRAME,
	UART_RX_TIMEOUT
};


int Uh_UartRxWait(int eUart, uint32_t ulTimeout);
int Uh_UartReset(int eUart);


#endif /* UARTHANDLER_INCLUDES_UARTHANDLER_H_ */
