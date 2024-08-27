/*
 * i2c_slave.c
 *
 *  Created on: Jun 9, 2023
 *      Author: controllerstech
 */


#include "main.h"
#include "i2c_slave.h"

extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart2;

#define RxSIZE  256
static uint8_t RxBuffer[RxSIZE];
static RxBufferState_t RxBufferState = RxBufferEmpty;
static uint8_t RxBufferNumber = 0;

RxBufferState_t GetRxBufferState(void)
{
	return RxBufferState;
}

uint8_t RxBuffer_Process(void)
{
	if((RxBufferState == RxBufferReady) && (RxBufferNumber != 0))
	{
		__disable_irq();
		RxBufferState = RxBufferProcessing;

		// process the data here
		printf("Recived data: ");
		for(uint8_t i = 0; i < RxBufferNumber; i++)
			printf("0x%x ", RxBuffer[i]);
		printf("\r\n");


		RxBufferNumber = 0;
		RxBufferState = RxBufferEmpty;
		__enable_irq();
		return 0;
	}
	return 1;
}


void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
	assert(hi2c->Instance == I2C1);
	if (TransferDirection == I2C_DIRECTION_TRANSMIT)  // if the master wants to transmit the data
	{
		assert(RxBufferState != RxBufferProcessing);
		if(RxBufferState == RxBufferReady)
			printf("WARNING: last rxdata has not prcessed, it gonna be overwrited\n\r");
		if(HAL_I2C_Slave_Sequential_Receive_IT(hi2c, RxBuffer, RxSIZE, I2C_FIRST_AND_LAST_FRAME) != HAL_OK)
		{
			printf("Error in HAL_I2C_Slave_Sequential_Receive_IT\r\n");
			Error_Handler();
		}
	}
	else  // if the master requests the data from the slave
	{
		Error_Handler();  // call error handler
	}
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	assert(hi2c->Instance == I2C1);
	if((hi2c->ErrorCode == HAL_I2C_ERROR_AF) && (hi2c->Mode == HAL_I2C_MODE_SLAVE) && (hi2c->State == HAL_I2C_STATE_LISTEN) && (hi2c->XferCount != 0))
	{
		RxBufferState = RxBufferReady;
		RxBufferNumber = hi2c->XferSize - hi2c->XferCount;
		hi2c->ErrorCode = HAL_I2C_ERROR_NONE;
		hi2c->State = HAL_I2C_STATE_READY;
		HAL_I2C_EnableListen_IT(hi2c);
		return;
	}
	Error_Handler();
}

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
	assert(hi2c->Instance == I2C1);
	// HAL_I2C_EnableListen_IT(hi2c);
	printf("We should never reach here: %s", __func__);
	Error_Handler();  // we should never reach here
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	assert(hi2c->Instance == I2C1);
	printf("Rx Buffer Overflow!\n\r");
	Error_Handler(); // we don't use this callback as a sign of the end of the transmission, we should never reach here
}

void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef *hi2c)
{
	assert(hi2c->Instance == I2C1);
	Error_Handler();
}
