/*
 * remanentDataHandler.c
 *
 *  Created on: 17.03.2021
 *      Author: dirk
 */

#include <string.h>
#include "remanentDataHandler.h"
#include "stm32l0xx_hal.h"

void Rd_Load(uint8_t* pbRemanentData, size_t tSize){
//ToDo
	#if(0)
	uint8_t* pbEeprom=(uint8_t*)DATA_EEPROM_BASE;
	uint8_t* pbDest=pbRemanentData;
	int i;
#endif

	memcpy((void*)pbRemanentData, (void*)DATA_EEPROM_BASE, tSize);

#if(0)
	for(i=0;i<tSize;i++){
		*(pbDest++)=*(pbEeprom++);
	}
#endif

}



void Rd_Store(uint8_t* pbRemanentData, size_t tSize){
	int i;
	uint8_t* pbEeprom=(uint8_t*)DATA_EEPROM_BASE;

	/* (1) Wait till no operation is on going */
	/* (2) Check if the PELOCK is unlocked */
	/* (3) Perform unlock sequence */
	while ((FLASH->SR & FLASH_SR_BSY) != 0) /* (1) */
	{
		/* For robust implementation, add here time-out management */
	}
	if ((FLASH->PECR & FLASH_PECR_PELOCK) != 0) /* (2) */
	{
		FLASH->PEKEYR = FLASH_PEKEY1; /* (3) */
		FLASH->PEKEYR = FLASH_PEKEY2;
	}


	for(i=0;i<tSize;i++){
		*(pbEeprom++)=*(pbRemanentData++);
	}


	/* (1) Wait till no operation is on going */
	/* (2) Locks the NVM by setting PELOCK in PECR */
	while ((FLASH->SR & FLASH_SR_BSY) != 0) /* (1) */
	{
		/* For robust implementation, add here time-out management */
	}
	FLASH->PECR |= FLASH_PECR_PELOCK; /* (2) */

}

