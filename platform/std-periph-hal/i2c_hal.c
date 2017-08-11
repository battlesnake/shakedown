#include <FreeRTOS.h>
#include <stm32f4xx_i2c.h>
#include <stm32f4xx_gpio.h>
#include <stdio.h>
#include "i2c_hal.h"

/*
 * Initialization of the HAL structure
 */
Status I2C_HAL_InitStruct(I2C_HALType *I2C_HALStruct, uint8_t id)
{
	// Set the I2C id number
	I2C_HALStruct->id = id;
        
	// Initialize the I2C HDL reference
	if (id > 0 && id <= I2C_MAX_ID) {
		I2C_HALStruct->I2Cx = I2Cs[id-1];
	} else {
		//	return Error;
	}

	// Default clock speed set to 5 KHz
	I2C_HALStruct->clockSpeed = 5000;

	// Default duty cycle
	I2C_HALStruct->dutyCycle = dutyCycle_2;

       	return Success;
}

/*
 * Start the corresponding I2C interface depending on the HAL structure parameters
 */
Status I2C_HAL_Init(I2C_HALType *I2C_HALStruct)
{
	I2C_InitTypeDef I2C_HDLStruct;
	GPIO_InitTypeDef GPIO_HDLStruct;

	if (I2C_HALStruct->id > I2C_MAX_ID || \
	    I2C_HALStruct->id == 0 || \
	    I2C_HALStruct->clockSpeed > I2C_MAX_CLOCK_SPEED || \
	    !IS_I2C_ALL_PERIPH(I2C_HALStruct->I2Cx) || \
	    !I2C_HAL_IS_DUTY_CYCLE(I2C_HALStruct->dutyCycle)) {
		return Error;
	}

	// Enable clocks and GPIOs according to the I2C id
	if (I2C_HALStruct->id == 1) {
		/* Enable the clock for the GPIOs */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

		// Connect the I2C function to the pins
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_I2C1);
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_I2C1);

		/* Configure the SPI chip select and reset pins */
		GPIO_StructInit(&GPIO_HDLStruct);
		GPIO_HDLStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
		GPIO_HDLStruct.GPIO_OType = GPIO_OType_OD;	
		GPIO_HDLStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_HDLStruct.GPIO_Mode = GPIO_Mode_AF;
		GPIO_HDLStruct.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOB, &GPIO_HDLStruct);
	
		/* Enable the clock for the I2C */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	} else if (I2C_HALStruct->id == 2) {
		// TODO
	} else {
		// TODO
	}

	// Initialize the HDL structure with default values
	I2C_StructInit(&I2C_HDLStruct);

	// Change some of the parameters
	I2C_HDLStruct.I2C_OwnAddress1 = 0x0;
	I2C_HDLStruct.I2C_Ack = I2C_Ack_Enable;
	I2C_HDLStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_HDLStruct.I2C_Mode = I2C_Mode_I2C;
	I2C_HDLStruct.I2C_ClockSpeed = I2C_HAL->clockSpeed;
	if (I2C_HAL->dutyCyle == dutyCycle_2) {
		I2C_HDLStruct.>I2C_DutyCycle = I2C_DutyCycle_2;
	} else {
		I2C_HDLStruct.I2C_DutyCycle = I2C_DutyCycle_16_9;
	}	
	I2C_Init(I2C_HALStruct->I2Cx, &I2C_HDLStruct);

       	I2C_AcknowledgeConfig(I2C_HALStruct->I2Cx, ENABLE);

	I2C_Cmd(I2C_HALStruct->I2Cx, ENABLE);

	// Check the enabled bit and return error if not enabled
	 
	return Success;
}

/* 
 *  Read in polling mode
 */
