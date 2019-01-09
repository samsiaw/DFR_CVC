/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx.h"
#include "stm32f7xx_nucleo_144.h"
#include "cvc_tasks.h"
#include "cvc_can.h"
#include "cvc_spi.h"
#include "demo.h"

/* PLC-specific includes -----------------------------------------------------*/
#include "x_nucleo_plc01a1.h"
#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_conf.h"
#include "stm32f7xx_hal_def.h"

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void CPU_CACHE_Enable(void);

/* Private Variables ----------------------------------------------------*/
uint8_t ubKeyNumber = 0x0;

/* Private PLC demo variables -----------------------------------------------*/
static uint8_t INPUT_READ_FLAG = 0;//Flag for reading input channel status
//static uint8_t ChannelsOn = 0x00;//Number of channels in on state
static uint16_t WD_Reset = 0;//Watchdog count (in ms)
static uint8_t Tick_10 = 0;//10ms count
static uint8_t* Input_Data;//Input channels state
static uint8_t oData = 0;
static uint8_t* Relay_Status;//Output relay status
static uint8_t* Current_Limiter_Status;//Input current limiter status
static uint32_t FreezeTime = 0;//Time in ms for which outpus to be freezed

/* Private PLC demo function prototypes --------------------------------------*/
static void initializePlc(void);
static void PLC_Handler(void);
uint8_t* CURRENT_LIMITER_Handler(void);
void RELAY_Handler(uint8_t*);


/**
  * @brief	 Main program
  * @param	 None
  * @retval	 None
  */
int main(void)
{
	/* Enable the CPU Cache */
	CPU_CACHE_Enable();

	/* STM32F7xx HAL library initialization */
	HAL_Init();

	/* Configure the system clock to 216 MHz */
	SystemClock_Config();

	/* PLC initialization */
	initializePlc();

	/* Reset relay at startup to avoid Fault */
	BSP_RELAY_Reset();

	/* Wait for 100 ms at startup */
	//while (WD_Reset < 100);
	HAL_Delay(120);


	/* Enable Relay Outputs */
	BSP_RELAY_EN_Out();


	/* Configure LED1 and LED3 */
/*	BSP_LED_Init(LED1);
	BSP_LED_Init(LED2);
	BSP_LED_Init(LED3);
*/
	/* Configure User push-button */
//	BSP_PB_Init(BUTTON_USER, BUTTON_MODE_GPIO);

	/* Initialize CAN */
//	CAN_Init();

	/* Initialize SPI */
/*	SPI_Config();

	SPI_outputs_vector.safety = 1;
	SPI_outputs_vector.ready_to_drive = 0;
	SPI_outputs_vector.rfg = 0;
	SPI_outputs_vector.ignition_kill = 1;
	SPI_outputs_vector.downshift_solenoid = 0;
	SPI_outputs_vector.upshift_solenoid = 0;
	SPI_Routine();
*/

	/* Create all tasks */
//	BaseType_t status = taskCreateAll();

	/* Start RTOS Scheduler */
//	vTaskStartScheduler();

	/* Function should never reach this point once scheduler is started */
	/* Infinite loop */
	while(1)
	{
		PLC_Handler();
	}

}



/**
* @brief  Initialize current limiter and relay.
*         Configures IOs and peripherals
* @param  None
* @retval None
*/
static void initializePlc(void)
{
  /* Initialize Relay and Current Limiter */
  BSP_Relay_Init();
  BSP_CurrentLimiter_Init();
}


/**
* @brief  Handles the relay and current limiter functions and demos
* @param  None
* @retval None
*/
static void PLC_Handler(void)
{
  if (INPUT_READ_FLAG)
  {
    INPUT_READ_FLAG = 0;

#ifdef OUTPUT_CYCLING
    /* Reset & set CS1 to refresh VNI watchdog */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
#else
    /* Handler for input current limiter */
    Input_Data = CURRENT_LIMITER_Handler();

    /* Handler for output relay */
    RELAY_Handler(Input_Data);
#endif /* OUTPUT_CYCLING */
  }
}


/**
  * @brief  Receive input data from Current Limiter and checks for error status
  * @param  None
  * @retval Pointer to input channels state
*/
uint8_t* CURRENT_LIMITER_Handler(void)
{
  static uint8_t* clData = NULL;

  clData = BSP_CURRENT_LIMITER_Read();

  Current_Limiter_Status = clData;

  if (BSP_GetCurrentLimiterStatus(Current_Limiter_Status) != CURRENT_LIMITER_OK)
  {
    /* Set input error code here */
  }

  return (clData+1);
}

/**
  * @brief  Select output control function and set outputs state
  * @param  Pointer to input channels state
  * @retval None
*/
void RELAY_Handler(uint8_t* iData)
{
  /* Uncomment the relevant function as required */

  /* Set Output same as input */
  oData = BSP_Signal_Mirror(*iData);
  /* Freeze selected outputs for a given time */
//  oData = BSP_Output_Freeze(*iData,0xFF,&FreezeTime);
  /* Regroup outputs */
//  oData = BSP_Output_Regroup(0xFF);
  /* Get the sum of input channels that are high */
//  ChannelsOn = BSP_Inputs_Sum(*iData); oData = 0x00;
  /* Set Outputs same as the required states */
//  oData = BSP_Output_ON(0xFF);
  /* Set Outputs same as required states */
//  oData = BSP_Output_OFF(0xFF);
  /* Set Outputs state according to the inputs state AND with required logic  */
//  oData = BSP_Inputs_AND(*iData,0x0F);
  /* Set Outputs state according to the inputs state OR with required logic */
//  oData = BSP_Inputs_OR(*iData,0x0F);
  /* Set Outputs state according to the inputs state NOT */
//  oData = BSP_Inputs_NOT(*iData);
  /* Set Outputs state according to the inputs state XOR */
//  oData = BSP_Inputs_XOR(*iData,0x00);

  Relay_Status = BSP_RELAY_SetOutputs(&oData);

  if (BSP_GetRelayStatus(Relay_Status) != RELAY_OK)
  {
    /* Set output error code here */
  }
}


/**
 * @brief  Systick callback implementation for systick interrupt
 * @param  None
 * @retval None
 */
void HAL_SYSTICK_Callback(void)
{
  if (WD_Reset <= 100)
    WD_Reset++;
  else
  {
    Tick_10++;
    if (Tick_10 == 10)
    {
      Tick_10 = 0;
      INPUT_READ_FLAG = 1;
    }
  }

  if (FreezeTime != 0)
    FreezeTime--;
}







/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 216000000
  *            HCLK(Hz)                       = 216000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 25
  *            PLL_N                          = 432
  *            PLL_P                          = 2
  *            PLL_Q                          = 9
  *            PLL_R                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 7
  * @param  None
  * @retval None
  */

static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  RCC_OscInitStruct.PLL.PLLR = 7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if(ret != HAL_OK)
  {
    Error_Handler();
  }

  /* Activate the OverDrive to reach the 216 MHz Frequency */
  ret = HAL_PWREx_EnableOverDrive();
  if(ret != HAL_OK)
  {
    Error_Handler();
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7);
  if(ret != HAL_OK)
  {
    Error_Handler();
  }

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  __SYSCFG_CLK_ENABLE();

}





/**
  * @brief  CPU L1-Cache enable.
  * @param  None
  * @retval None
  */
static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}
