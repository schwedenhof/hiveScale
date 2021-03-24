/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 *
 * This software is published under GNU General Public License v3.0.
 * It is hosted on github: https://github.com/schwedenhof/hiveScale
 *
 * Author: Drk Fischer, www.schwedenhof.net
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "printf.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "remanentDataHandler.h"
#include "terminalHandler.h"
#include "uartHandler.h"
#include "ringBuffer.h"
#include "hx711.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc;

UART_HandleTypeDef hlpuart1;
UART_HandleTypeDef huart2;

RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim21;

/* USER CODE BEGIN PV */
APP_DATA_T g_tAppData={0};


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_LPUART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_RTC_Init(void);
static void MX_TIM2_Init(void);
static void MX_ADC_Init(void);
static void MX_TIM21_Init(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */




void _putchar(char character){

	HAL_UART_Transmit(g_tAppData.tUart[APP_UART2].phUart, (uint8_t*) &character, (uint16_t)1,FASTUART_TIMEOUT);

}




void App_FlashLed(int iLedFlashCnt,uint32_t ulFlashPeriod){
	while(iLedFlashCnt--){
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		HAL_Delay(ulFlashPeriod);
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		HAL_Delay(ulFlashPeriod);
	}
}



void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin==UI_INPUT_Pin){
		if(g_tAppData.iSystemStateNext!=APP_SYSTEM_RUN){
			g_tAppData.iSystemStateNext=APP_SYSTEM_WAKEUP;
		}
	}
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef* hrtc){
	SystemClock_Config();
	g_tAppData.iSystemStateNext=APP_SYSTEM_ALARM;
}


// TIMEBASE for button and LED handling
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *phtim){

	if(phtim==&htim21){
		// LPUART1 RX timeout
		HAL_TIM_Base_Stop_IT(&htim21); // stop timeout timer
		HAL_GPIO_TogglePin(LED_GPIO_Port,  LED_Pin);
		HAL_GPIO_TogglePin(LED_GPIO_Port,  LED_Pin);
		if(g_tAppData.tUart[APP_LPUART1].fRxMsg == UART_RX_ONGOING){
			g_tAppData.tUart[APP_LPUART1].fRxMsg=UART_RX_ENDOFFRAME;
		}
	}


	if(phtim==&htim2){
		// time base for user interface button and LED
		g_tAppData.ulTimTick++;

		if(!(g_tAppData.ulTimTick%g_tAppData.iBlinkPeriod)){
			if(g_tAppData.fBlink){
				HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
			}

			// check user interface input signal
			if(GPIO_PIN_RESET == HAL_GPIO_ReadPin(UI_INPUT_GPIO_Port, UI_INPUT_Pin)){
				// active input signal detected
				switch (g_tAppData.iCmdState){

				case APP_CMD_STATE_IDLE: // start, first active input detected
					g_tAppData.iCmd=1;
					g_tAppData.iCmdState=APP_CMD_STATE_ACTIVE;
					break;

				case APP_CMD_STATE_ACTIVE: // ongoing active input state
					g_tAppData.iCmd++;
					break;

				case APP_CMD_STATE_END: // new active phase, but last cmd not acknowledged yet
					// do nothing
					break;

				default: // should never happen
					g_tAppData.iCmdState=APP_CMD_STATE_IDLE; // reset state machine
					g_tAppData.iCmd=0; // reset cmd code
					break;
				}
			}
		}
		else {
			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
		}

		if(GPIO_PIN_SET == HAL_GPIO_ReadPin(UI_INPUT_GPIO_Port, UI_INPUT_Pin)) { // inactive input
			switch (g_tAppData.iCmdState){

			case APP_CMD_STATE_IDLE: // idle wait for first active signal
				g_tAppData.iCmd=APP_CMD_STATE_IDLE; // reset cmd code
				break;

			case APP_CMD_STATE_ACTIVE: // end, first inactive state after active phase
				g_tAppData.iCmdState=APP_CMD_STATE_END;
				break;

			case APP_CMD_STATE_END: // idle, cmd not executed yet, wait for command acknowledge
				break;


			default:  // should never happen
				g_tAppData.iCmdState=APP_CMD_STATE_IDLE; // reset state machine
				g_tAppData.iCmd=0; // reset cmd code
				break;
			}
		}
	}
}






