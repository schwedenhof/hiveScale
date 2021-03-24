/*
 * remanentDataHandler.h
 *
 *  Created on: 17.03.2021
 *      Author: dirk
 */

#ifndef REMANENTDATAHANDLER_INCLUDES_REMANENTDATAHANDLER_H_
#define REMANENTDATAHANDLER_INCLUDES_REMANENTDATAHANDLER_H_

#include <stdint.h>
#include "stm32l0xx_hal.h"


void Rd_Load(uint8_t* pbRemanentData, size_t tSize);
void Rd_Store(uint8_t* pbRemanentData, size_t tSize);

#endif /* REMANENTDATAHANDLER_INCLUDES_REMANENTDATAHANDLER_H_ */
