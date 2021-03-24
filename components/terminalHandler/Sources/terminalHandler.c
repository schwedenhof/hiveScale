
/**
 ******************************************************************************
 * @file           : terminalhandler.c
 * @brief          :
 ******************************************************************************
 *
 * This software is published under GNU General Public License v3.0.
 * It is hosted on github: https://github.com/schwedenhof/hiveScale
 *
 * Author: Drk Fischer, www.schwedenhof.net
 *
 ******************************************************************************
 */



#include "printf.h"
#include "main.h"
#include "remanentDataHandler.h"
#include "terminalHandler.h"
#include "uartHandler.h"
#include "ringBuffer.h"
#include "hx711.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>



/** Terminal service commands */
TERM_TERMINAL_CMD_T g_tTerminalCommands[] =
{
		{TERM_HELP,          "help"},
		{TERM_BLINK_ON,      "bln1"},
		{TERM_BLINK_OFF,     "bln0"},
		{TERM_RTC,           "rtc"},
		{TERM_AT,            "at"},
		{TERM_BAT,           "bat"},
		{TERM_BAT_LOW,       "batl"},
		{TERM_BAT_HIGH,      "bath"},
		{TERM_HX,            "hx"},
		{TERM_TARA,          "tara"},
		{TERM_CALI,          "cali"},
		{TERM_PUSH,          "push"},
		{TERM_ALARM,         "alrm"},
		{TERM_SETPARAM,      "setp"},
		{TERM_SETTIME,       "sett"},
		{TERM_STARTAPP,      "app"},
		{TERM_SIM800_ON,     "on"},
		{TERM_SIM800_OFF,    "off"},
		{TERM_BUFLIST, 		 "bufl"},
		{TERM_BUFFLUSH, 	 "buff"},
		{TERM_BUFCLR, 		 "bufc"},
		{TERM_BUFINFO, 		 "bufi"},
		{TERM_REMINFO, 		 "remi"},
		{TERM_REMLOAD, 		 "reml"},
		{TERM_REMSTORE,      "rems"},
};



const char* pCmd0 = "ate0\r";                                // disable echo
const char* pCmd1 = "at+sapbr=3,1,\"Contype\",\"GPRS\"\r";   // set parameter
const char* pCmd2 = "at+sapbr=3,1,\"APN\",\"";               // set parameter - to be completed by actual APN of network
const char* pCmd3 = "at+sapbr=1,1\r";                        // switch on GPRS (IP application)
const char* pCmd4 = "at+httpinit\r";                         // init http service
const char* pCmd5 = "at+httppara=\"CID\",1\r";               // set parmeter
const char* pCmd6 = "at+httppara=\"URL\",\"";                // set parmeter - to be completed by actual used URL
const char* pCmd7 = "at+httpdata=10240,6000\r";              // data  10KB, 6sec (10KB @19200baud ~4.3sec)
const char* pCmd8 = "at+httpaction=1\r";                     // execute command POST
const char* pCmd9 = "at+httpterm\r";                         // terminate http service
const char* pCmd10 = "at+sapbr=0,1\r";                       // switch off GPRS (IP application)



const char* pCmd[11];

void terminalInitCmdList(void){
	pCmd[0]=pCmd0;
	pCmd[1]=pCmd1;
	pCmd[2]=pCmd2;
	pCmd[3]=pCmd3;
	pCmd[4]=pCmd4;
	pCmd[5]=pCmd5;
	pCmd[6]=pCmd6;
	pCmd[7]=pCmd7;
	pCmd[8]=pCmd8;
	pCmd[9]=pCmd9;
	pCmd[10]=pCmd10;

}


extern APP_DATA_T g_tAppData;


static APP_RETURN_E setTimeDate(char *szCltsString, RTC_TimeTypeDef *tTime, RTC_DateTypeDef *tDate){
	//+CCLK: "21/01/27,15:35:41+04"
	int i=27;
	szCltsString[i]=0;

	if(strlen(szCltsString)<25){
		return APP_ERROR;
	}


	i=i-2;
	tTime->Seconds=(uint8_t)atoi(&szCltsString[i]);
	szCltsString[--i]=0;

	i=i-2;
	tTime->Minutes=(uint8_t)atoi(&szCltsString[i]);
	szCltsString[--i]=0;

	i=i-2;
	tTime->Hours=(uint8_t)atoi(&szCltsString[i]);
	szCltsString[--i]=0;

	i=i-2;
	tDate->Date=(uint8_t)atoi(&szCltsString[i]);
	szCltsString[--i]=0;

	i=i-2;
	tDate->Month=(uint8_t)atoi(&szCltsString[i]);
	szCltsString[--i]=0;

	i=i-2;
	tDate->Year=(uint8_t)atoi(&szCltsString[i]);
	szCltsString[--i]=0;


	if(tDate->Month == 0 || tDate->Date == 0){
		return APP_ERROR;
	}
	else {
		return APP_OK;
	}



}


