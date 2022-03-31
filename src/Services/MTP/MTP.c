/*
 * MTP.c
 *
 *  Created on: Dec 16, 2021
 *      Author: Mohammed Ali
 */


#include "STD_Types.h"
#include "Bit_Math.h"

#include "MCAL/RCC/RCC_interface.h"
#include "MCAL/UART/UART_interface.h"
#include "MCAL/GPIO/GPIO_interface.h"
#include "HAL/Interrupts/Interrupts_interface.h"

#include "MTP_cfg.h"
#include "MTP_prv.h"
#include "MTP.h"

/*
 * Message Transfer Protocol
 * */

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//           //        //             //             //                            //               //               //
// StartByte // IDByte // LengthByte0 // LengthByte1 // DataByte0        DataByteN // CheckSumByte0 // CheckSumByte1 //
//           //        //             //             //                            //               //               //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* MTP Initialization */
void MTP_Init(void)
{
	u8 Local_Iterator = 0;
	RCC_EnableGPIOA();
	RCC_EnableUART();

	for(Local_Iterator = 0; Local_Iterator < sizeof(UARTPins)/sizeof(GPIO_t); Local_Iterator++)
	{
		GPIO_Init(&UARTPins[Local_Iterator]);
	}

	UART_Init();

	UART_SetInterruptHandler(MTP_Notify);

	Interrupts_EnableIRQ(UART_IRQ);

}

void MTP_Notify(u8 flags)
{
	if(flags & UART_RXNE_FLAG)
	{
		ReceivedByte = UART_ReadBuffer();
		ReceivedByteState = BYTE_RECEIVED;
		if(flags & UART_PE_FLAG)
		{
			ReceiveProcessState = FAILED;
		}
	}
	if(flags & UART_TC_FLAG)
	{
		if(SentByteState == SENDING_BYTE)
		{
			UART_WriteToBuffer(ByteToSend);
			SentByteState = BUFFER_EMPTY;
		}
		else if(SentByteState == FINISHED)
		{
			SentByteState = UNINITIALIZED;
		}
	}

}

void MTP_ReceiveProcess(void)
{
	u32 Local_Iterator = 0;
	u16 Local_DataCounter = 0;
	u16 Local_CHKCounter = 0;
	u16 Local_CheckSum = 0;
	u8 * Local_DataBuffer = RXFrameBuffer.Buffer;
	ReceiveProcessState = WAITING;

	while(1)
	{
		if(ReceiveProcessState == WAITING)
		{
			if(Local_Iterator >= MTPCyclesToBreak)
			{
				ReceiveProcessState = NO_FRAME_RECEIVED;
			}
		}
		else if(Local_Iterator >= MTPWaitCycles)
		{
			ReceiveProcessState = FAILED;
			break;
		}

		if(ReceivedByteState == BYTE_RECEIVED)
		{
			Local_Iterator = 0;
			ReceivedByteState = NO_DATA;

			switch(ReceiveProcessState)
			{
			case WAITING:
				if(ReceivedByte == MTP_START_BYTE)
				{
					ReceiveProcessState = START_BYTE_RECEIVED;
				}
				break;

			case START_BYTE_RECEIVED:
				RXFrameBuffer.ID = ReceivedByte;
				ReceiveProcessState = ID_RECEIVED;
				break;

			case ID_RECEIVED:
				RXFrameBuffer.length = ReceivedByte;
				ReceiveProcessState = LENGTH_BYTE_0_RECEIVED;
				break;

			case LENGTH_BYTE_0_RECEIVED:
				RXFrameBuffer.length |= (((u16)ReceivedByte) << 8);
				if(RXFrameBuffer.length == 0)
				{
					ReceiveProcessState = DATA_RECEIVED;
				}
				else
				{
					ReceiveProcessState = LENGTH_BYTE_1_RECEIVED;
				}
				break;

			case LENGTH_BYTE_1_RECEIVED:
				*Local_DataBuffer = ReceivedByte;
				Local_DataBuffer++;
				Local_DataCounter++;
				if(Local_DataCounter >= RXFrameBuffer.length)
				{
					ReceiveProcessState = DATA_RECEIVED;
				}
				break;

			case DATA_RECEIVED:
				RXFrameBuffer.checksum = ReceivedByte;
				ReceiveProcessState = CHECKSUM_BYTE_0_RECEIVED;
				break;

			case CHECKSUM_BYTE_0_RECEIVED:
				RXFrameBuffer.checksum |= (((u16)ReceivedByte) << 8);
				ReceiveProcessState = CHECKSUM_BYTE_1_RECEIVED;

				Local_DataBuffer = RXFrameBuffer.Buffer;
				for(Local_CHKCounter = 0; Local_CHKCounter < RXFrameBuffer.length; Local_CHKCounter++)
				{
					Local_CheckSum += *Local_DataBuffer;
					Local_DataBuffer++;
				}
				Local_CheckSum += RXFrameBuffer.ID;
				Local_CheckSum += (u8)(RXFrameBuffer.length);
				Local_CheckSum += (u8)((RXFrameBuffer.length) >> 8);
				if(RXFrameBuffer.checksum == Local_CheckSum)
				{
					ReceiveProcessState = SUCCESS;
					return;
				}
				else
				{
					ReceiveProcessState = FAILED;
					return;
				}
				break;

			default:
				ReceiveProcessState = FAILED;
				ReceivedByteState = NO_DATA;
				return;
			}
		}
		Local_Iterator++;
	}
}

