#include <stdio.h>
#include <stm32xxxx_rcc.h>
#include <stm32xxxx_gpio.h>
#include <stm32xxxx_tim.h>
#include "spi_hal.h"
#include "crc_calc.h"

#ifdef STM32F4XX // Add definitions in board file and remove the if statment!!!!!

static uint16_t SPI_SendReceive16(uint16_t writeData);
static void delayus(uint8_t us);
static void TIMInit(TIM_TypeDef* TIMx);
static void GPIOInit();

static volatile uint32_t *delayTimer;

Status_t SPI_HAL_InitStruct(SPI_HALType *SPI_HALStruct, uint8_t id)
{
	SPI_HALStruct->id = id;

	// Initialize the I2C HDL reference
	if (id > 0 && id <= SPI_MAX_ID) {
		SPI_HALStruct->SPIx = (SPI_TypeDef *)SPIs(id-1);
	} else {
		return Error;
	}

	SPI_HALStruct->mode = SPI_Mode_Master;
	SPI_HALStruct->direction = SPI_Direction_2Lines_FullDuplex;
        SPI_HALStruct->dataSize = SPI_DataSize_16b;

	return Success;
}

/*
 * Start the corresponding I2C interface depending on the HAL structure parameters
 */
Status_t SPI_HAL_Init(SPI_HALType *SPI_HALStruct)
{
        SPI_InitTypeDef SPI_HDLStruct;

	// Check if parameters are properly set
	if (SPI_HALStruct->id > SPI_MAX_ID || \
	    SPI_HALStruct->id == 0 || \
	    !IS_SPI_ALL_PERIPH(SPI_HALStruct->SPIx) || \
	    !IS_SPI_DIRECTION_MODE(SPI_HALStruct->direction) || \
	    !IS_SPI_MODE(SPI_HALStruct->mode) || \
	    !IS_SPI_DATASIZE(SPI_HALStruct->dataSize)) {
		return Error;
	}

	// Initialize GPIOs (hardcoded for SPI1)
	GPIOInit();

	// Initialize TIM1 for intercommand delay
	TIMInit(TIM1);

	// Enable SPI clock
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

        // Initialize and enable SPI module
	SPI_StructInit(&SPI_HDLStruct);

	SPI_HDLStruct.SPI_Direction = SPI_HALStruct->direction;
	SPI_HDLStruct.SPI_Mode = SPI_HALStruct->mode;
	SPI_HDLStruct.SPI_DataSize = SPI_HALStruct->dataSize;

	SPI_HDLStruct.SPI_CPOL = SPI_CPOL_Low;
	SPI_HDLStruct.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_HDLStruct.SPI_NSS = SPI_NSS_Soft | SPI_NSSInternalSoft_Set;
	SPI_HDLStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
	SPI_HDLStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_Init(SPI_HALStruct->SPIx, &SPI_HDLStruct);

	SPI_Cmd(SPI1, ENABLE);

	return Success;
}

static uint16_t SPI_SendReceive16(uint16_t writeData)
{
	uint16_t readData;
	
				
	/* Wait for the TX register to get empty */
	while ((SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET));	
 	SPI_I2S_SendData(SPI1, writeData);

	/* Wait for RX register to get data */
	while ((SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET));
        readData =  (uint16_t)SPI_I2S_ReceiveData(SPI1);
	
        return readData;
}

void SPI_Send(uint32_t writeData)
{
        split_data split;
	uint32_t dataWithCRC;

	// Add CRC to the data
	dataWithCRC = prepareMessageCMS300(writeData);
        split.data32 = dataWithCRC;
	
	GPIO_ResetBits(GPIOA, GPIO_Pin_9);
	delayus(1);
	SPI_SendReceive16(split.data16[1]);
        SPI_SendReceive16(split.data16[0]);
	delayus(1);
	GPIO_SetBits(GPIOA, GPIO_Pin_9);
}

void SPI_Receive(uint32_t *readData)
{
        split_data split;
  
	// Send 32 bits dummy to read back data
	GPIO_ResetBits(GPIOA, GPIO_Pin_9);
	delayus(1);
	split.data16[1] = SPI_SendReceive16(0x00);
	split.data16[0] = SPI_SendReceive16(0x00);
	delayus(1);
	GPIO_SetBits(GPIOA, GPIO_Pin_9);

        // Check CRC
	if (!checkCRCmessageCMS300(split.data32)) {
		printf("CRC error\n");
	}
	
        *readData = split.data32;
}

void delayus(uint8_t us)
{
	const uint16_t usTicks = 4; // 4 MHz Timer Clock
	uint16_t wait, now;

	now = *delayTimer;
        wait = *delayTimer + us * usTicks; //read timer and add the wait period
	
	if (wait < now) {
		while (!(*delayTimer > wait && *delayTimer < now)) {}
	} else {
		while (*delayTimer < wait) {}
	}
}

void TIMInit(TIM_TypeDef* TIMx)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	//RCC_GetClocksFreq(RCC_Clocks); // where to get the pointer for the clocks struct?

	/* Configure the timer for the triggers */
	TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
	TIM_TimeBaseInitStruct.TIM_Period = 0xFFFFF;
	TIM_TimeBaseInitStruct.TIM_Prescaler = 0x00005; // 24 Mhz / (5 + 1) = 4 Mhz clock
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIMx, &TIM_TimeBaseInitStruct);

        delayTimer = &(TIMx->CNT);
	
	TIM_Cmd(TIMx, ENABLE);
}

void GPIOInit()
{
	GPIO_InitTypeDef GPIO_InitStruct;

	// Nucleo Board Ref
        // D13 PA5  SCK
	// D12 PA6  MISO
	// D11 PA7  MOSI
	// D2  PA10 OK
	// D8  PA9  CS
 	// D7  PA8  Reset
	
	/* Enable the clock for the GPIOs */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	/* Configure the SPI chip select and reset pins */
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9; 
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* Configure an input for the gyro OK status signal */
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Pin =  GPIO_Pin_10; 
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* Configure the pins of the SPI */
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
        GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);		
	
        /* CS in inactive high */ 
	GPIO_SetBits(GPIOA, GPIO_Pin_9);
	/* Reset in active low */
	GPIO_ResetBits(GPIOA, GPIO_Pin_8);
}
#endif
