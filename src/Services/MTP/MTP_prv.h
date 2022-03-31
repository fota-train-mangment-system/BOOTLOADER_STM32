/*
 * MTP_prv.h
 *
 *  Created on: Dec 16, 2021
 *      Author: Mohammed Ali
 */

#ifndef SERVICES_MTP_MTP_PRV_H_
#define SERVICES_MTP_MTP_PRV_H_

#define UART_IRQ		37
#define MTP_START_BYTE	0x25

/* Private Types */
typedef enum
{
	STOPPED,
	WAITING,
	NO_FRAME_RECEIVED,
	START_BYTE_RECEIVED,
	START_BYTE_SENT,
	ID_RECEIVED,
	ID_SENT,
	LENGTH_BYTE_0_RECEIVED,
	LENGTH_BYTE_0_SENT,
	LENGTH_BYTE_1_RECEIVED,
	LENGTH_BYTE_1_SENT,
	DATA_RECEIVED,
	DATA_SENT,
	CHECKSUM_BYTE_0_RECEIVED,
	CHECKSUM_BYTE_0_SENT,
	CHECKSUM_BYTE_1_RECEIVED,
	CHECKSUM_BYTE_1_SENT,
	FAILED,
	SUCCESS

} MTPState_t;

typedef enum
{
	NO_DATA,
	BYTE_RECEIVED
} MTPReceivedByteState_t;

typedef enum
{
	UNINITIALIZED,
	SENDING_BYTE,
	BUFFER_EMPTY,
	FINISHED,
} MTPSentByteState_t;

typedef struct
{
	u8 ID;
	u16 length;
	u8 * Buffer;
	u16 checksum;
} MTPFrame_t;

/* Private Variables */

__VOLATILE MTPState_t	ReceiveProcessState = STOPPED;
__VOLATILE MTPState_t	SendProcessState = STOPPED;
__VOLATILE MTPReceivedByteState_t ReceivedByteState = NO_DATA;
__VOLATILE MTPSentByteState_t SentByteState = UNINITIALIZED;
u8 ReceivedByte;
u8 ByteToSend;

/* Cycles required to received each byte */
u32 MTPWaitCycles = MTP_WAIT_CYCLES;

/* Cycles to wait for a frame */
u32 MTPCyclesToBreak;

/* Frame Data */
MTPFrame_t RXFrameBuffer;
MTPFrame_t TXFrameBuffer;

GPIO_t UARTPins[] =
{
		{GPIOA, GPIO_PIN_9, GPIO_OUTPUT_AF_PUSH_PULL},
		{GPIOA, GPIO_PIN_10, GPIO_INPUT_FLOATING}
};

/* Private Functions */
void MTP_Notify(u8 flags);
void MTP_ReceiveProcess(void);
void MTP_SendProcess(void);


#endif /* SERVICES_MTP_MTP_PRV_H_ */
