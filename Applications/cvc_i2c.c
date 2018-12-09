/*
 * cvc_i2c.c
 *
 *  Created on: Dec 8, 2018
 *      Author: f002ccx
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx.h"
#include "stm32f7xx_nucleo_144.h"
#include "cvc_i2c.h"

/* I2C handler declaration */
I2C_HandleTypeDef I2cHandle;

/* Buffer used for transmission */
uint8_t aTxBuffer[] = " ****I2C_TwoBoards advanced communication based on IT****  ****I2C_TwoBoards advanced communication based on IT****  ****I2C_TwoBoards advanced communication based on IT**** ";

/* Buffer used for reception */
uint8_t aRxBuffer[RXBUFFERSIZE];

/* Function Prototypes -------------------------------------------------------*/
static void Error_Handler(void);
static uint16_t Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength);


/**
  * @brief	Configure I2C peripheral
  * @param	I2C_Timing (uint32_t), I2C_Address (uint32_t)
  * @retval	None
  */
void Config_I2C_Periph(uint32_t I2C_Timing, uint32_t I2C_Address)
{
	/*##-1- Configure the I2C peripheral ######################################*/
	I2cHandle.Instance             = I2Cx;
	I2cHandle.Init.Timing          = I2C_Timing;
	I2cHandle.Init.OwnAddress1     = I2C_Address;
	I2cHandle.Init.AddressingMode  = I2C_ADDRESSINGMODE_10BIT;
	I2cHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	I2cHandle.Init.OwnAddress2     = 0xFF;
	I2cHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	I2cHandle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

	if(HAL_I2C_Init(&I2cHandle) != HAL_OK)
	{
		/* Initialization Error */
		Error_Handler();
	}

	/* Enable Fast Mode Plus FM+ on I2C1 */
	HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C1);
}



/**
  * @brief	Master board transmit and receive I2C demo function
  * @param	None
  * @retval	None
  */
void I2C_Master_Transmit_Receive_Demo(uint32_t I2C_Address)
{
	/* The board sends the message and expects to receive it back */

	  /*##-2- Start the transmission process #####################################*/
	  /* While the I2C in reception process, user can transmit data through
	     "aTxBuffer" buffer */
	  do
	  {
	    if(HAL_I2C_Master_Sequential_Transmit_IT(&I2cHandle, (uint16_t)I2C_Address, (uint8_t*)aTxBuffer, TXBUFFERSIZE, I2C_FIRST_FRAME)!= HAL_OK)
	    {
	      /* Error_Handler() function is called when error occurs. */
	      Error_Handler();
	    }

	    /*##-3- Wait for the end of the transfer #################################*/
	    /*  Before starting a new communication transfer, you need to check the current
	        state of the peripheral; if it’s busy you need to wait for the end of current
	        transfer before starting a new one.
	        For simplicity reasons, this example is just waiting till the end of the
	        transfer, but application may perform other tasks while transfer operation
	        is ongoing. */
	    while (HAL_I2C_GetState(&I2cHandle) != HAL_I2C_STATE_READY)
	    {
	    }

	    /* When Acknowledge failure occurs (Slave don't acknowledge it's address)
	       Master restarts communication */
	  }
	  while(HAL_I2C_GetError(&I2cHandle) == HAL_I2C_ERROR_AF);

	  /* Wait for User push-button press before starting the Communication */
	  while (BSP_PB_GetState(BUTTON_USER) != GPIO_PIN_SET)
	  {
	  }

	  /* Wait for User push-button release before starting the Communication */
	  while (BSP_PB_GetState(BUTTON_USER) != GPIO_PIN_RESET)
	  {
	  }

	  /*##-4- Put I2C peripheral in reception process ############################*/
	  do
	  {
	    if(HAL_I2C_Master_Sequential_Receive_IT(&I2cHandle, (uint16_t)I2C_Address, (uint8_t *)aRxBuffer, RXBUFFERSIZE, I2C_LAST_FRAME) != HAL_OK)
	    {
	      /* Error_Handler() function is called when error occurs. */
	      Error_Handler();
	    }

	    /*##-5- Wait for the end of the transfer #################################*/
	    /*  Before starting a new communication transfer, you need to check the current
	        state of the peripheral; if it’s busy you need to wait for the end of current
	        transfer before starting a new one.
	        For simplicity reasons, this example is just waiting till the end of the
	        transfer, but application may perform other tasks while transfer operation
	        is ongoing. */
	    while (HAL_I2C_GetState(&I2cHandle) != HAL_I2C_STATE_READY)
	    {
	    }

	    /* When Acknowledge failure occurs (Slave don't acknowledge it's address)
	       Master restarts communication */
	  }
	  while(HAL_I2C_GetError(&I2cHandle) == HAL_I2C_ERROR_AF);
}



/**
  * @brief	Slave board transmit and receive I2C demo function
  * @param	None
  * @retval	None
  */
