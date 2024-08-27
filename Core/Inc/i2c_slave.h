/*
 * i2c_slave.h
 *
 *  Created on: Jun 9, 2023
 *      Author: controllerstech
 */

#ifndef INC_I2C_SLAVE_H_
#define INC_I2C_SLAVE_H_

typedef enum {
	RxBufferEmpty = 0,
	RxBufferReady = 1,
	RxBufferProcessing = 2,
} RxBufferState_t;

extern RxBufferState_t GetRxBufferState(void);
extern uint8_t RxBuffer_Process(void);

#endif /* INC_I2C_SLAVE_H_ */