Status I2C_HAL_Read(I2C_HalType* I2C_HALStruct, uint8_t *buf, uint32_t nbyte, uint8_t slaveAddres)
{
	__IO uint32_t Timeout = 0;
	I2C_TypeDef* I2Cx;

	I2Cx = I2C_HALStruct->I2Cx;

	// I2Cx->CR2 |= I2C_IT_ERR;  interrupts for errors
	
	if (!nbyte)
		return Success;

	// Wait for idle I2C interface
	Timed(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));

	// Enable Acknowledgement, clear POS flag
	I2C_AcknowledgeConfig(I2Cx, ENABLE);
	I2C_NACKPositionConfig(I2Cx, I2C_NACKPosition_Current);

	// Intiate Start Sequence (wait for EV5
	I2C_GenerateSTART(I2Cx, ENABLE);
	Timed(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));

	// Send Address
	I2C_Send7bitAddress(I2Cx, slaveAddres, I2C_Direction_Receiver);

	// EV6
  	Timed(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_ADDR));

	if (nbyte == 1) {
		// Clear Ack bit      
		I2C_AcknowledgeConfig(I2Cx, DISABLE);

		// EV6_1 -- must be atomic -- Clear ADDR, generate STOP
		__disable_irq();
		(void) I2Cx->SR2;                           
		I2C_GenerateSTOP(I2Cx,ENABLE);      
		__enable_irq();

		// Receive data   EV7
		Timed(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_RXNE));
		*buf++ = I2C_ReceiveData(I2Cx);
	}
	else if (nbyte == 2) {
		// Set POS flag
		I2C_NACKPositionConfig(I2Cx, I2C_NACKPosition_Next);

		// EV6_1 -- must be atomic and in this order
		__disable_irq();
		(void) I2Cx->SR2;                           // Clear ADDR flag
		I2C_AcknowledgeConfig(I2Cx, DISABLE);       // Clear Ack bit
		__enable_irq();

		// EV7_3  -- Wait for BTF, program stop, read data twice
		Timed(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_BTF));

		__disable_irq();
		I2C_GenerateSTOP(I2Cx,ENABLE);
		*buf++ = I2Cx->DR;
		__enable_irq();

		*buf++ = I2Cx->DR;

	} else {
		(void) I2Cx->SR2;                           // Clear ADDR flag
		while (nbyte-- != 3) {
			// EV7 -- cannot guarantee 1 transfer completion time, wait for BTF 
			//        instead of RXNE
			Timed(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_BTF)); 
			*buf++ = I2C_ReceiveData(I2Cx);
		}

		Timed(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_BTF));  

		// EV7_2 -- Figure 1 has an error, doesn't read N-2 !
		I2C_AcknowledgeConfig(I2Cx, DISABLE);       // clear ack bit

		__disable_irq();
		*buf++ = I2C_ReceiveData(I2Cx);             // receive byte N-2
		I2C_GenerateSTOP(I2Cx,ENABLE);              // program stop
		__enable_irq();
		*buf++ = I2C_ReceiveData(I2Cx);             // receive byte N-1

		// wait for byte N
		Timed(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED)); 
		*buf++ = I2C_ReceiveData(I2Cx);

		nbyte = 0;
	}

	// Wait for stop
	Timed(I2C_GetFlagStatus(I2Cx, I2C_FLAG_STOPF));
	
	return Success;

 errReturn:

	return Error;
}

/*
 * Write in polling mode
 */
Status I2C_HAL_Write(I2C_HALType* I2C_HALStruct, const uint8_t* buf,  uint32_t nbyte, uint8_t slaveAddr)
{
	__IO uint32_t Timeout = 0;
	I2C_TypeDef* I2Cx;

	I2Cx = I2C_HALStruct->I2Cx;

	/* Enable Error IT (used in all modes: DMA, Polling and Interrupts */
	//    I2Cx->CR2 |= I2C_IT_ERR;
	if (nbyte) {
		Timed(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));

		// Intiate Start Sequence
		I2C_GenerateSTART(I2Cx, ENABLE);
		Timed(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));

		// Send Address  EV5
		I2C_Send7bitAddress(I2Cx, slaveAddres, I2C_Direction_Transmitter);
		Timed(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

		// EV6
		// Write first byte EV8_1
		I2C_SendData(I2Cx, *buf++);

		while (--nbyte) {
			// wait on BTF
			Timed(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_BTF));  
			I2C_SendData(I2Cx, *buf++);
		}

		Timed(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_BTF));  
		I2C_GenerateSTOP(I2Cx, ENABLE);
		Timed(I2C_GetFlagStatus(I2C1, I2C_FLAG_STOPF));
	}
	
	return Success;

 errReturn:
	
	return Error;
}

Status I2C_HAL_DeInit(I2C_HALType *I2C_HALStruct)
{
        I2C_DeInit(I2C_HALStruct->I2Cx);
}
