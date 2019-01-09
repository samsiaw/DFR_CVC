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

void set_SPI_check_bit_outputs(volatile PLC_Write_u_t *WriteValue);

/* Volatile variables --------------------------------------------------------*/
volatile PLC_Read_u_t PLC_Read;
volatile PLC_Write_u_t PLC_Write;
volatile SPI_inputs_vector_t SPI_inputs_vector;
volatile SPI_outputs_vector_t SPI_outputs_vector;

/* Private variables ---------------------------------------------------------*/
/* SPI handler declaration */
SPI_HandleTypeDef SpiHandle;

/* Buffer used for transmission */
uint8_t aTxBuffer[] = "****SPI - Two Boards communication based on Interrupt **** SPI Message ******** SPI Message ******** SPI Message ****";

uint8_t SPI_PLC_TxBuffer[PLCBUFFERSIZE];

/* Buffer used for reception */
uint8_t aRxBuffer[BUFFERSIZE];

uint8_t SPI_PLC_RxBuffer[PLCBUFFERSIZE];

/* transfer state */
__IO uint32_t wTransferState = TRANSFER_WAIT;


/**
  * @brief  SPI routine function that does SPI communication within synchronous task
  * @param	None
  * @retval	None
  */
void SPI_Routine(void)
{

	/*
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
	read_SPI_Rx_Buffer(SPI_PLC_RxBuffer, PLCBUFFERSIZE);
	SPI_PLC_Set_Inputs();

	SPI_PLC_Set_Outputs();
	write_SPI_Tx_Buffer(SPI_PLC_TxBuffer, PLCBUFFERSIZE);

	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	SPI_PLC_Transmit_Receive();
	*/

	SPI_PLC_Set_Outputs();
}



/**
  * @brief	Configure SPI
  * @param	Board type (MASTER or SLAVE)
  * @retval	None
  */
void SPI_Config(void)
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

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, GPIO_PIN_SET);

}



/**
  * @brief	Read new data from RxBuffer into PLC_Read
  * @param	RxBuffer pointer, buffer length
  * @retval	None
  */
void read_SPI_Rx_Buffer(uint8_t	*RxBuffer, uint16_t Buffer_Length)
{
	int i;
	for (i=0; i<Buffer_Length; i++)
	{
		PLC_Read.bytes[i] = RxBuffer[i];
	}
}


/**
  * @brief	Write data from PLC_Write to TxBuffer
  * @param	TxBuffer pointer, buffer length
  * @retval	None
  */
void write_SPI_Tx_Buffer(uint8_t *TxBuffer, uint16_t Buffer_Length)
{
	int i;
	for (i=0; i<Buffer_Length; i++)
	{
		TxBuffer[i] = PLC_Write.bytes[i];
	}
}


/**
  * @brief	Set SPI_inputs_vector values using PLC_Read data
  * @param	None
  * @retval	None
  */
void SPI_PLC_Set_Inputs(void)
{
	SPI_inputs_vector.ICE_enable 					= PLC_Read.bit.IN1;
	SPI_inputs_vector.Motor_enable 					= PLC_Read.bit.IN2;
	SPI_inputs_vector.Ready_to_drive 				= PLC_Read.bit.IN3;
	SPI_inputs_vector.Dash_BRB_press 				= !PLC_Read.bit.IN4;
	SPI_inputs_vector.IMD_safety_circuit_fault 		= PLC_Read.bit.IN5;
	SPI_inputs_vector.BMS_safety_circuit_fault 		= PLC_Read.bit.IN6;
	SPI_inputs_vector.Bamocar_safety_circuit_fault 	= PLC_Read.bit.IN7;

}


/**
  * @brief	Set PLC_Write values using SPI_ouputs_vector values
  * @param	None
  * @retval	None
  */
void SPI_PLC_Set_Outputs(void)
{
	PLC_Write.bit.OUT1 = SPI_outputs_vector.safety;
	PLC_Write.bit.OUT2 = SPI_outputs_vector.ready_to_drive;
	PLC_Write.bit.OUT3 = SPI_outputs_vector.rfg;
	PLC_Write.bit.OUT4 = SPI_outputs_vector.ignition_kill;
	PLC_Write.bit.OUT5 = SPI_outputs_vector.downshift_solenoid;
	PLC_Write.bit.OUT6 = SPI_outputs_vector.upshift_solenoid;
	PLC_Write.bit.OUT7 = 0;
	PLC_Write.bit.OUT8 = 0;

	set_SPI_check_bit_outputs(&PLC_Write);
}


/**
  * @brief	Set special output bits for PLC_Write
  * @param	None
  * @retval	None
  */
void set_SPI_check_bit_outputs(volatile PLC_Write_u_t *WriteValue)
{
	WriteValue->bit.SPARE= 0;

	WriteValue->bit.P0	=
	(
		  WriteValue->bit.OUT1
		+ WriteValue->bit.OUT2
		+ WriteValue->bit.OUT3
		+ WriteValue->bit.OUT4
		+ WriteValue->bit.OUT5
		+ WriteValue->bit.OUT6
		+ WriteValue->bit.OUT7
		+ WriteValue->bit.OUT8
	);

	WriteValue->bit.P1 	=
	(
		  WriteValue->bit.OUT2
		+ WriteValue->bit.OUT4
		+ WriteValue->bit.OUT6
		+ WriteValue->bit.OUT8
	);

	WriteValue->bit.P2 	=
	(
		  WriteValue->bit.OUT1
		+ WriteValue->bit.OUT3
		+ WriteValue->bit.OUT5
		+ WriteValue->bit.OUT7
	);

	WriteValue->bit.nP0 = !(WriteValue->bit.P0);
}

void SPI_PLC_Transmit_Receive(void)
{

	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);

	if(HAL_SPI_TransmitReceive_IT(&SpiHandle, (uint8_t*)SPI_PLC_TxBuffer, (uint8_t *)SPI_PLC_RxBuffer, PLCBUFFERSIZE) != HAL_OK)
	{
		/* Transfer error in transmission process */
		Error_Handler();
	}

	while (wTransferState == TRANSFER_WAIT)
	{
	}

	switch(wTransferState)
	{
	    case TRANSFER_COMPLETE :
	    	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
	    	break;
	    default :
	    	Error_Handler();
	    	break;
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
	//trans_complete_LEDs();
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
