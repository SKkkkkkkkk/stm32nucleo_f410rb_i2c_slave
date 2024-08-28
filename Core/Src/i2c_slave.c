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

typedef struct {
	bool	address_setting;
	uint8_t current_address;
	uint8_t eeprom_buffer[256];
} EEPROM_t;
EEPROM_t eeprom = {false, 0, {0}};

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
	assert(hi2c->Instance == I2C1);
	if (TransferDirection == I2C_DIRECTION_TRANSMIT)
	{
		// if the master wants to write the data to the slave,
		// the first byte is alway the address of the memory location.
		if(HAL_I2C_Slave_Sequential_Receive_IT(hi2c, &eeprom.current_address, 1, I2C_FIRST_FRAME) != HAL_OK)
		{
			printf("Error in HAL_I2C_Slave_Sequential_Receive_IT\r\n");
			Error_Handler();
		}
		eeprom.address_setting = true;
	}
	else  // if the master requests the data from the slave
	{
		HAL_I2C_Slave_Seq_Transmit_IT(hi2c, &eeprom.eeprom_buffer[eeprom.current_address], 1, I2C_FIRST_FRAME);
	}
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	assert(hi2c->Instance == I2C1);

	if(!eeprom.address_setting)
		eeprom.current_address++; // current_address is uint8_t, so it will wrap around automatically
	else
		eeprom.address_setting = false;

	HAL_I2C_Slave_Seq_Receive_IT(hi2c, &eeprom.eeprom_buffer[eeprom.current_address], 1, I2C_NEXT_FRAME);
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	assert(hi2c->Instance == I2C1);
	eeprom.current_address++;
	HAL_I2C_Slave_Seq_Transmit_IT(hi2c, &eeprom.eeprom_buffer[eeprom.current_address], 1, I2C_NEXT_FRAME);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	assert(hi2c->Instance == I2C1);
	if((hi2c->ErrorCode == HAL_I2C_ERROR_AF) && (hi2c->Mode == HAL_I2C_MODE_SLAVE) && (hi2c->State == HAL_I2C_STATE_LISTEN) && (hi2c->XferCount != 0))
	{	// STOP
		hi2c->ErrorCode = HAL_I2C_ERROR_NONE;
		hi2c->State = HAL_I2C_STATE_READY;
		HAL_I2C_EnableListen_IT(hi2c);
		return;
	}
	printf("HAL_I2C_ErrorCallback: ErrorCode = %d\r\n", hi2c->ErrorCode);
	Error_Handler();
}

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
	assert(hi2c->Instance == I2C1);
	HAL_I2C_EnableListen_IT(hi2c);
}

void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef *hi2c)
{
	assert(hi2c->Instance == I2C1);
	Error_Handler();
}