void I2C_Slave_Demo(uint32_t uwTransferRequested)
{

	  /* The board receives the message and sends it back */

	  /*##-3- Put I2C peripheral in listen mode process ###########################*/
	  if(HAL_I2C_EnableListen_IT(&I2cHandle) != HAL_OK)
	  {
	    /* Transfer error in reception process */
	    Error_Handler();
	  }

	  /*##-4- Wait Address Match Code event ######################################*/
	  /*  Before starting a transfer, you need to wait a Master request event.
	      For simplicity reasons, this example is just waiting till an Address callback event,
	      but application may perform other tasks while transfer operation is ongoing. */
	  while(uwTransferRequested != 1)
	  {
	  }

	  /*##-5- Put I2C peripheral in reception process ############################*/
	  if(HAL_I2C_Slave_Sequential_Receive_IT(&I2cHandle, (uint8_t *)aRxBuffer, RXBUFFERSIZE, I2C_FIRST_FRAME) != HAL_OK)
	  {
	    /* Transfer error in reception process */
	    Error_Handler();
	  }

	  /*##-6- Wait for the end of the transfer ###################################*/
	  /*  Before starting a new communication transfer, you need to check the current
	      state of the peripheral; if it’s busy you need to wait for the end of current
	      transfer before starting a new one.
	      For simplicity reasons, this example is just waiting till the end of the
	      transfer, but application may perform other tasks while transfer operation
	      is ongoing. */
	  while (HAL_I2C_GetState(&I2cHandle) != HAL_I2C_STATE_LISTEN)
	  {
	  }

	  /*##-7- Wait Address Match Code event ######################################*/
	  /*  Before starting a transfer, you need to wait a Master request event.
	      For simplicity reasons, this example is just waiting till an Address callback event,
	      but application may perform other tasks while transfer operation is ongoing. */
	  while(uwTransferRequested != 1)
	  {
	  }

	  /*##-8- Start the transmission process #####################################*/
	  /* While the I2C in reception process, user can transmit data through
	     "aTxBuffer" buffer */
	  if(HAL_I2C_Slave_Sequential_Transmit_IT(&I2cHandle, (uint8_t*)aTxBuffer, TXBUFFERSIZE, I2C_LAST_FRAME)!= HAL_OK)
	  {
	    /* Transfer error in transmission process */
	    Error_Handler();
	  }
}





/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* Turn LED3 on */
  BSP_LED_On(LED3);
  while(1)
  {
  }
}

/**
  * @brief  Compares two buffers.
  * @param  pBuffer1, pBuffer2: buffers to be compared.
  * @param  BufferLength: buffer's length
  * @retval 0  : pBuffer1 identical to pBuffer2
  *         >0 : pBuffer1 differs from pBuffer2
  */
static uint16_t Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength)
{
  while (BufferLength--)
  {
    if ((*pBuffer1) != *pBuffer2)
    {
      return BufferLength;
    }
    pBuffer1++;
    pBuffer2++;
  }

  return 0;
}



/**
  * @brief I2C MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration
  *           - DMA configuration for transmission request by peripheral
  *           - NVIC configuration for DMA interrupt request enable
  * @param hi2c: I2C handle pointer
  * @retval None
  */
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO TX/RX clock */
  I2Cx_SCL_GPIO_CLK_ENABLE();
  I2Cx_SDA_GPIO_CLK_ENABLE();
  /* Enable I2Cx clock */
  I2Cx_CLK_ENABLE();

  /*##-2- Configure peripheral GPIO ##########################################*/
  /* I2C TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = I2Cx_SCL_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = I2Cx_SCL_SDA_AF;
  HAL_GPIO_Init(I2Cx_SCL_GPIO_PORT, &GPIO_InitStruct);

  /* I2C RX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = I2Cx_SDA_PIN;
  GPIO_InitStruct.Alternate = I2Cx_SCL_SDA_AF;
  HAL_GPIO_Init(I2Cx_SDA_GPIO_PORT, &GPIO_InitStruct);

  /*##-3- Configure the NVIC for I2C ########################################*/
  /* NVIC for I2Cx */
  HAL_NVIC_SetPriority(I2Cx_ER_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(I2Cx_ER_IRQn);
  HAL_NVIC_SetPriority(I2Cx_EV_IRQn, 0, 2);
  HAL_NVIC_EnableIRQ(I2Cx_EV_IRQn);
}

/**
  * @brief I2C MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO, DMA and NVIC configuration to their default state
  * @param hi2c: I2C handle pointer
  * @retval None
  */
void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{

  /*##-1- Reset peripherals ##################################################*/
  I2Cx_FORCE_RESET();
  I2Cx_RELEASE_RESET();

  /*##-2- Disable peripherals and GPIO Clocks #################################*/
  /* Configure I2C Tx as alternate function  */
  HAL_GPIO_DeInit(I2Cx_SCL_GPIO_PORT, I2Cx_SCL_PIN);
  /* Configure I2C Rx as alternate function  */
  HAL_GPIO_DeInit(I2Cx_SDA_GPIO_PORT, I2Cx_SDA_PIN);

  /*##-3- Disable the NVIC for I2C ##########################################*/
  HAL_NVIC_DisableIRQ(I2Cx_ER_IRQn);
  HAL_NVIC_DisableIRQ(I2Cx_EV_IRQn);
}
