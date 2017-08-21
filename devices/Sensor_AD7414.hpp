#ifndef DEF_Sensor_AD7414
#define DEF_Sensor_AD7414

#include "Sensor.hpp"

extern "C" {
#include "i2c_hal.h"
}

/* AD7414 sensor address */
#define AD7414_ADDRESS 0b1001010 // In the case of part number 7414-0, AS = Vdd

/* AD7414 commands */
#define COMMAND_ONE_SHOT 0b00000100 // use it to set D2 of the config register
#define COMMAND_POWER_DOWN 0b10000000 // use it to set D7 in the config register
#define COMMAND_POWER_ON 0b00000000 // use it to set D7 in the config register

/* AD7414 registers */
#define AD7414_REG_TEMP		0x00
#define AD7414_REG_CONF		0x01
#define AD7414_REG_T_HIGH	0x02
#define AD7414_REG_T_LOW	0x03

#define AD7414_IS_MODE(MODE) (((MODE) == 0) || \
			      ((MODE) == 1))
#define AD7414_IS_ID(ID) (((ID) == 1) || \
			  ((ID) == 2))

//! \brief The Sensor_AD7414 class is a driver for the AD7414 Temperature sensor.
//! It allows to acquire measurements, as well as getting and setting the configuration of the sensor.

class Sensor_AD7414 : public Sensor
{
    public:

	typedef enum {
		powerOn = 0, // conversions are performed at each 800 ms
		powerDown = 1 // low power consumption, conversions are performed after one shot command
	} ad7414Mode;
	
	I2C_HALType I2C_HALStruct; //!< Reference to the I2C interface HAL structure

	uint8_t slaveAddress; //!< Address of the sensor
	uint8_t interfaceId; //!< Configures the serial interface id
	uint32_t clockSpeed; //!< Configures the serial interface clock speed
	DutyCycleType dutyCycle; //!< Configures the serial interface duty cycle
	
	uint16_t temperature; //!< Stores the temperature value from the sensor
	uint8_t valid; //!< Indicates if the current temperature value is valid
	
	Sensor_AD7414::ad7414Mode powerMode; //!< Sets power on or power down mode
	
	//! \brief Builds a sensor interface with the provided sensor configuration. By default, the default configuration is used.
        Sensor_AD7414();

	Status_t initialize();

	// Note: write 1 to D7 of the config

	Status_t getTemperature(int16_t *temperature);

	//Status_t getAlarm(); //!< Gets the alarm status

	Status_t deinitialize();
	
    protected:
	
	int16_t tempFromReg(uint16_t reg); //!< Converts the register value into celsius
	Status_t fullPowerDown(bool state); //!< Enables or disables the full power down mode
	//Status_t setMinMax(); //!< Sets the min/max alarm values
	//Status_t getMinMax(uint8_t *min, uint8_t *max); // Get the min/max alarm values
};
#endif
