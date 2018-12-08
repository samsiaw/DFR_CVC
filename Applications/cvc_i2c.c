/*
 * cvc_i2c.c
 *
 *  Created on: Dec 8, 2018
 *      Author: f002ccx
 */

#include "stm32f7xx.h"
#include "stm32f7xx_nucleo_144.h"
#include "cvc_i2c.h"

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