#define MAXCNT 50

APP_RETURN_E getAdcValue(uint32_t* pulAdcValue, uint32_t* pulBatVoltage){

	int i;
	volatile uint32_t ulOffset;
	volatile uint32_t ulGrad;
	volatile uint32_t ulValue[MAXCNT];
	uint32_t ulSum=0;

	/* ### - 2 - Start calibration ############################################ */
	//			if (HAL_ADCEx_Calibration_Start(&AdcHandle, ADC_SINGLE_ENDED) != HAL_OK)
	//			{
	//				Error_Handler();
	//			}

	// THIS HAPPENS IN CubeMX INIT Routien already
	/* ### - 3 - Channel configuration ######################################## */
	/* Select Channel 0 to be converted */
	//			sConfig.Channel = ADC_CHANNEL_0;
	//			if (HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK)
	//			{
	//				Error_Handler();
	//			}

	for(i=0;i<MAXCNT;i++)
	{
		/*##- 4- Start the conversion process #######################################*/
		if (HAL_ADC_Start(g_tAppData.phadc) != HAL_OK)
		{
			/* Start Conversation Error */
			Error_Handler();
		}

		/*##- 5- Wait for the end of conversion #####################################*/
		/*  Before starting a new conversion, you need to check the current state of
   the peripheral; if it’s busy you need to wait for the end of current
   conversion before starting a new one.
   For simplicity reasons, this example is just waiting till the end of the
   conversion, but application may perform other tasks while conversion
   operation is ongoing. */
		if (HAL_ADC_PollForConversion(g_tAppData.phadc, 10) != HAL_OK)
		{
			/* End Of Conversion flag not set on time */
			Error_Handler();
		}

		/* Check if the continuous conversion of regular channel is finished */
		if ((HAL_ADC_GetState(g_tAppData.phadc) & HAL_ADC_STATE_REG_EOC) == HAL_ADC_STATE_REG_EOC)
		{
			/*##-6- Get the converted value of regular channel  ########################*/
			ulValue[i]= HAL_ADC_GetValue(g_tAppData.phadc);
			ulSum+=ulValue[i];

		}
		//	HAL_Delay(100);
	}

	*pulAdcValue=ulSum/(uint32_t)MAXCNT;
	ulGrad=(10*(g_tAppData.tRemanentData.ulAdcHigh-g_tAppData.tRemanentData.ulAdcLow))/(g_tAppData.tRemanentData.ulVoltHigh-g_tAppData.tRemanentData.ulVoltLow); // ulAdcVolt voltage delta between AdcLow and AdcHigh value
	ulOffset=10*g_tAppData.tRemanentData.ulAdcHigh-ulGrad*g_tAppData.tRemanentData.ulVoltHigh;
	*pulBatVoltage=1000*((10*(*pulAdcValue)-ulOffset))/ulGrad;

	return APP_OK;
}










/*****************************************************************************/
/*! Term_MatchTerminalCommand
 *   \param szCmd command string
 *   \return Terminal Command code                                            */
/*****************************************************************************/
static int Term_MatchTerminalCommand(char *szCmd)
{
	int i=0;
	int iTerminalCommandCode=TERM_UNKNOWN;

	for(i=0; i<(TERM_LAST-1); i++){
		if( 0 == strcmp(szCmd, &g_tTerminalCommands[i].szString[0])){
			iTerminalCommandCode=g_tTerminalCommands[i].iCode;
			break;
		}
	}
	// exception: AT command: no exact match required, but only the first 2 characters
	if(0==strncmp(szCmd,"at",2)){
		iTerminalCommandCode=TERM_AT;
	}

	return iTerminalCommandCode;
} /** Term_MatchTerminalCommand */



/*****************************************************************************/
/*! Term_PrintHelp
 *   Displays a list of supported commands                                    */
