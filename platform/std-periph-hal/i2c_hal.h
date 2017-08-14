#ifndef I2C_HAL
#define I2C_HAL

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	dutyCycle_2 = 0,
	dutyCycke_16_9 = 1
} DutyCycleType;
#define I2C_HAL_IS_DUTY_CYCLE(CYCLE) (((CYCLE) == 0) || \
				      ((CYCLE) == 1))
typedef struct {
	uint8_t id;              // I2C id number
	I2C_TypeDef* I2Cx;       // Reference to the I2C
	uint32_t clockSpeed;     // Specifies the clock frequency. Must be below 400 KHz
	DutyCycleType dutyCycle; // Specifies the I2C fast mode duty cycle
} I2C_HALType;

Status_t I2C_HAL_InitStruct(I2C_HALType *I2C_HALStruct, uint8_t id);
Status_t I2C_HAL_Init(I2C_HALType *I2C_HALStruct);
Status_t I2C_HAL_Read(I2C_HALType *I2C_HALStruct, uint8_t *buf, uint32_t nbyte, uint8_t slaveAddress);
Status_t I2C_HAL_Write(I2C_HALType *I2C_HALStruct, const uint8_t* buf, uint32_t nbyte, uint8_t slaveAddress);
Status_t I2C_HAL_Deinit(I2C_HALType *I2C_HALStruct);

#define Timed(x) Timeout = 0xFFFF; while (x) { if (Timeout-- == 0) goto errReturn;}

// TODO: Move the platform specific I2C instances, GPIO pins and clocks to a stm32fxx config file
// 
// * Nucleo Board Pins
// * D15 PB8 SCL
// * D14 PB9 SDA
#define I2C_MAX_ID 3
#define I2C_MAX_CLOCK_SPEED 400000

I2C_TypeDef *I2Cs[I2C_MAX_ID] = {I2C1, I2C2, I2C3};

#ifdef __cplusplus
}
#endif

#endif
