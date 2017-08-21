extern "C" {
#include "i2c_hal.h"
}
#include "Sensor_AD7414.h"

Sensor_AD7414::Sensor_AD7414()
{
	clockSpeed = 4000;
	dutyCycle = dutyCycle_2;
	temperature = 0;
	valid = 0;
}
