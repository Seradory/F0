/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include <string.h>
#include "veri.h"
#include "pwm.h"
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
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim14;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM14_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
///timer 2 triac driver 25us 1 tick
///timer 3 pwm driver 25us 1 tick
///timer 14 on/off driver 1 ms 1 tick
__attribute__((section(".myCustomSection"))) int test_data[512];


uint8_t rx_Buffer[2048];
uint32_t Mesaj_id=0;
uint32_t Mesaj_boyu=0;
uint8_t HeaderOrData=0x01;

uint8_t Versiyon_Cevap[]= {0x00,0x00,0x00,0xf2,0x01,0x00,0xff,0xff};
uint8_t OK_NOK_Cevap[]= {0x00,0x00,0x00,0xf4,0x00,0x00,0x00,0x01};

uint8_t on_or_off_time=0x01;

uint32_t zero_cross_dedect=0;

uint8_t  test_seq=0x0;
uint16_t test_on_time=0;
uint16_t test_off_time=0;
uint32_t test_m=0;
uint32_t test_v=0;
uint16_t test_v_step=0;
////////MATRİS VERİLERİ


///////

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2)
  {
	  start_uart_timer();
	  if(HeaderOrData)
	  {
		  Mesaj_id=rx_Buffer[0]<<24 | rx_Buffer[1]<<16 | rx_Buffer[2]<<8 | rx_Buffer[3];
		  Mesaj_boyu=rx_Buffer[4]<<24 | rx_Buffer[5]<<16 | rx_Buffer[6]<<8 | rx_Buffer[7];

		  if(Mesaj_id==0xF1000000)
		  {
			  //HAL_TIM_Base_Start_IT(&htim6);
			  start_int_timer(&htim6,2000);
			  HeaderOrData=0x00;
			  memset(rx_Buffer,0,sizeof(rx_Buffer));
			  HAL_UART_Receive_IT(&huart2,rx_Buffer,1);

		  }
		  if(Mesaj_id==0xF3000000)
		  {
			  //HAL_TIM_Base_Start_IT(&htim6);
			  start_int_timer(&htim6,2000);
			  HeaderOrData=0x00;
			  memset(rx_Buffer,0,sizeof(rx_Buffer));
			  HAL_UART_Receive_IT(&huart2,rx_Buffer,1701); //50 1700 olacak.
		  }
		  if(Mesaj_id==0xF5000000)
		  {
			 // HAL_TIM_Base_Start_IT(&htim6);
			  start_int_timer(&htim6,2000);
			  HeaderOrData=0x00;
			  memset(rx_Buffer,0,sizeof(rx_Buffer));
			  HAL_UART_Receive_IT(&huart2,rx_Buffer,102); //doğru mesaj boylarını gir.
		  }

	  }
	  else //data
	  {
		  if(Mesaj_id==0xF1000000)
		  {
			  HAL_TIM_Base_Stop_IT(&htim6);
			  HeaderOrData=0x01;
			  memset(rx_Buffer,0,sizeof(rx_Buffer));
			  HAL_UART_Receive_IT(&huart2,rx_Buffer,8); // tekrardan header bekliyor
			  HAL_UART_Transmit(huart, Versiyon_Cevap, 8, 5000);
		  }

		  if(Mesaj_id==0xF3000000)
		  {
		      HAL_TIM_Base_Stop_IT(&htim6);
			  HeaderOrData=0x01;
			  //burda data atama işlemleri yapılacak
			  ///
			  memset(rx_Buffer,0,sizeof(rx_Buffer));
			  HAL_UART_Receive_IT(&huart2,rx_Buffer,8); // tekrardan header bekliyor

			  OK_NOK_Cevap[3]=0xf4;
			  HAL_UART_Transmit(huart, OK_NOK_Cevap, 8, 5000);
		  }

		  if(Mesaj_id==0xF5000000)
		  {

			  HAL_TIM_Base_Stop_IT(&htim6);

			  HeaderOrData=0x01;
			  //burda data atama işlemleri yapılacak
			  ///

			  HAL_UART_Receive_IT(&huart2,rx_Buffer,8); // tekrardan header bekliyor
			  OK_NOK_Cevap[3]=0xf6;
			  HAL_UART_Transmit(huart, OK_NOK_Cevap, 8, 5000);
			  if(rx_Buffer[0]!=0xff) // senaryo başlat ff gelirse durdur ve sistemi 0la. diğer türlü başlat.
			  {
				  if(test_seq==1)
				  {
					  Veri_Matrisi[0].V_Acis_Zaman=test_on_time;
					  Veri_Matrisi[0].V_Kapanis_Zaman=test_off_time;
					  Veri_Matrisi[0].M_deger=test_m;
					  Veri_Matrisi[0].V_deger=test_v;
					  Veri_Matrisi[0].M_deger=test_m;

				  }
				  start_int_timer(&htim14, Veri_Matrisi[Aktif_Indeks].V_Acis_Zaman);
				  enable_triac_int();
				  start_pwm_m(Veri_Matrisi[Aktif_Indeks].M_deger);
				  on_or_off_time=1; //Von da başlayacak.
			  }
			  else
			  {
				  HAL_TIM_Base_Stop_IT(&htim14);
				  disable_triac_int();
				  on_or_off_time=0; //bura kritik değil uarttan başlarken hep  vonda başlıyor nasılsa.
			  }
			  memset(rx_Buffer,0,sizeof(rx_Buffer));
		  }
	  }

  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6)		// uart protection
    {
    	HAL_TIM_Base_Stop_IT(&htim6);
    	HeaderOrData=0x01;
    	HAL_UART_DeInit(&huart2);
    	MX_USART2_UART_Init();
    	memset(rx_Buffer,0,sizeof(rx_Buffer));
    	//uint8_t* error_message="\r\nComm Error";
    	//HAL_UART_Transmit(&huart2, error_message, 12, 5000);
    	HAL_UART_Receive_IT(&huart2,rx_Buffer,8);

    }
    if (htim->Instance == TIM14)		//veri açık kapalı süresini ayarlar
    {
    	HAL_TIM_Base_Stop_IT(&htim14);
    	if(on_or_off_time==1)	//açık süre sonrası giriyor
    	{
        	///diğer timerleri m timer kapat
    		disable_triac_int();
    		stop_pwm_m();
    		on_or_off_time=0;
    		if(test_seq==1)
    		{
    			Veri_Matrisi[Aktif_Indeks].V_deger=Veri_Matrisi[Aktif_Indeks].V_deger+test_v_step;
    		}
    		start_int_timer(&htim14, Veri_Matrisi[Aktif_Indeks].V_Kapanis_Zaman);


    	}
    	else				//kapali süre sonrası giriyor
    	{
    		if(test_seq==0)
    		{
    			Aktif_Indeks++;
    		}
    		on_or_off_time=1;
    		start_int_timer(&htim14, Veri_Matrisi[Aktif_Indeks].V_Acis_Zaman);
    		start_pwm_m(Veri_Matrisi[Aktif_Indeks].M_deger);
    		enable_triac_int();

    	}

    }

    if (htim->Instance == TIM2)		// interrupt sonrası alfa kadar bekleyip triac pinini sürer
    {

    	HAL_TIM_Base_Stop_IT(&htim2);
    	HAL_GPIO_WritePin(triac_output_GPIO_Port, triac_output_Pin, GPIO_PIN_SET);

    	for(int i=0;i<3000;i++)
    	{

    	}
    	HAL_GPIO_WritePin(triac_output_GPIO_Port, triac_output_Pin, GPIO_PIN_RESET);
    	EXTI->PR=EXTI_PR_PR1;
    }



}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == GPIO_PIN_1)
    {
    	if(EXTI->RTSR==0x02) // rising edge int yakalandı
    	{

    	zero_cross_dedect++;
    	start_int_timer(&htim2, Veri_Matrisi[Aktif_Indeks].V_deger);


        // PC burda alfa counterini aktif et ve o kadar süre bekle. sonra alfa pininden pulse ver.
    	}
    	else  // falling edge yakalandı.
    	{

    	}
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
	///timer 2 triac driver 25us 1 tick
	///timer 3 pwm driver 25us 1 tick
	///timer 14 on/off driver 1 ms 1 tick
	memset(rx_Buffer,0,sizeof(rx_Buffer));
	////BRUAYI UARTTA KONFIG MESAJI İLE AYARLAYACAKSIN.
	Veri_Matrisi[0].V_Acis_Zaman=5000;
	Veri_Matrisi[0].V_Kapanis_Zaman=1000;
	Veri_Matrisi[0].M_deger=1000;
	Veri_Matrisi[0].V_deger=320;
	Veri_Matrisi[0].Aktif_Kapali=0x01;

	Veri_Matrisi[1].V_Acis_Zaman=5000;
	Veri_Matrisi[1].V_Kapanis_Zaman=1000;
	Veri_Matrisi[1].M_deger=500;
	Veri_Matrisi[1].V_deger=320;
	Veri_Matrisi[1].Aktif_Kapali=0x01;

	Veri_Matrisi[2].V_Acis_Zaman=5000;
	Veri_Matrisi[2].V_Kapanis_Zaman=1000;
	Veri_Matrisi[2].M_deger=250;
	Veri_Matrisi[2].V_deger=280;
	Veri_Matrisi[2].Aktif_Kapali=0x01;

	Veri_Matrisi[3].V_Acis_Zaman=5000;
	Veri_Matrisi[3].V_Kapanis_Zaman=1000;
	Veri_Matrisi[3].M_deger=125;
	Veri_Matrisi[3].V_deger=240;
	Veri_Matrisi[3].Aktif_Kapali=0x01;

	Veri_Matrisi[4].V_Acis_Zaman=5000;
	Veri_Matrisi[4].V_Kapanis_Zaman=1000;
	Veri_Matrisi[4].M_deger=100;
	Veri_Matrisi[4].V_deger=200;
	Veri_Matrisi[4].Aktif_Kapali=0x01;

	Veri_Matrisi[5].V_Acis_Zaman=5000;
	Veri_Matrisi[5].V_Kapanis_Zaman=1000;
	Veri_Matrisi[5].M_deger=75;
	Veri_Matrisi[5].V_deger=160;
	Veri_Matrisi[5].Aktif_Kapali=0x01;

	Veri_Matrisi[6].V_Acis_Zaman=5000;
	Veri_Matrisi[6].V_Kapanis_Zaman=1000;
	Veri_Matrisi[6].M_deger=20;
	Veri_Matrisi[6].V_deger=120;
	Veri_Matrisi[6].Aktif_Kapali=0x01;

	Veri_Matrisi[7].V_Acis_Zaman=5000;
	Veri_Matrisi[7].V_Kapanis_Zaman=1000;
	Veri_Matrisi[7].M_deger=10;
	Veri_Matrisi[7].V_deger=80;
	Veri_Matrisi[7].Aktif_Kapali=0x01;

	Veri_Matrisi[8].V_Acis_Zaman=5000;
	Veri_Matrisi[8].V_Kapanis_Zaman=1000;
	Veri_Matrisi[8].M_deger=8;
	Veri_Matrisi[8].V_deger=40;
	Veri_Matrisi[8].Aktif_Kapali=0x01;

	Veri_Matrisi[9].V_Acis_Zaman=5000;
	Veri_Matrisi[9].V_Kapanis_Zaman=1000;
	Veri_Matrisi[9].M_deger=4;
	Veri_Matrisi[9].V_deger=20;
	Veri_Matrisi[9].Aktif_Kapali=0x01;
	////


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
  MX_TIM3_Init();
  MX_USART2_UART_Init();
  MX_TIM14_Init();
  MX_TIM6_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart2,rx_Buffer,8);
  start_uart_timer();
 // HAL_TIM_Base_Start_IT(&htim6);
	//HAL_UART_Receive_IT(&huart2,rx_Buffer, sizeof(rx_Buffer));
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if(Aktif_Indeks==10)
	  {
		  Aktif_Indeks=0;
	  }
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
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
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 1199;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 360;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OnePulse_Init(&htim2, TIM_OPMODE_SINGLE) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 1199;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 100;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 38;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 47999;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 5000;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */
  stop_int_timer(&htim6);
  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM14 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM14_Init(void)
{

  /* USER CODE BEGIN TIM14_Init 0 */

  /* USER CODE END TIM14_Init 0 */

  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM14_Init 1 */

  /* USER CODE END TIM14_Init 1 */
  htim14.Instance = TIM14;
  htim14.Init.Prescaler = 47999;
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim14.Init.Period = 4000;
  htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OnePulse_Init(&htim14, TIM_OPMODE_SINGLE) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim14, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM14_Init 2 */

  /* USER CODE END TIM14_Init 2 */

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
  huart2.Init.BaudRate = 9600;
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
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(triac_output_GPIO_Port, triac_output_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : zero_cros_detector_Pin */
  GPIO_InitStruct.Pin = zero_cros_detector_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(zero_cros_detector_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : triac_output_Pin */
  GPIO_InitStruct.Pin = triac_output_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(triac_output_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
  HAL_NVIC_DisableIRQ(EXTI0_1_IRQn);
  EXTI->FTSR=0x00;
  EXTI->RTSR=0x02;
/* USER CODE END MX_GPIO_Init_2 */
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
