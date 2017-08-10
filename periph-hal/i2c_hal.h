#ifndef I2C_HAL
#define I2C_HAL

typedef enum {Error = 0, Success = !Error } Status;
typedef enum {dutyCycle_2 = 0, dutyCycke_16_9 = 1} DutyCycleType;

#define HAL_IS_I2C_DUTY_CYCLE(CYCLE) (((CYCLE) == dutyCycle_2) || \
				      ((CYCLE) == dutyCycle_16_9))

typedef struct
{
	I2CID Id;                //! I2C id number
	I2C_TypeDef *I2Cx;       //! Reference to the I2C
	I2C_InitTypeDef I2C_HDL; //! Reference to the I2C HDL structure
	uint32_t clockSpeed;     //! Specifies the clock frequency. Set to below 400KHz.
	DutyCycleType dutyCycle;      //! Specifies the I2C fast mode duty cycle.
} I2C_HALType;

Status I2C_Init(I2C_HALType *I2C_HAL, uint8_t id);
Status I2C_Open(I2C_HALType *I2C_HAL);
Status I2C_Read(I2C_HALType *I2C_HAL, uint8_t *buf, uint32_t nbyte, uint8_t SlaveAddress);
Status I2C_Write(I2C_HALType *I2C_HAL, const uint8_t* buf, uint32_t nbyte, uint8_t SlaveAddress);
Status I2C_Close(I2C_HALType *I2C_HAL);
Status I2C_Deinit(I2C_HALType *I2C_HAL);

#define Timed(x) Timeout = 0xFFFF; while (x) { if (Timeout-- == 0) goto errReturn;}
#define maxId 3
#define maxClockSpeed 400000

const I2C_TypeDef I2Cs[3] = {I2C1, I2C2, I2C3};

// TODO: Where to define the platform specific GPIO pins and clockS??!
// Nucleo Board Ref Pins
// D15 PB8 SCL
// D14 PB9 SDA

#endif
