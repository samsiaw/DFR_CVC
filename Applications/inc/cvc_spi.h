/*
 * cvc_spi.h
 *
 *  Created on: Dec 12, 2018
 *      Author: f002ccx
 */

#ifndef INC_CVC_SPI_H_
#define INC_CVC_SPI_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_nucleo_144.h"



/* Exported constants --------------------------------------------------------*/
/* User can use this section to tailor SPIx instance used and associated
   resources */
/* Definition for SPIx clock resources */
#define SPIx                             SPI1
#define SPIx_CLK_ENABLE()                __HAL_RCC_SPI1_CLK_ENABLE()
#define SPIx_SCK_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOB_CLK_ENABLE()
#define SPIx_MISO_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define SPIx_MOSI_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()

#define SPIx_FORCE_RESET()               __HAL_RCC_SPI1_FORCE_RESET()
#define SPIx_RELEASE_RESET()             __HAL_RCC_SPI1_RELEASE_RESET()

/* Definition for SPIx Pins */
#define SPIx_SCK_PIN                     GPIO_PIN_5
#define SPIx_SCK_GPIO_PORT               GPIOA
#define SPIx_SCK_AF                      GPIO_AF5_SPI1
#define SPIx_MISO_PIN                    GPIO_PIN_6
#define SPIx_MISO_GPIO_PORT              GPIOA
#define SPIx_MISO_AF                     GPIO_AF5_SPI1
#define SPIx_MOSI_PIN                    GPIO_PIN_7			//change to 5 for PCB
#define SPIx_MOSI_GPIO_PORT              GPIOA				//change to GPIOB for PCB
#define SPIx_MOSI_AF                     GPIO_AF5_SPI1
#define SPIx_CS1_PIN					 GPIO_PIN_15
#define SPIx_CS1_GPIO_PORT				 GPIOD
#define SPIx_CS1_AF						 GPIO_AF5_SPI1
#define SPIx_CS2_PIN					 GPIO_PIN_14
#define SPIx_CS2_GPIO_PORT				 GPIOD
#define SPIx_CS2_AF						 GPIO_AF5_SPI1
#define SPIx_OUTEN_PIN					 GPIO_PIN_9
#define SPIx_OUTEN_GPIO_PORT			 GPIOE
#define SPIx_OUTEN_AF					 GPIO_AF5_SPI1

/* Definition for SPIx's NVIC */
#define SPIx_IRQn                        SPI1_IRQn
#define SPIx_IRQHandler                  SPI1_IRQHandler

/* Size of buffer */
#define BUFFERSIZE                       (COUNTOF(aTxBuffer) - 1)
#define PLCBUFFERSIZE					 2

/* Exported macro ------------------------------------------------------------*/
#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))

/* Function Prototypes ------------------------------------------------------ */
void SPI_Routine(void);


void SPI_Config(void);

void read_SPI_Rx_Buffer(uint8_t	*RxBuffer, uint16_t Buffer_Length);
void write_SPI_Tx_Buffer(uint8_t *TxBuffer, uint16_t Buffer_Length);

void SPI_PLC_Set_Inputs(void);
void SPI_PLC_Set_Outputs(void);



void demo_Transmit_Receive(void);




void trans_complete_LEDs(void);


/* Exported types ------------------------------------------------------------*/

volatile typedef struct SPI_inputs_vector_s
{
	uint8_t ICE_enable;						// PLC input 1
	uint8_t Motor_enable;					// PLC input 2
	uint8_t Ready_to_drive;					// PLC input 3
	uint8_t Dash_BRB_press;					// PLC input 4
	uint8_t	IMD_safety_circuit_fault;		// PLC input 5
	uint8_t BMS_safety_circuit_fault;		// PLC input 6
	uint8_t Bamocar_safety_circuit_fault;	// PLC input 7
	//extra input							// PLC input 8

} SPI_inputs_vector_t;

volatile typedef struct SPI_outputs_vector_s
{
	uint8_t	safety;					// PLC output 1
	uint8_t	ready_to_drive;			// PLC output 2
	uint8_t	rfg;					// PLC output 3
	uint8_t ignition_kill;			// PLC output 4
	uint8_t	downshift_solenoid;		// PLC output 5
	uint8_t upshift_solenoid;		// PLC output 6
	//extra output					// PLC output 7
	//extra output					// PLC output 8

} SPI_outputs_vector_t;


typedef struct PLC_Read_s
{
	int	High	:	1;	// Bit 0
	int Low		:	1;	// Bit 1
	int PC4		:	1;	// Bit 2
	int PC3		:	1;	// Bit 3

	int PC2		:	1;
	int PC1		:	1;
	int OTA		:	1;
	int UVA		:	1;

	int IN1		:	1;
	int IN2		:	1;
	int IN3		:	1;
	int IN4		:	1;
	int IN5		:	1;
	int IN6		:	1;
	int IN7		:	1;
	int IN8		:	1;

} PLC_Read_t;


typedef struct PLC_Write_s
{
	uint16_t	nP0		:	1;  // Bit 0
	uint16_t	P0		:	1;  // Bit 1
	uint16_t	P1		:	1;  // Bit 2
	uint16_t	P2		:	1;  // Bit 3

	uint16_t  	SPARE 	:	4;  // Bits 4-7

	uint16_t	OUT1	:	1;	// Bit 9
	uint16_t	OUT2	:	1;	// Bit 10
	uint16_t	OUT3	:	1;	// Bit 11
	uint16_t	OUT4	:	1;	// Bit 12
	uint16_t	OUT5	:	1;	// Bit 13
	uint16_t	OUT6	:	1;	// Bit 14
	uint16_t	OUT7	:	1;	// Bit 15
	uint16_t	OUT8	:	1;	// Bit 8

} PLC_Write_t;

typedef union PLC_Read_u
{
	uint16_t	word;
	uint8_t		bytes[2];
	PLC_Read_t	bit;
} PLC_Read_u_t;

typedef union PLC_Write_u
{
	uint16_t	word;
	uint8_t		bytes[2];
	PLC_Write_t	bit;
} PLC_Write_u_t;

/* Exported variables ----------------------------------------------------------------- */
enum {
	MASTER,
	SLAVE
};

extern volatile SPI_inputs_vector_t SPI_inputs_vector;
extern volatile SPI_outputs_vector_t SPI_outputs_vector;

extern volatile PLC_Read_u_t	PLC_Read;  // Read from SPI input chip
extern volatile PLC_Write_u_t	PLC_Write; // Write to SPI output chip


#endif /* INC_CVC_SPI_H_ */