STD_ERR MTP_ReceiveData(MTP_MSG_t * msg, u32 CyclesToBreak)
{
	RXFrameBuffer.Buffer = msg->Buffer;
	MTPCyclesToBreak = CyclesToBreak;

	UART_Enable();

	MTP_ReceiveProcess();

	UART_Disable();
	if(ReceiveProcessState == SUCCESS)
	{
		ReceiveProcessState = STOPPED;
		msg->ID = (RXFrameBuffer.ID);
		msg->length = (RXFrameBuffer.length);
		return NO_ERR;
	}
	else
	{
		ReceiveProcessState = STOPPED;
		return ERR;
	}
}

void MTP_SendProcess(void)
{
	u32 Local_Iterator = 0;
	u16 Local_DataCounter = 0;
	u8 * Local_DataBuffer = TXFrameBuffer.Buffer;

	UART_WriteToBuffer(MTP_START_BYTE);
	SendProcessState = START_BYTE_SENT;
	SentByteState = BUFFER_EMPTY;

	while(1)
	{
		if(Local_Iterator >= MTPWaitCycles)
		{
			SendProcessState = FAILED;
			break;
		}

		if(SentByteState == BUFFER_EMPTY)
		{
			Local_Iterator = 0;
			SentByteState = SENDING_BYTE;

			switch(SendProcessState)
			{
			case START_BYTE_SENT:
				ByteToSend = TXFrameBuffer.ID;
				SendProcessState = ID_SENT;
				break;

			case ID_SENT:
				ByteToSend = (u8)(TXFrameBuffer.length);
				SendProcessState = LENGTH_BYTE_0_SENT;
				break;

			case LENGTH_BYTE_0_SENT:
				ByteToSend = (u8)((TXFrameBuffer.length) >> 8);
				if(TXFrameBuffer.length == 0)
				{
					SendProcessState = DATA_SENT;
				}
				else
				{
					SendProcessState = LENGTH_BYTE_1_SENT;
				}
				break;

			case LENGTH_BYTE_1_SENT:
				ByteToSend = *Local_DataBuffer;
				Local_DataBuffer++;
				Local_DataCounter++;
				if(Local_DataCounter >= TXFrameBuffer.length)
				{
					SendProcessState = DATA_SENT;
				}
				break;

			case DATA_SENT:
				ByteToSend = (u8)(TXFrameBuffer.checksum);
				SendProcessState = CHECKSUM_BYTE_0_SENT;
				break;

			case CHECKSUM_BYTE_0_SENT:
				ByteToSend = (u8)((TXFrameBuffer.checksum) >> 8);
				SendProcessState = CHECKSUM_BYTE_1_SENT;
				break;

			case CHECKSUM_BYTE_1_SENT:
				SentByteState = FINISHED;
				break;

			default:
				SendProcessState = FAILED;
				return;
			}
		}

		if(SentByteState == UNINITIALIZED)
		{
			SendProcessState = SUCCESS;
			return;
		}
		Local_Iterator++;
	}
}

STD_ERR MTP_SendData(MTP_MSG_t * msg)
{
	u16 Local_CHKCounter = 0;
	u16 Local_CheckSum = 0;
	u8 * Local_DataBuffer = (msg->Buffer);

	TXFrameBuffer.Buffer = (msg->Buffer);
	TXFrameBuffer.ID = (msg->ID);
	TXFrameBuffer.length = (msg->length);

	for(Local_CHKCounter = 0; Local_CHKCounter < (msg->length); Local_CHKCounter++)
	{
		Local_CheckSum += *Local_DataBuffer;
		Local_DataBuffer++;
	}
	Local_CheckSum += (msg->ID);
	Local_CheckSum += (u8)(msg->length);
	Local_CheckSum += (u8)((msg->length) >> 8);
	TXFrameBuffer.checksum = Local_CheckSum;

	UART_Enable();

	MTP_SendProcess();

	UART_Disable();

	if(SendProcessState == SUCCESS)
	{
		SendProcessState = STOPPED;
		return NO_ERR;
	}
	else
	{
		SendProcessState = STOPPED;
		return ERR;
	}
}
