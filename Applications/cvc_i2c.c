/*
 * cvc_i2c.c
 *
 *  Created on: Dec 8, 2018
 *      Author: f002ccx
 */

#include "cvc_i2c.h"




/**
  * @brief	Configure I2C Peripheral
  * @param	I2C_Address, I2C_Timing, I2cHandle pointer
  * @retval	None
  */
void Config_I2C_Periph(uint32_t I2C_Address, uint32_t I2C_Timing, I2C_HandleTypeDef* I2cHandle)
{
	/*##-1- Configure the I2C peripheral ######################################*/
	I2cHandle->Instance             = I2Cx;
	I2cHandle->Init.Timing          = I2C_Timing;
	I2cHandle->Init.OwnAddress1     = I2C_Address;
	I2cHandle->Init.AddressingMode  = I2C_ADDRESSINGMODE_10BIT;
	I2cHandle->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	I2cHandle->Init.OwnAddress2     = 0xFF;
	I2cHandle->Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	I2cHandle->Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

	if(HAL_I2C_Init(I2cHandle) != HAL_OK)
	{
		/* Initialization Error */
		Error_Handler();
	}

	/* Enable Fast Mode Plus FM+ on I2C1 */
	HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C1);
}


/**
  * @brief	Master demo I2C Transmission
  * @param	I2c_Address, I2cHandle pointer
  * @retval	None
  */
void I2C_Master_Demo_Transmit(uint32_t I2C_Address, I2C_HandleTypeDef* I2cHandle, uint8_t* aTxBuffer)
{
	 /* The board sends the message and expects to receive it back */

	  /*##-2- Start the transmission process #####################################*/
	  /* While the I2C in reception process, user can transmit data through
	     "aTxBuffer" buffer */
	  do
	  {
	    if(HAL_I2C_Master_Sequential_Transmit_IT(I2cHandle, (uint16_t)I2C_Address, (uint8_t*)aTxBuffer, TXBUFFERSIZE, I2C_FIRST_FRAME)!= HAL_OK)
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
	    while (HAL_I2C_GetState(I2cHandle) != HAL_I2C_STATE_READY)
	    {
	    }

	    /* When Acknowledge failure occurs (Slave don't acknowledge it's address)
	       Master restarts communication */
	  }
	  while(HAL_I2C_GetError(I2cHandle) == HAL_I2C_ERROR_AF);

}



/**
  * @brief	Master demo I2C receive
  * @param	I2c_Address,I2cHandle pointer
  * @retval	None
  */
void I2C_Master_Demo_Receive(uint32_t I2C_Address, I2C_HandleTypeDef* I2cHandle, uint8_t* aRxBuffer)
{

	  /*##-4- Put I2C peripheral in reception process ############################*/
	  do
	  {
	    if(HAL_I2C_Master_Sequential_Receive_IT(I2cHandle, (uint16_t)I2C_Address, (uint8_t *)aRxBuffer, RXBUFFERSIZE, I2C_LAST_FRAME) != HAL_OK)
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
	    while (HAL_I2C_GetState(I2cHandle) != HAL_I2C_STATE_READY)
	    {
	    }

	    /* When Acknowledge failure occurs (Slave don't acknowledge it's address)
	       Master restarts communication */
	  }
	  while(HAL_I2C_GetError(I2cHandle) == HAL_I2C_ERROR_AF);
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
