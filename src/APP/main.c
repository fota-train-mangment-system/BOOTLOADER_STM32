/*
 * main.c
 *
 *  Created on: Dec 1, 2021
 *      Author: Mohammed Ali
 */

/* Includes */
#include "STD_Types.h"
#include "Bit_Math.h"

#include "MCAL/UART/UART_interface.h"
#include "MCAL/GPIO/GPIO_interface.h"
#include "MCAL/Flash/Flash_interface.h"
#include "MCAL/RCC/RCC_interface.h"

#include "Services/MTP/MTP.h"

#include "BootloaderProcess.h"
/* Private typedef */
/* Private define  */
/* Private macro */
/* Private variables */
u8 arr[100];
MTP_MSG_t msg;

/* Private function prototypes */
/* Private functions */

/**
**===========================================================================
**
**  Abstract: main program
**
**===========================================================================
*/


int main(void)
{
	/* Initialize Message buffer */
	msg.Buffer = arr;

	/* Message Transfer Protocol Initialization */
	MTP_Init();

	/* Bootloader Application */
	Bootloader_Main();

	while(1)
	{

	}
}