static void App_Init(void){
	g_tAppData.tUart[APP_LPUART1].phUart=&hlpuart1;
	g_tAppData.tUart[APP_UART2].phUart=&huart2;
	g_tAppData.phtim2=&htim2;
	g_tAppData.phtim21=&htim21;
	g_tAppData.phrtc=&hrtc;
	g_tAppData.phadc=&hadc;

	HAL_RTC_GetAlarm(g_tAppData.phrtc, &g_tAppData.tAlarm, RTC_ALARM_A, RTC_FORMAT_BIN);


	g_tAppData.iErrorCnt=0;

	g_tAppData.iSystemState=APP_SYSTEM_STOP;
	g_tAppData.iSystemStateNext=APP_SYSTEM_STOP;

	g_tAppData.fBlink=1;
	g_tAppData.iBlinkPeriod=5;

	g_tAppData.iCmd=0;
	g_tAppData.iCmdState=APP_CMD_STATE_IDLE;


	Rb_BufferClear();

	terminalInitCmdList();
}







void App_Hx711Init(void){
	HX711_CONFIG_T tHx711Config;

	tHx711Config.eGain=HX711_GAIN_128;
	tHx711Config.iIgnoreBits=8;
	tHx711Config.iArraySize=1;
	tHx711Config.iFilterDeviation=40;
	tHx711Config.iMeasurementPause=50;
	tHx711Config.ulTara=g_tAppData.tRemanentData.ulTara;
	tHx711Config.ulCali=g_tAppData.tRemanentData.ulCali;
	hx711_init(&tHx711Config);

}



void App_AlarmInit(void){
	HAL_RTCEx_SetWakeUpTimer_IT(g_tAppData.phrtc, g_tAppData.tRemanentData.ulWakeupPeriod, RTC_WAKEUPCLOCK_CK_SPRE_16BITS);
	g_tAppData.tAlarm.AlarmTime.Hours=g_tAppData.tRemanentData.bAlarmHrs;
	g_tAppData.tAlarm.AlarmTime.Minutes=g_tAppData.tRemanentData.bAlarmMin;
	HAL_RTC_SetAlarm_IT(g_tAppData.phrtc, &g_tAppData.tAlarm, RTC_FORMAT_BIN);
}


APP_RETURN_E App_Sim800On(void){
	return APP_OK;
}


void App_ErrorHandler(void){
	while(1){
	}
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	HAL_StatusTypeDef eStat=HAL_OK;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_LPUART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_ADC_Init();
  MX_TIM21_Init();
  /* USER CODE BEGIN 2 */


	MX_RTC_Init();
#if(0)
	/*##-Check if Data stored in BackUp register1: No Need to reconfigure RTC if flag is set#*/
	/* Read the Back Up Register 0 Data */
	if (!((HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0)&BKUP_FLAG_RTC))){
		/* Configure RTC Calendar only if flag not set*/
		MX_RTC_Init();
	}
	else {
		hrtc.Instance = RTC;
		hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
		hrtc.Init.AsynchPrediv = 127;
		hrtc.Init.SynchPrediv = 255;
		hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
		hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
		hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
		hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
		if (HAL_RTC_Init(&hrtc) != HAL_OK)
		{
			Error_Handler();
		}
	}
