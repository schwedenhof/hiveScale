/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  *
  * This software is published under GNU General Public License v3.0.
  * It is hosted on github: https://github.com/schwedenhof/hiveScale
  *
  * Author: Drk Fischer, www.schwedenhof.net
  *
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

#define APP_TX_BUFFER_LEN 160
#define UART_RX_BUFFER_LEN 160
#define APP_MAX_TERM_PARAM 5
#define APP_MAX_LOG_DATA_SETS 290  // 12 per hour = 5min interval -> >=288 entries
#define APP_WAKEUPTIME_MINUTES 5

#define APP_RD_MAX_APN_STRING_LEN 35
#define APP_RD_MAX_URL_STRING_LEN 120
#define APP_RD_MAX_KEY_STRING_LEN 20

#if(24*60/APP_WAKEUPTIME_MINUTES > APP_MAX_LOG_DATA_SETS)
#warning "ring buffer size too small"
#endif
enum {
	APP_LPUART1 = 0,
	APP_UART2
};


enum {
	APP_SYSTEM_RUN,
	APP_SYSTEM_MEASURE,
	APP_SYSTEM_WAKEUP,
	APP_SYSTEM_ALARM,
	APP_SYSTEM_STOP,
};

enum {
	APP_CMD_STATE_IDLE,
	APP_CMD_STATE_ACTIVE,
	APP_CMD_STATE_END,
};


typedef enum APP_RETURN_Et{
	APP_OK=0,
	APP_ERROR
} APP_RETURN_E;



typedef struct APP_REMANENT_DATA_Ttag{
	uint32_t ulTara;
	uint32_t ulCali;

	uint32_t ulWakeupPeriod;

	uint8_t bAlarmHrs;
	uint8_t bAlarmMin;
	uint8_t bUploadHoursIncrement;
	uint8_t bReserved;

	uint32_t ulAdcHigh;
	uint32_t ulAdcLow;
	uint32_t ulVoltLow;
	uint32_t ulVoltHigh;

	char szApn[APP_RD_MAX_APN_STRING_LEN];
	char szPrvUrl[APP_RD_MAX_URL_STRING_LEN];

} APP_REMANENT_DATA_T;




typedef struct APP_DATA_Ttag{
	struct {
		UART_HandleTypeDef* phUart;          // pointer to HAL UART driver handle structure
		int iTxIdx;                          // length of TX message
		int iRxIdx;                          // current RX byte index in receive buffer
		int fRxMsg;                          // flag which indicates end of message (reception of APP_NL)
		int iDataLen;
		uint8_t abDataBuffer[UART_RX_BUFFER_LEN];
		uint8_t abTxBuffer[APP_TX_BUFFER_LEN];
		uint8_t abRxBuffer[UART_RX_BUFFER_LEN];
	} tUart[2];

	int fBlink;
	int iBlinkPeriod;
	int iCmd;
	int iCmdState;
	int iSystemState;
	int iSystemStateNext;
	int iErrorCnt;
	uint8_t bLastMeasureHours;

	RTC_HandleTypeDef* phrtc;
	TIM_HandleTypeDef* phtim2;
	TIM_HandleTypeDef* phtim21;
	ADC_HandleTypeDef* phadc;
//	SPI_HandleTypeDef* phspi;

	RTC_TimeTypeDef tTime;
	RTC_DateTypeDef tDate;
	RTC_AlarmTypeDef tAlarm;

	uint32_t ulTimTick;

	APP_REMANENT_DATA_T tRemanentData;

} APP_DATA_T;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

void App_Hx711Init(void);
void App_AlarmInit(void);
void App_FlashLed(int iLedFlashCnt,uint32_t ulFlashPeriod);
void App_ErrorHandler(void);
APP_RETURN_E App_Sim800On(void);


void SystemClock_Config();
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define MODULE_SLEEPTIME 300
#define ALARM_MIN 2
#define ALARM_SEC 10
#define ALARM_HR 0
#define TIM_PRESCALER_100USEC_AT_8MHZ 800
#define TIM_PRESCALER_10USEC_AT_8MHZ 80
#define TIM_PRESCALER_1MSEC_AT_8MHZ 8000
#define TIM_PERIOD_100MSEC 100
#define TIM_PERIOD_200MSEC 200
#define TIM_PERIOD_3MSEC 3
#define ADC_BAT_Pin GPIO_PIN_1
#define ADC_BAT_GPIO_Port GPIOA
#define UART_TX_SIM800_Pin GPIO_PIN_2
#define UART_TX_SIM800_GPIO_Port GPIOA
#define UART_RX_SIM800_Pin GPIO_PIN_3
#define UART_RX_SIM800_GPIO_Port GPIOA
#define DS1820_Pin GPIO_PIN_1
#define DS1820_GPIO_Port GPIOB
#define HX711_SCK_Pin GPIO_PIN_8
#define HX711_SCK_GPIO_Port GPIOA
#define VCP_TX_Pin GPIO_PIN_9
#define VCP_TX_GPIO_Port GPIOA
#define VCP_RX_Pin GPIO_PIN_10
#define VCP_RX_GPIO_Port GPIOA
#define HX711_DOUT_Pin GPIO_PIN_11
#define HX711_DOUT_GPIO_Port GPIOA
#define UI_INPUT_Pin GPIO_PIN_12
#define UI_INPUT_GPIO_Port GPIOA
#define UI_INPUT_EXTI_IRQn EXTI4_15_IRQn
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SIM800_ON_Pin GPIO_PIN_6
#define SIM800_ON_GPIO_Port GPIOB
#define LED_Pin GPIO_PIN_7
#define LED_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
#define APP_EOL "\r\n"
#define APP_NL '\n'
#define APP_CR '\r'
#define FASTUART_TIMEOUT 200
#define SIM800_TIMEOUT 2000
#define BKUP_FLAG_RTC (0x00000001U)
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