/*****************************************************************************/
static void Term_PrintHelp(void)
{
	int i=0;
	for(i=0; i<(TERM_LAST-1); i++){
		printf("%s\r\n", g_tTerminalCommands[i].szString);
	}
} /** Term_PrintHelp */







/*****************************************************************************/
/*! Term_TerminalCommandHandler
 *   \param ptAppData pointer to APP_DATA_T structure
 *   \param uiTerminalCommandCode terminal command code
 *   \param iArgc
 *   \param uiArgv                                                            */
/*****************************************************************************/
void Term_TerminalCommandHandler(int uiTerminalCommandCode, int argc, char** argv )
{
	int i;
	uint32_t ulHxResult;
	char szFloat[10];
	char szFloat2[10];
	RB_DATA_T* ptLogData;
	APP_RETURN_E eReturn = APP_OK;
	int iParam=0;
	uint32_t ulAdcValue;
	uint32_t ulBatVoltage;


	HX711_STATE_E eRslt=HX711_OK;
	HAL_StatusTypeDef tRes=HAL_OK;
	GPIO_InitTypeDef GPIO_InitStruct = {0};


	switch(uiTerminalCommandCode){

	case TERM_HELP:
		Term_PrintHelp();
		break;

	case TERM_TARA:
		eRslt=hx711_measure(&ulHxResult);
		if(eRslt==HX711_OK){
			hx711_setTara(ulHxResult);
			g_tAppData.tRemanentData.ulTara=ulHxResult;
		}
		break;

	case TERM_CALI:
		eRslt=hx711_measure(&ulHxResult);
		if(eRslt==HX711_OK){
			hx711_setCali(ulHxResult);
			g_tAppData.tRemanentData.ulCali=ulHxResult;
		}
		break;


	case TERM_HX:
		HAL_RTC_GetTime(g_tAppData.phrtc, &g_tAppData.tTime, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(g_tAppData.phrtc, &g_tAppData.tDate, RTC_FORMAT_BIN);

		//in case of date change (1st measurement after midnight) discard buffer
		if(g_tAppData.tTime.Hours < g_tAppData.bLastMeasureHours){
			Rb_BufferClear();
		}
		g_tAppData.bLastMeasureHours=g_tAppData.tTime.Hours; // remember hour of last measurement in order to detect date change (pass midnight)


		eRslt=hx711_measure(&ulHxResult);

		switch(eRslt){
		case HX711_OK:
		case HX711_FILTER_ACTIVE:
			Rb_DataPut(&g_tAppData.tTime, hx711_gramm(ulHxResult));
			printf("HX711: %d " APP_EOL,(int)ulHxResult);
			hx711_string(hx711_gramm(ulHxResult), szFloat);
			printf("HX711: %s " APP_EOL,szFloat);
			break;

		default:
		case HX711_FILTER_ERROR:
			printf("Filter Error" APP_EOL);
			break;
		}
		break;




	case TERM_BUFCLR:
		Rb_BufferClear();
		break;

	case TERM_BUFINFO:
		printf("ringbuffer %d" APP_EOL, Rb_BufferInfo());
		break;

	case TERM_BUFLIST:
		HAL_RTC_GetTime(g_tAppData.phrtc, &g_tAppData.tTime, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(g_tAppData.phrtc, &g_tAppData.tDate, RTC_FORMAT_BIN);

		Rb_StartGlance();
		while((ptLogData=Rb_DataGlance())){
			hx711_string(ptLogData->ulWeight, szFloat);
			printf("timestamp=%04d-%02d-%02dT%02d:%02d:00 value=%s" APP_EOL, g_tAppData.tDate.Year+2000,g_tAppData.tDate.Month, g_tAppData.tDate.Date, (int)(ptLogData->usTime>>8), (int)(ptLogData->usTime&0x00ff), szFloat);
		}
		break;


	case TERM_BUFFLUSH:
		HAL_RTC_GetTime(g_tAppData.phrtc, &g_tAppData.tTime, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(g_tAppData.phrtc, &g_tAppData.tDate, RTC_FORMAT_BIN);

		while((ptLogData=Rb_DataGet())){
			hx711_string(ptLogData->ulWeight, szFloat);
			printf("timestamp=%04d-%02d-%02dT%02d:%02d:00 value=%s" APP_EOL, g_tAppData.tDate.Year+2000,g_tAppData.tDate.Month, g_tAppData.tDate.Date, (int)(ptLogData->usTime>>8), (int)(ptLogData->usTime&0x00ff), szFloat);
		}
		break;



	case TERM_BLINK_ON:
		g_tAppData.fBlink=1;
		break;

	case TERM_BLINK_OFF:
		g_tAppData.fBlink=0;
		HAL_GPIO_WritePin(LED_GPIO_Port,  LED_Pin, GPIO_PIN_RESET);
		break;



	case TERM_RTC:
		HAL_RTC_GetTime(g_tAppData.phrtc, &g_tAppData.tTime, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(g_tAppData.phrtc, &g_tAppData.tDate, RTC_FORMAT_BIN);
		printf("%04d-%02d-%02dT%02d:%02d:%02d\n\r", g_tAppData.tDate.Year+2000,g_tAppData.tDate.Month, g_tAppData.tDate.Date, g_tAppData.tTime.Hours, g_tAppData.tTime.Minutes, g_tAppData.tTime.Seconds);
		break;

	case TERM_ALARM:
		HAL_RTC_GetAlarm(g_tAppData.phrtc, &g_tAppData.tAlarm, RTC_ALARM_A, RTC_FORMAT_BIN);
		printf("ALARM A: %02d:%02d:%02d\n\r", g_tAppData.tAlarm.AlarmTime.Hours, g_tAppData.tAlarm.AlarmTime.Minutes, g_tAppData.tAlarm.AlarmTime.Seconds);
		break;

	case TERM_REMLOAD:
		Rd_Load((uint8_t*)&g_tAppData.tRemanentData, sizeof(g_tAppData.tRemanentData));
		App_Hx711Init();
		App_AlarmInit();
		break;

	case TERM_REMINFO:
		printf("APN:       %s" APP_EOL,g_tAppData.tRemanentData.szApn);
		printf("URL:       %s" APP_EOL,g_tAppData.tRemanentData.szPrvUrl);
		printf("tara:      %d" APP_EOL,(int)g_tAppData.tRemanentData.ulTara);
		printf("cali:      %d" APP_EOL,(int)g_tAppData.tRemanentData.ulCali);
		printf("AdcLow:    %d" APP_EOL,(int)g_tAppData.tRemanentData.ulAdcLow);
		printf("AdcHigh:   %d" APP_EOL,(int)g_tAppData.tRemanentData.ulAdcHigh);
		printf("VoltLow:   %d" APP_EOL,(int)g_tAppData.tRemanentData.ulVoltLow);
		printf("VoltHigh:  %d" APP_EOL,(int)g_tAppData.tRemanentData.ulVoltHigh);
		printf("WKUP:      %d" APP_EOL,(int)g_tAppData.tRemanentData.ulWakeupPeriod);
		printf("ALARM:     %02d:%02d" APP_EOL,(int)g_tAppData.tRemanentData.bAlarmHrs,(int)g_tAppData.tRemanentData.bAlarmMin);
		printf("ALARM INC: %d" APP_EOL,(int)g_tAppData.tRemanentData.bUploadHoursIncrement);
		break;

	case TERM_REMSTORE:
		Rd_Store((uint8_t*)&g_tAppData.tRemanentData, sizeof(g_tAppData.tRemanentData));
		break;

	case TERM_STARTAPP:
		g_tAppData.iSystemStateNext=APP_SYSTEM_STOP;
		break;

	case TERM_STOP:
		HAL_TIM_Base_Stop_IT(g_tAppData.phtim2);
		HAL_SuspendTick();

		memset(&GPIO_InitStruct,0,sizeof(GPIO_InitStruct));
		GPIO_InitStruct.Pin = VCP_RX_Pin|VCP_TX_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(VCP_RX_GPIO_Port, &GPIO_InitStruct);
		// clear alarm and wakeup flags
		__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
		__HAL_RTC_CLEAR_FLAG(RTC_ISR_WUTF);
		__HAL_RTC_CLEAR_FLAG(RTC_ISR_ALRAF);
		__HAL_GPIO_EXTI_CLEAR_FLAG(UI_INPUT_Pin);
		HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
		// now wait for wakeup interrupt

		// OK woke up again ...
		SystemClock_Config();
		HAL_UART_MspInit(g_tAppData.tUart[APP_UART2].phUart);
		HAL_ResumeTick();
		tRes=HAL_TIM_Base_Start_IT(g_tAppData.phtim2);
		if(tRes!=HAL_OK){
			g_tAppData.iErrorCnt++;
		}
		break;



	case TERM_SIM800_ON:
		g_tAppData.iBlinkPeriod=2; // increase blink frequency to indicate modem activity
		// set UART pins from analog (save energy) to active push/pull mode
		HAL_UART_MspInit(g_tAppData.tUart[APP_LPUART1].phUart);
		// start waiting for incoming data -> RX event will be handled in callback function
		Uh_UartReset(APP_LPUART1);

		HAL_GPIO_WritePin(SIM800_ON_GPIO_Port, SIM800_ON_Pin, GPIO_PIN_RESET); // switch on power of SIM800
		HAL_Delay(50000);

#if(0)
		while(UART_RX_DST != Uh_UartRxWait(APP_LPUART1, 20000)){
			HAL_UART_Transmit(g_tAppData.tUart[APP_UART2].phUart, g_tAppData.tUart[APP_LPUART1].abDataBuffer,g_tAppData.tUart[APP_LPUART1].iDataLen,2000);
		}
		while(UART_RX_DST != Uh_UartRxWait(APP_LPUART1, 20000)){
			HAL_UART_Transmit(g_tAppData.tUart[APP_UART2].phUart, g_tAppData.tUart[APP_LPUART1].abDataBuffer,g_tAppData.tUart[APP_LPUART1].iDataLen,2000);
		}
#endif
		printf("SIM800 ON" APP_EOL);


		break;

	case TERM_SIM800_OFF:
		g_tAppData.iBlinkPeriod=5; // set default blink frquency
		HAL_GPIO_WritePin(SIM800_ON_GPIO_Port, SIM800_ON_Pin, GPIO_PIN_SET); // switch SIM800 power off
		// set UART pins to analog mode in order to save energy
		memset(&GPIO_InitStruct,0,sizeof(GPIO_InitStruct));
		GPIO_InitStruct.Pin = UART_TX_SIM800_Pin|UART_RX_SIM800_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(UART_TX_SIM800_GPIO_Port, &GPIO_InitStruct);
		break;

	case TERM_AT:
		strcat(argv[0],"\n");
		HAL_UART_Transmit(g_tAppData.tUart[APP_LPUART1].phUart,(uint8_t*) argv[0], (uint16_t)strlen(argv[0]),SIM800_TIMEOUT);
		break;



		case TERM_PUSH:
			getAdcValue(&ulAdcValue, &ulBatVoltage);
			hx711_string(ulBatVoltage, szFloat2);

			HAL_RTC_GetTime(g_tAppData.phrtc, &g_tAppData.tTime, RTC_FORMAT_BIN);
			HAL_RTC_GetDate(g_tAppData.phrtc, &g_tAppData.tDate, RTC_FORMAT_BIN);

			for(i=0;i<8;i++){

				switch(i){
				case 2:
					sprintf((char*)g_tAppData.tUart[APP_LPUART1].abTxBuffer, "%s%s\"\r", pCmd2, g_tAppData.tRemanentData.szApn);
					pCmd[i]=(char*)g_tAppData.tUart[APP_LPUART1].abTxBuffer;
					break;
				case 6:
					sprintf((char*)g_tAppData.tUart[APP_LPUART1].abTxBuffer, "%s%s\"\r", pCmd6, g_tAppData.tRemanentData.szPrvUrl);
					pCmd[i]=(char*)g_tAppData.tUart[APP_LPUART1].abTxBuffer;
					break;
				default:
					break;
				}



				HAL_UART_Transmit(g_tAppData.tUart[APP_LPUART1].phUart,  (uint8_t*)pCmd[i], strlen(pCmd[i]),SIM800_TIMEOUT);
				if(UART_RX_TIMEOUT != Uh_UartRxWait(APP_LPUART1, 5000)){
					HAL_UART_Transmit(g_tAppData.tUart[APP_UART2].phUart, g_tAppData.tUart[APP_LPUART1].abDataBuffer,g_tAppData.tUart[APP_LPUART1].iDataLen,2000);
				}
				else {
					printf("timeout %s\r\n",pCmd[i]);
				}
			}


			// loop over complete ring buffer
			while((ptLogData=Rb_DataGet())){

				hx711_string(ptLogData->ulWeight, szFloat);
				sprintf((char*)g_tAppData.tUart[APP_LPUART1].abTxBuffer, "%04d-%02d-%02dT%02d:%02d:00,%s,%s\n", g_tAppData.tDate.Year+2000,g_tAppData.tDate.Month, g_tAppData.tDate.Date, (int)(ptLogData->usTime>>8), (int)(ptLogData->usTime&0x00ff), szFloat, szFloat2);
				HAL_UART_Transmit(g_tAppData.tUart[APP_LPUART1].phUart,  g_tAppData.tUart[APP_LPUART1].abTxBuffer, strlen((char*)g_tAppData.tUart[APP_LPUART1].abTxBuffer),SIM800_TIMEOUT);

			}
			// ring buffer loop end

			//expect OK
			if(UART_RX_TIMEOUT != Uh_UartRxWait(APP_LPUART1, 25000)){
				HAL_UART_Transmit(g_tAppData.tUart[APP_UART2].phUart, g_tAppData.tUart[APP_LPUART1].abDataBuffer,g_tAppData.tUart[APP_LPUART1].iDataLen,2000);
			}
			else {
				printf("timeout\r\n");
			}



			HAL_UART_Transmit(g_tAppData.tUart[APP_LPUART1].phUart,  (uint8_t*)pCmd[8], strlen(pCmd[8]),SIM800_TIMEOUT);
			if(UART_RX_TIMEOUT != Uh_UartRxWait(APP_LPUART1, 5000)){
				HAL_UART_Transmit(g_tAppData.tUart[APP_UART2].phUart, g_tAppData.tUart[APP_LPUART1].abDataBuffer,g_tAppData.tUart[APP_LPUART1].iDataLen,2000);
			}
			else {
				printf("timeout %s\r\n",pCmd[8]);
			}

			if(UART_RX_TIMEOUT != Uh_UartRxWait(APP_LPUART1, 20000)){
				HAL_UART_Transmit(g_tAppData.tUart[APP_UART2].phUart, g_tAppData.tUart[APP_LPUART1].abDataBuffer,g_tAppData.tUart[APP_LPUART1].iDataLen,2000);
			}
			else {
				printf("timeout %s\r\n",pCmd[8]);
			}


			for(i=9;i<11;i++){
				HAL_UART_Transmit(g_tAppData.tUart[APP_LPUART1].phUart,  (uint8_t*)pCmd[i], strlen(pCmd[i]),SIM800_TIMEOUT);
				if(UART_RX_TIMEOUT != Uh_UartRxWait(APP_LPUART1, 5000)){
					HAL_UART_Transmit(g_tAppData.tUart[APP_UART2].phUart, g_tAppData.tUart[APP_LPUART1].abDataBuffer,g_tAppData.tUart[APP_LPUART1].iDataLen,2000);
				}
				else {
					printf("timeout %s\r\n",pCmd[i]);
				}
			}
			break;



		case TERM_SETPARAM:
			iParam=atoi(argv[1]);
			printf("set param %d to value %s " APP_EOL,iParam,argv[2]);
			switch(iParam){
			case PARAM_WAKEUP_PERIOD:
				g_tAppData.tRemanentData.ulWakeupPeriod=atoi(argv[2]);
				HAL_RTCEx_SetWakeUpTimer_IT(g_tAppData.phrtc, g_tAppData.tRemanentData.ulWakeupPeriod, RTC_WAKEUPCLOCK_CK_SPRE_16BITS);
				break;
			case PARAM_ALARM_HRS:
				g_tAppData.tRemanentData.bAlarmHrs=(uint8_t)atoi(argv[2]);
				g_tAppData.tAlarm.AlarmTime.Hours=g_tAppData.tRemanentData.bAlarmHrs;
				HAL_RTC_SetAlarm_IT(g_tAppData.phrtc, &g_tAppData.tAlarm, RTC_FORMAT_BIN);
				break;
			case PARAM_ALARM_MIN:
				g_tAppData.tRemanentData.bAlarmMin=(uint8_t)atoi(argv[2]);
				g_tAppData.tAlarm.AlarmTime.Minutes=g_tAppData.tRemanentData.bAlarmMin;
				HAL_RTC_SetAlarm_IT(g_tAppData.phrtc, &g_tAppData.tAlarm, RTC_FORMAT_BIN);
				break;
			case PARAM_ALARM_INCREMENT:
				g_tAppData.tRemanentData.bUploadHoursIncrement=(uint8_t)atoi(argv[2]);
				break;
			case PARAM_TARA:
				g_tAppData.tRemanentData.ulTara=(uint32_t)atoi(argv[2]);
				break;
			case PARAM_CALI:
				g_tAppData.tRemanentData.ulCali=(uint32_t)atoi(argv[2]);
				break;
			case PARAM_ADC_LOW:
				g_tAppData.tRemanentData.ulAdcLow=(uint32_t)atoi(argv[2]);
				break;
			case PARAM_ADC_HIGH:
				g_tAppData.tRemanentData.ulAdcHigh=(uint32_t)atoi(argv[2]);
				break;
			case PARAM_VOLT_LOW:
				g_tAppData.tRemanentData.ulVoltLow=(uint32_t)atoi(argv[2]);
				break;
			case PARAM_VOLT_HIGH:
				g_tAppData.tRemanentData.ulVoltHigh=(uint32_t)atoi(argv[2]);
				break;
			case PARAM_APN:
				strncpy(g_tAppData.tRemanentData.szApn,argv[2],sizeof(g_tAppData.tRemanentData.szApn));
				break;
			case PARAM_URL:
				strncpy(g_tAppData.tRemanentData.szPrvUrl,argv[2],sizeof(g_tAppData.tRemanentData.szPrvUrl));
				break;
			default:
				break;
			}
			break;


			case TERM_SETTIME:
				Uh_UartRxWait(APP_LPUART1, 0);
				HAL_UART_Transmit(g_tAppData.tUart[APP_LPUART1].phUart,  (uint8_t*)"ate0\r", strlen("ate0\r"),SIM800_TIMEOUT);
				Uh_UartRxWait(APP_LPUART1, 10000);
				HAL_UART_Transmit(g_tAppData.tUart[APP_LPUART1].phUart,  (uint8_t*)"at+cclk?\r", strlen("at+cclk?\r"),SIM800_TIMEOUT);
				Uh_UartRxWait(APP_LPUART1, 10000);

				// now process the received data from SIM800
				eReturn=setTimeDate((char*)g_tAppData.tUart[APP_LPUART1].abDataBuffer, &g_tAppData.tTime, &g_tAppData.tDate);
				if(eReturn != APP_OK){
					App_ErrorHandler();
				}
				HAL_RTC_SetTime(g_tAppData.phrtc, &g_tAppData.tTime, RTC_FORMAT_BIN);
				HAL_RTC_SetDate(g_tAppData.phrtc, &g_tAppData.tDate, RTC_FORMAT_BIN);
				break;




			case TERM_BAT:
				getAdcValue(&ulAdcValue, &ulBatVoltage);
				hx711_string(ulBatVoltage, szFloat);

				printf("ADC %d = BatVol: %sV " APP_EOL,(int)ulAdcValue, szFloat);
				break;

			case TERM_BAT_LOW:
				getAdcValue(&ulAdcValue, &ulBatVoltage);
				g_tAppData.tRemanentData.ulAdcLow=ulAdcValue;
				break;

			case TERM_BAT_HIGH:
				getAdcValue(&ulAdcValue, &ulBatVoltage);
				g_tAppData.tRemanentData.ulAdcHigh=ulAdcValue;
				break;



			case TERM_UNKNOWN:
				printf("unknown"APP_EOL);
				break;

			default:
				//				printf("unhandled"APP_EOL);
				break;
	}

} /** Term_TerminalCommandHandler */




void terminalHandler(uint8_t* abBuffer, int iLength){
	int i;
	int argc=0;
	char* argv[APP_MAX_TERM_PARAM];

	//assumption: received command in abBuffer is terminated by 2 charaters "\r\n" from UART communication
	if(iLength<2) {
		Error_Handler();
	}
	else {
		abBuffer[iLength-2]=0; // truncate terminating 2 characters \r\n from terminal command and replace them by string terminating \0
	}

	argc=0;
	argv[0]=(char*)&abBuffer[0];
	i=0;
	while(abBuffer[i] != 0 && argc<APP_MAX_TERM_PARAM){
		if(abBuffer[i]==' '){
			abBuffer[i]=0; // replace whitespace by string terminating \0
			argc++;
			argv[argc]=(char*)&abBuffer[i+1];
		}
		i++;
	}
	if(i>0){//if command string has more than zero characters (i.e. is not empty) , increase number of arguments
		argc++;
	}


	Term_TerminalCommandHandler(Term_MatchTerminalCommand(argv[0]), argc, argv);
}