#endif

	App_Init();
	Term_TerminalCommandHandler(TERM_SIM800_OFF,0,0);

	// start cyclic timer for user interface button and LED -> reload event is handled in callback function
	HAL_TIM_Base_Start_IT(&htim2);
	printf("Hello hiveScale" APP_EOL);

	// start waiting for incoming data -> RX event will be handled in callback function
	HAL_UART_Receive_IT(g_tAppData.tUart[APP_UART2].phUart, &g_tAppData.tUart[APP_UART2].abRxBuffer[g_tAppData.tUart[APP_UART2].iRxIdx], 1);

	Rd_Load((uint8_t*)&g_tAppData.tRemanentData, sizeof(g_tAppData.tRemanentData));
	App_Hx711Init();
	App_AlarmInit();

	// clear wakeup interrupt flag in order to enter standby mode again
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
	__HAL_RTC_CLEAR_FLAG(RTC_ISR_ALRAF);

	// start/stop time once
	// not sure why this is necessary, has to be checked
	HAL_TIM_Base_Start_IT(&htim21); // start UART timeout timer
	HAL_TIM_Base_Stop_IT(&htim21); // stop UART timeout timer
	__HAL_TIM_SET_COUNTER(&htim21,0);


#ifdef DEBUG
	g_tAppData.iSystemStateNext=APP_SYSTEM_RUN;
#else
	Term_TerminalCommandHandler(TERM_SIM800_ON,0,0);
	Term_TerminalCommandHandler(TERM_SETTIME,0,0);
	Term_TerminalCommandHandler(TERM_SIM800_OFF,0,0);
