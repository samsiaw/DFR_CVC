/*
 * cvc_spi.c
 *
 *  Created on: Dec 5, 2018
 *      Author: f002ccx
 */


/* Includes ------------------------------------------------------------------*/
#include "cvc_spi.h"

#include "stm32f7xx.h"
#include "stm32f7xx_nucleo_144.h"

/* Private define ------------------------------------------------------------*/
enum {
	TRANSFER_WAIT,
	TRANSFER_COMPLETE,
	TRANSFER_ERROR
};

/* Private functions ---------------------------------------------------------*/
static uint16_t Buffercmp(uint8_t *pBuffer1, uint8_t *pBuffer2, uint16_t BufferLength);
static void Error_Handler(void);

/* Private variables ---------------------------------------------------------*/
/* SPI handler declaration */
SPI_HandleTypeDef SpiHandle;

/* Buffer used for transmission */
uint8_t aTxBuffer[] = "****SPI - Two Boards communication based on Interrupt **** SPI Message ******** SPI Message ******** SPI Message ****";

/* Buffer used for reception */
uint8_t aRxBuffer[BUFFERSIZE];

/* transfer state */
__IO uint32_t wTransferState = TRANSFER_WAIT;



/**
  * @brief	Configure SPI
  * @param	Board type (MASTER or SLAVE)
  * @retval	None
  */
void SPI_Config(uint32_t boardType)
{
	/*##-1- Configure the SPI peripheral #######################################*/
	/* Set the SPI parameters */
	SpiHandle.Instance               = SPIx;
	SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
	SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
	SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
	SpiHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
	SpiHandle.Init.DataSize          = SPI_DATASIZE_16BIT;
	SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
	SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLE;
	SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
	SpiHandle.Init.CRCPolynomial     = 7;
	SpiHandle.Init.NSS               = SPI_NSS_SOFT;
	SpiHandle.Init.Mode 			 = SPI_MODE_MASTER;


	if(HAL_SPI_Init(&SpiHandle) != HAL_OK)
	{
		/* Initialization Error */
		Error_Handler();
	}
}



/**
  * @brief	Demo function for transmitting/receiving SPI data
  * @param	None
  * @retval	None
  */
void demo_Transmit_Receive(void)
{
	/*##-2- Start the Full Duplex Communication process ########################*/
	/* While the SPI in TransmitReceive process, user can transmit data through
     "aTxBuffer" buffer & receive data through "aRxBuffer" */
	if(HAL_SPI_TransmitReceive_IT(&SpiHandle, (uint8_t*)aTxBuffer, (uint8_t *)aRxBuffer, BUFFERSIZE) != HAL_OK)
	{
		/* Transfer error in transmission process */
		Error_Handler();
	}

	/*##-3- Wait for the end of the transfer ###################################*/
	/*  Before starting a new communication transfer, you must wait the callback call
      	to get the transfer complete confirmation or an error detection.
      	For simplicity reasons, this example is just waiting till the end of the
      	transfer, but application may perform other tasks while transfer operation
      	is ongoing. */
	while (wTransferState == TRANSFER_WAIT)
	{
	}

	switch(wTransferState)
	{
    	case TRANSFER_COMPLETE :
    		/*##-4- Compare the sent and received buffers ##############################*/
    		if(Buffercmp((uint8_t*)aTxBuffer, (uint8_t*)aRxBuffer, BUFFERSIZE))
    		{
    			/* Processing Error */
    			Error_Handler();
    		}
    		break;
    	default :
    		Error_Handler();
    		break;
	}
}


/**
  * @brief  Compares two buffers.
  * @param  pBuffer1, pBuffer2: buffers to be compared.
  * @param  BufferLength: buffer's length
  * @retval 0  : pBuffer1 identical to pBuffer2
  *         >0 : pBuffer1 differs from pBuffer2
  */
static uint16_t Buffercmp(uint8_t *pBuffer1, uint8_t *pBuffer2, uint16_t BufferLength)
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
  * @brief  TxRx Transfer completed callback.
  * @param  hspi: SPI handle
  * @note   This example shows a simple way to report end of Interrupt TxRx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	/* Transfer complete LED function*/
	trans_complete_LEDs();
	wTransferState = TRANSFER_COMPLETE;
}

