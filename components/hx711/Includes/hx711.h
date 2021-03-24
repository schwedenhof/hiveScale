
#ifndef __HX711_H
#define __HX711_H


#include <stdint.h>


typedef enum {
	HX711_GAIN_128 = 0,
	HX711_GAIN_32,
	HX711_GAIN_64
} HX711_GAIN_E;


typedef enum {
	HX711_OK = 0,
	HX711_NOT_READY,	
	HX711_FILTER_ACTIVE,
	HX711_FILTER_ERROR
} HX711_STATE_E;


typedef struct HX711_CONFIG_Ttag {
	HX711_GAIN_E eGain;
	int iIgnoreBits;
	int iArraySize;
	int iMeasurementPause;
	int iFilterDeviation;
	uint32_t ulTara;
	uint32_t ulCali;
} HX711_CONFIG_T;





HX711_STATE_E hx711_string(uint32_t ulWeight, char* szResult);
void hx711_init(HX711_CONFIG_T *tConfig);
HX711_STATE_E hx711_measure(uint32_t* ulResult);
uint32_t hx711_gramm(uint32_t ulWeight);
void hx711_setTara(uint32_t ulTara);
void hx711_setCali(uint32_t ulCali);


#endif
