
#include "stm32l0xx_hal.h"
#include "printf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "main.h"
#include "hx711.h"


static HX711_CONFIG_T s_hx711Config={0};
static uint32_t s_aulArray[10];


void hx711_setGain(HX711_GAIN_E eGain){
	s_hx711Config.eGain=eGain;
}

void hx711_setTara(uint32_t ulTara){
	s_hx711Config.ulTara=s_hx711Config.ulTara+ulTara;
}

void hx711_setCali(uint32_t ulCali){
	s_hx711Config.ulCali=ulCali;
}


void hx711_setArraySize(int iArraySize){
	s_hx711Config.iArraySize=iArraySize;
}


void hx711_init(HX711_CONFIG_T *ptConfig){
	memcpy((void*)&s_hx711Config,(void*)ptConfig, sizeof(HX711_CONFIG_T));
}



static HX711_STATE_E hx711_convert(uint32_t *ulResult){
	GPIO_PinState eDoutState=GPIO_PIN_RESET;
	uint8_t bBitNr=0;
	uint32_t ulValue=0;

	HAL_GPIO_WritePin(HX711_SCK_GPIO_Port, HX711_SCK_Pin, GPIO_PIN_RESET);
	HAL_Delay(800); // wait to wakeup HX711
	eDoutState=HAL_GPIO_ReadPin(HX711_DOUT_GPIO_Port, HX711_DOUT_Pin);

	if(eDoutState==GPIO_PIN_SET){
		/*HX711 is not ready*/
		// set HX711 to sleep mode, (60usec after SCK=high, sleep mode)
		HAL_GPIO_WritePin(HX711_SCK_GPIO_Port, HX711_SCK_Pin, GPIO_PIN_SET);

		return HX711_NOT_READY;
	}

	for(bBitNr=0; bBitNr<25+s_hx711Config.eGain; bBitNr++){
		HAL_GPIO_TogglePin(HX711_SCK_GPIO_Port, HX711_SCK_Pin);   /* ___.--- rising edge  */
		HAL_GPIO_TogglePin(HX711_SCK_GPIO_Port, HX711_SCK_Pin);   /* ---.___ falling edge */
		eDoutState=HAL_GPIO_ReadPin(HX711_DOUT_GPIO_Port, HX711_DOUT_Pin);
		ulValue|=eDoutState;
		ulValue=ulValue<<1;
	}

	// set HX711 to sleep mode, (60usec after SCK=high, sleep mode)
	HAL_GPIO_WritePin(HX711_SCK_GPIO_Port, HX711_SCK_Pin, GPIO_PIN_SET);

	ulValue=ulValue>>s_hx711Config.iIgnoreBits;
	*ulResult=ulValue-s_hx711Config.ulTara;
	//*ulResult=ulValue;
	return HX711_OK;
}


static HX711_STATE_E hx711_fillArray(uint32_t *aulArray, uint32_t *ulAverage){
	int i;
	HX711_STATE_E eRslt=HX711_OK;

	*ulAverage=0;

	for(i=0;i<s_hx711Config.iArraySize;i++){
		eRslt=hx711_convert(&aulArray[i]);
		//eRslt=hx711_convertTest(&aulArray[i]);
		if(HX711_OK != eRslt){
			return eRslt;
		}
		*ulAverage+=aulArray[i];
		HAL_Delay(s_hx711Config.iMeasurementPause);
	}

	*ulAverage=*ulAverage/s_hx711Config.iArraySize;
	return HX711_OK;
}


static HX711_STATE_E hx711_filterArray(uint32_t *aulArray, uint32_t *ulResult){
	uint32_t ulNrElem=0;
	int i;
	uint32_t ulAverage=0, ulStdDeviation=0,ulDelta=0;


	/*calculate average*/
	for(i=0;i<s_hx711Config.iArraySize;i++){
		ulAverage+=aulArray[i];
	}
	ulAverage=ulAverage/s_hx711Config.iArraySize;


	/*calculate standard deviation*/
	for(i=0;i<s_hx711Config.iArraySize;i++){

		ulStdDeviation=ulStdDeviation+((aulArray[i]-ulAverage)*(aulArray[i]-ulAverage));
	}
	//	ulStdDeviation=(uint32_t)sqrt((float)ulStdDeviation);
	/*TODO: fix sqrt issue*/
#warning "Fix sqrt issue"

	/**/
	*ulResult=0;
	for(i=0;i<s_hx711Config.iArraySize;i++){

		ulDelta=(uint32_t)abs((int32_t)aulArray[i]-(int32_t)ulAverage);
		if(ulDelta<ulStdDeviation){
			*ulResult+=aulArray[i];
			ulNrElem++;
		}
	}

	if(ulNrElem==0){
		return HX711_FILTER_ERROR;
	}
	else {
		*ulResult=*ulResult/ulNrElem;
		if(ulNrElem==s_hx711Config.iArraySize){
			return HX711_OK;
		}
		else {
			return HX711_FILTER_ACTIVE;
		}

	}

}



HX711_STATE_E hx711_string(uint32_t ulWeight, char* szResult){
	HX711_STATE_E eRes=HX711_OK;

	uint32_t ulG;
	uint32_t ulR;

	ulG=ulWeight/1000;
	ulR=ulWeight%1000;

	snprintf(szResult,10,"%d.%02d",(int)ulG,(int)(ulR/10));

	return eRes;
}




uint32_t hx711_gramm(uint32_t ulWeight){
	if((int)ulWeight>=0){
	return (10000*ulWeight)/s_hx711Config.ulCali;
	}
	else {
		return 0;
	}
}



HX711_STATE_E hx711_measure(uint32_t* ulResult){
	HX711_STATE_E eRes=HX711_OK;

	eRes=hx711_fillArray(&s_aulArray[0], ulResult);
	//eRes=hx711_filterArray(&s_aulArray[0], ulResult);


	return eRes;
}

