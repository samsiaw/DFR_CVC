/*
 * cvc_logging.h
 *
 *  Created on: Dec 7, 2018
 *      Author: f002bc7
 */

#ifndef INC_CVC_LOGGING_H_
#define INC_CVC_LOGGING_H_


/* Includes ------------------------------------------------------------------*/
/* Standard includes. */
#include <stdio.h>
#include <time.h>

/* FreeRTOS includes. */
#include <FreeRTOS.h>
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"

/* FreeRTOS+FAT includes. */
#include "ff_stdio.h"
#include "ff_ramdisk.h"
#include "ff_sddisk.h"

/* ST includes. */
#include "stm32f7xx_hal.h"

/* Public Function Prototypes ------------------------------------------------*/
void demoLoggingTask(void * parameters);

#endif /* INC_CVC_LOGGING_H_ */