/**
  * @brief  SPI error callbacks.
  * @param  hspi: SPI handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
	wTransferState = TRANSFER_ERROR;
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
	/* Error LED function */
	error_LEDs();
	while (1)
	{
	}
}




/**
  * @brief SPI MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration
  * @param hspi: SPI handle pointer
  * @retval None
  */
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
GPIO_InitTypeDef  GPIO_InitStruct;

  if (hspi->Instance == SPIx)
  {
    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO TX/RX clock */
    SPIx_SCK_GPIO_CLK_ENABLE();
    SPIx_MISO_GPIO_CLK_ENABLE();
    SPIx_MOSI_GPIO_CLK_ENABLE();
    /* Enable SPI clock */
    SPIx_CLK_ENABLE();

    /*##-2- Configure peripheral GPIO ##########################################*/
    /* SPI SCK GPIO pin configuration  */
    GPIO_InitStruct.Pin       = SPIx_SCK_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed     = GPIO_SPEED_MEDIUM;
    GPIO_InitStruct.Alternate = SPIx_SCK_AF;
    HAL_GPIO_Init(SPIx_SCK_GPIO_PORT, &GPIO_InitStruct);

    /* SPI MISO GPIO pin configuration  */
    GPIO_InitStruct.Pin 	  = SPIx_MISO_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_MEDIUM;
    GPIO_InitStruct.Alternate = SPIx_MISO_AF;
    HAL_GPIO_Init(SPIx_MISO_GPIO_PORT, &GPIO_InitStruct);

    /* SPI MOSI GPIO pin configuration  */
    GPIO_InitStruct.Pin = SPIx_MOSI_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_MEDIUM;
    GPIO_InitStruct.Alternate = SPIx_MOSI_AF;
    HAL_GPIO_Init(SPIx_MOSI_GPIO_PORT, &GPIO_InitStruct);

    /* SPI CS1 GPIO pin configuration */
    GPIO_InitStruct.Pin 	  = SPIx_CS1_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = SPIx_CS1_AF;
    HAL_GPIO_Init(SPIx_CS1_GPIO_PORT, &GPIO_InitStruct);

    /* SPI CS2 GPIO pin configuration */
    GPIO_InitStruct.Pin = SPIx_CS2_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = SPIx_CS1_AF;
    HAL_GPIO_Init(SPIx_CS2_GPIO_PORT, &GPIO_InitStruct);

    /* SPI Output Enable GPIO pin configuration */
    GPIO_InitStruct.Pin = SPIx_OUTEN_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = SPIx_OUTEN_AF;
    HAL_GPIO_Init(SPIx_OUTEN_GPIO_PORT, &GPIO_InitStruct);


    /*##-3- Configure the NVIC for SPI #########################################*/
    /* NVIC for SPI */
    HAL_NVIC_SetPriority(SPIx_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(SPIx_IRQn);
  }
}


/**
  * @brief SPI MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO and NVIC configuration to their default state
  * @param hspi: SPI handle pointer
  * @retval None
  */
void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
  if(hspi->Instance == SPIx)
  {
    /*##-1- Reset peripherals ##################################################*/
    SPIx_FORCE_RESET();
    SPIx_RELEASE_RESET();

    /*##-2- Disable peripherals and GPIO Clocks ################################*/
    /* Configure SPI SCK as alternate function  */
    HAL_GPIO_DeInit(SPIx_SCK_GPIO_PORT, SPIx_SCK_PIN);
    /* Configure SPI MISO as alternate function  */
    HAL_GPIO_DeInit(SPIx_MISO_GPIO_PORT, SPIx_MISO_PIN);
    /* Configure SPI MOSI as alternate function  */
    HAL_GPIO_DeInit(SPIx_MOSI_GPIO_PORT, SPIx_MOSI_PIN);

    /*##-3- Disable the NVIC for SPI ###########################################*/
    HAL_NVIC_DisableIRQ(SPIx_IRQn);
  }
}
