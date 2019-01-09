/**
  ******************************************************************************
  * @file    stm32f7xx_it.c
  * @author  Ac6
  * @version V1.0
  * @date    02-Feb-2015
  * @brief   Default Interrupt Service Routines.
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "stm32f7xx.h"
#ifdef USE_RTOS_SYSTICK
#include <cmsis_os.h>
#endif
#include "stm32f7xx_it.h"
#include "cvc_can.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern CAN_HandleTypeDef	CanHandle;
extern SPI_HandleTypeDef SpiHandle;

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef TIM_EXPBD_Handle;


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            	  	    Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles SysTick Handler, but only if no RTOS defines it.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
/*
#ifdef USE_RTOS_SYSTICK
	osSystickHandler();
#endif
*/
}



/**
* @brief This function handles TIM3 global interrupt.
*/
void TIM3_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&TIM_EXPBD_Handle);
}


/******************************************************************************/
/*            	  	   		 CAN Interrupt Handlers                            */
/******************************************************************************/
/**
  * @brief	This function handles CAN1 RX0 interrupt requests
  * @param	None
  * @retval	None
  */
void CANx_RX_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&CanHandle);
}


/******************************************************************************/
/*            	  	   		 SPI Interrupt Handlers                            */
/******************************************************************************/
/**
  * @brief  This function handles SPI interrupt request.
  * @param  None
  * @retval None
  */
void SPIx_IRQHandler(void)
{
  HAL_SPI_IRQHandler(&SpiHandle);
}
