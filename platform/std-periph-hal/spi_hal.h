#ifndef SPI_HAL
#define SPI_HAL
#include <FreeRTOS.h>
#include <config.h>
#include <stm32xxxx_spi.h>

typedef union {
	uint32_t data32;
	uint16_t data16[2];
	uint8_t data8[4];
} split_data;

typedef struct {
	uint8_t id;         // Module id number
	I2C_TypeDef* SPIx;  // Reference to the module
	uint16_t direction; // Sets the direction of the transfer
	uint16_t mode;      // Sets slave or master mode
	uint16_t dataSize;  // Sets the interface message data size
} SPI_HALType;

Status_t SPI_HAL_InitStruct(SPI_HALType *SPI_HALStruct, uint8_t id);
Status_t SPI_HAL_Init(SPI_HALType *SPI_HALStruct);
Status_t SPI_HAL_Write(SPI_HALType *SPI_HALStruct, uint32_t writeData);
Status_t SPI_HAL_Read(SPI_HALType *SPI_HALStruct, uint32_t *readData);
Status_t SPI_HAL_DeInit(SPI_HALType *SPI_HALStruct);

#define SPI_MAX_ID 6

#endif