#endif



	HAL_RTC_GetTime(g_tAppData.phrtc, &g_tAppData.tTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(g_tAppData.phrtc, &g_tAppData.tDate, RTC_FORMAT_BIN);
	printf("%04d-%02d-%02d %02d:%02d:%02d" APP_EOL, g_tAppData.tDate.Year+2000, g_tAppData.tDate.Month, g_tAppData.tDate.Date, g_tAppData.tTime.Hours, g_tAppData.tTime.Minutes, g_tAppData.tTime.Seconds);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while(1){

		g_tAppData.iSystemState=g_tAppData.iSystemStateNext;

		switch(g_tAppData.iSystemState){

		case APP_SYSTEM_STOP:
			Term_TerminalCommandHandler(TERM_BLINK_OFF,0,0);
			Term_TerminalCommandHandler(TERM_SIM800_OFF,0,0);
			Term_TerminalCommandHandler(TERM_STOP,0,0);
			// now sleeping
			// ...
			// wakeup
			if(g_tAppData.iSystemState==g_tAppData.iSystemStateNext)
				g_tAppData.iSystemStateNext=APP_SYSTEM_MEASURE;
			break;

		case APP_SYSTEM_MEASURE:
			Term_TerminalCommandHandler(TERM_HX,0,0);
			if(g_tAppData.iSystemState==g_tAppData.iSystemStateNext)
				g_tAppData.iSystemStateNext=APP_SYSTEM_STOP;
			break;

		case APP_SYSTEM_WAKEUP:
			printf("runmode" APP_EOL);
			Term_TerminalCommandHandler(TERM_BLINK_ON,0,0);
			// start waiting for incoming data -> RX event will be handled in callback function
			Uh_UartReset(APP_UART2);
			g_tAppData.iSystemStateNext=APP_SYSTEM_RUN;
			break;

		case APP_SYSTEM_ALARM:
			HAL_RTC_GetTime(g_tAppData.phrtc, &g_tAppData.tTime, RTC_FORMAT_BIN);
			HAL_RTC_GetDate(g_tAppData.phrtc, &g_tAppData.tDate, RTC_FORMAT_BIN);
			printf("alarm at %04d-%02d-%02d %02d:%02d:%02d" APP_EOL, g_tAppData.tDate.Year+2000, g_tAppData.tDate.Month, g_tAppData.tDate.Date, g_tAppData.tTime.Hours, g_tAppData.tTime.Minutes, g_tAppData.tTime.Seconds);
			Term_TerminalCommandHandler(TERM_SIM800_ON,0,0);
			Term_TerminalCommandHandler(TERM_PUSH,0,0);
			Term_TerminalCommandHandler(TERM_SIM800_OFF,0,0);
			// set next alarm (increment alarm hour by defined value)
			g_tAppData.tAlarm.AlarmTime.Hours+=g_tAppData.tRemanentData.bUploadHoursIncrement;
			if(g_tAppData.tAlarm.AlarmTime.Hours>23)
				g_tAppData.tAlarm.AlarmTime.Hours-=24;

			HAL_RTC_SetAlarm_IT(g_tAppData.phrtc, &g_tAppData.tAlarm, RTC_FORMAT_BIN);



			if(g_tAppData.iSystemState==g_tAppData.iSystemStateNext)
				g_tAppData.iSystemStateNext=APP_SYSTEM_STOP;
			break;

		case APP_SYSTEM_RUN:
		default:
			if(g_tAppData.iCmdState==APP_CMD_STATE_END){
				switch(g_tAppData.iCmd){

				case 2: // 2 pulses
					Term_TerminalCommandHandler(TERM_STARTAPP,0,0);
					break;


				case 5: // 5 pulses
					Term_TerminalCommandHandler(TERM_TARA,0,0);
					break;

				case 6: // 6 pulses
					Term_TerminalCommandHandler(TERM_CALI,0,0);
					break;

				case 7:
					Term_TerminalCommandHandler(TERM_BAT_LOW,0,0);
					break;

				case 8:
					Term_TerminalCommandHandler(TERM_BAT_HIGH,0,0);
					break;


				case 9: // store remanent data
					Term_TerminalCommandHandler(TERM_REMSTORE,0,0);
					break;

				case 10: // test data upload to server
					Term_TerminalCommandHandler(TERM_HX,0,0);
					Term_TerminalCommandHandler(TERM_SIM800_ON,0,0);
					Term_TerminalCommandHandler(TERM_PUSH,0,0);
					Term_TerminalCommandHandler(TERM_SIM800_OFF,0,0);
					break;


				default :
					// unknown cmd code
					break;
				}
				g_tAppData.iCmdState=APP_CMD_STATE_IDLE;
			}



			if(UART_RX_NL==Uh_UartRxWait(APP_UART2, 0)){
				terminalHandler(g_tAppData.tUart[APP_UART2].abDataBuffer, g_tAppData.tUart[APP_UART2].iDataLen);
			}

			if(UART_RX_ONGOING != Uh_UartRxWait(APP_LPUART1, 0)){
				// send (echo) received data to fast usart
				eStat=HAL_UART_Transmit(g_tAppData.tUart[APP_UART2].phUart, g_tAppData.tUart[APP_LPUART1].abDataBuffer, g_tAppData.tUart[APP_LPUART1].iDataLen,FASTUART_TIMEOUT);
				if(eStat==HAL_BUSY){
					g_tAppData.iErrorCnt++;
				}
			}

			// keep SystemStateNext untouched, SystemStateNext might have changed in terminal handler
			break;

		}

#if(0)

		HAL_Delay(500);
		HAL_RTC_WaitForSynchro(&hrtc);
		HAL_RTC_GetTime(&hrtc, &tTime, RTC_FORMAT_BIN);
		tTime.Hours=13;
		tTime.Minutes=10;
		tTime.Seconds=0;
		HAL_RTC_SetTime(&hrtc,&tTime,RTC_FORMAT_BIN);

		HAL_RTC_GetAlarm(&hrtc,&tAlarm,RTC_ALARM_A,RTC_FORMAT_BIN);
		tAlarm.AlarmTime.Hours=13;
		tAlarm.AlarmTime.Minutes=12;
		HAL_RTC_SetAlarm(&hrtc,&tAlarm,RTC_FORMAT_BIN);

#endif


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV4;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_8;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_LPUART1
                              |RCC_PERIPHCLK_RTC;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_SYSCLK;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC_Init(void)
{

  /* USER CODE BEGIN ADC_Init 0 */

  /* USER CODE END ADC_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC_Init 1 */

  /* USER CODE END ADC_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc.Instance = ADC1;
  hadc.Init.OversamplingMode = DISABLE;
  hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.SamplingTime = ADC_SAMPLETIME_160CYCLES_5;
  hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ContinuousConvMode = DISABLE;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.DMAContinuousRequests = DISABLE;
  hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.LowPowerFrequencyMode = DISABLE;
  hadc.Init.LowPowerAutoPowerOff = DISABLE;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC_Init 2 */

  /* USER CODE END ADC_Init 2 */

}

/**
  * @brief LPUART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_LPUART1_UART_Init(void)
{

  /* USER CODE BEGIN LPUART1_Init 0 */

  /* USER CODE END LPUART1_Init 0 */

  /* USER CODE BEGIN LPUART1_Init 1 */

  /* USER CODE END LPUART1_Init 1 */
  hlpuart1.Instance = LPUART1;
  hlpuart1.Init.BaudRate = 19200;
  hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
  hlpuart1.Init.StopBits = UART_STOPBITS_1;
  hlpuart1.Init.Parity = UART_PARITY_NONE;
  hlpuart1.Init.Mode = UART_MODE_TX_RX;
  hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPUART1_Init 2 */

  /* USER CODE END LPUART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};
  RTC_AlarmTypeDef sAlarm = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0;
  sTime.Minutes = 0;
  sTime.Seconds = 0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_SATURDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 23;
  sDate.Year = 21;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  /** Enable the Alarm A
  */
  sAlarm.AlarmTime.Hours = 23;
  sAlarm.AlarmTime.Minutes = 55;
  sAlarm.AlarmTime.Seconds = 0;
  sAlarm.AlarmTime.SubSeconds = 0;
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
  sAlarm.AlarmDateWeekDay = RTC_WEEKDAY_SATURDAY;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  /** Enable the WakeUp
  */
  if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, MODULE_SLEEPTIME, RTC_WAKEUPCLOCK_CK_SPRE_16BITS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */


	/*##- SET Flag in RTC Backup data Register1 #######################*/
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0)|BKUP_FLAG_RTC);

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = TIM_PRESCALER_1MSEC_AT_8MHZ;
  htim2.Init.CounterMode = TIM_COUNTERMODE_DOWN;
  htim2.Init.Period = TIM_PERIOD_200MSEC;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV4;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM21 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM21_Init(void)
{

  /* USER CODE BEGIN TIM21_Init 0 */

  /* USER CODE END TIM21_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM21_Init 1 */

  /* USER CODE END TIM21_Init 1 */
  htim21.Instance = TIM21;
  htim21.Init.Prescaler = TIM_PRESCALER_1MSEC_AT_8MHZ;
  htim21.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim21.Init.Period = TIM_PERIOD_3MSEC;
  htim21.Init.ClockDivision = TIM_CLOCKDIVISION_DIV4;
  htim21.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim21) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim21, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim21, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM21_Init 2 */

  /* USER CODE END TIM21_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(HX711_SCK_GPIO_Port, HX711_SCK_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SIM800_ON_GPIO_Port, SIM800_ON_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA0 PA4 PA5 PA6
                           PA7 PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
                          |GPIO_PIN_7|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB3 PB4 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : DS1820_Pin */
  GPIO_InitStruct.Pin = DS1820_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DS1820_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : HX711_SCK_Pin */
  GPIO_InitStruct.Pin = HX711_SCK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(HX711_SCK_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : HX711_DOUT_Pin */
  GPIO_InitStruct.Pin = HX711_DOUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(HX711_DOUT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : UI_INPUT_Pin */
  GPIO_InitStruct.Pin = UI_INPUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(UI_INPUT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SIM800_ON_Pin LED_Pin */
  GPIO_InitStruct.Pin = SIM800_ON_Pin|LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
