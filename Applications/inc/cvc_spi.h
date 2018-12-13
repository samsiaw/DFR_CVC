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

/* Exported types ------------------------------------------------------------*/
volatile typedef struct SPI_outputs_vector_s
{
	uint8_t	safety;
	uint8_t	ready_to_drive;
	uint8_t	rfg;
	uint8_t ignition_kill;
	uint8_t	downshift_solenoid;
	uint8_t upshift_solenoid;

} SPI_outputs_vector_t;

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

/* Exported macro ------------------------------------------------------------*/
#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))

/* Function Prototypes ------------------------------------------------------ */
void SPI_Config(uint32_t boardType);
void demo_Transmit_Receive(void);

void trans_complete_LEDs(void);
void error_LEDs(void);

/* Variables ----------------------------------------------------------------- */
enum {
	MASTER,
	SLAVE
};

#endif /* INC_CVC_SPI_H_ */
