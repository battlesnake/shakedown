#ifndef DEF_Sensor_AD7414
#define DEF_Sensor_AD7414

#include "Sensor.h"
#include "i2c_hal.h"

/* AD7414 registers */
#define AD7414_REG_TEMP		0x00
#define AD7414_REG_CONF		0x01
#define AD7414_REG_T_HIGH	0x02
#define AD7414_REG_T_LOW	0x03

typedef enum {
	powerOn = 0,
	powerDown = 1
} ad7414Mode;
#define I2C_HAL_IS_AD7414_MODE(MODE) (((MODE) == 0) || \
			      ((MODE) == 1))

//! \brief The Sensor_AD7414 class is a driver for the AD7414 Temperature sensor.
//! It allows to acquire measurements, as well as getting and setting the configuration of the sensor.

class Sensor_AD7414 : public Sensor
{
    public:
	
	uint32_t clockSpeed; //!< Configures the serial interface clock speed
	uint8_t dutyCycle; //!< Configures the serial interface duty cycle
	uint16_t temperature; //!< Stores the temperature value from the sensor
	
	//! \brief Builds a sensor interface with the provided sensor configuration. By default, the default configuration is used.
        Sensor_AD7414();

	bool initialize();

	// Note: write 1 to D7 of the config
	bool fullPowerDown(bool state);

	bool getTemperature(int16 *temperature);
	
	//bool setMinMax();

	//bool getMinMax(uint8_t *min, uint8_t *max);

	//bool getAlarm();

	bool deinitialize();
	
    protected:
	
	int16_t tempFromReg(uint16_t reg);		
}
