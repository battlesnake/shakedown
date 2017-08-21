#include "Sensor_AD7414.hpp"

Sensor_AD7414::Sensor_AD7414()
{
	slaveAddress = AD7414_ADDRESS;
	powerMode = powerOn;
	interfaceId = 1;
	clockSpeed = 5000;
	dutyCycle = dutyCycle_2;
	temperature = 0;
	valid = 0;
}

Status_t Sensor_AD7414::initialize()
{
	// Check if parameters are properly initialized
	if (!AD7414_IS_ID(interfaceId) || \
	    !AD7414_IS_MODE(powerMode)) {
		return Error;
	}
	
	// Initialize the HAL structure with default settings
	I2C_HAL_InitStruct(&I2C_HALStruct, interfaceId);

	// Initialize the communication interface
	
	// Alter some of the interface settings
	I2C_HALStruct.id = interfaceId;
	I2C_HALStruct.clockSpeed = clockSpeed;
	I2C_HALStruct.dutyCycle = dutyCycle;

	// Start the interface (including enable clocks and gpio)
	I2C_HAL_Init(&I2C_HALStruct);

	// Initlialize the sensor power mode
	if (powerMode == powerDown) {
		fullPowerDown(ENABLE);
	}
	
	return Success;
}

Status_t Sensor_AD7414::getTemperature(int16_t *tempC)
{
	uint8_t buffer[2]; // buffer for writing and reading data
	uint16_t tempReg; // contains the temperature register value
	Status_t status;

	if (powerMode == powerDown) {
		// Send one shot comand to power up
		buffer[0] = AD7414_REG_CONF;
		buffer[1] = COMMAND_ONE_SHOT | COMMAND_POWER_DOWN;
	        I2C_HAL_Write(&I2C_HALStruct, buffer, 2, slaveAddress);
		// put delay > 29us here to wait for the power up and conversion!!!!!!!!!!
	}
	
	// Set the address to read from
	buffer[0] = AD7414_REG_TEMP;
	status = I2C_HAL_Write(&I2C_HALStruct, buffer, 1, slaveAddress);

	if (status == Error) {
		return Error;
	}       
	
	// Read from the address that is set and get the two bytes that contain the temperature
	status = I2C_HAL_Read(&I2C_HALStruct, buffer, 2, slaveAddress);

	// Get the useful temperature data from the two register bytes 
	tempReg = (buffer[0] << 2) | ((buffer[1] & 0b11000000) >> 6);

	temperature = tempFromReg(tempReg);
	*tempC = temperature;

	if (status == Success) {
		valid = 1;
	} else {
		valid = 0;
	}
	
	return status;
}

Status_t Sensor_AD7414::deinitialize()
{
	I2C_HAL_Deinit(&I2C_HALStruct);
	
	return Success;
}

Status_t Sensor_AD7414::fullPowerDown(bool state)
{
	Status_t status;
	uint8_t buffer[2]; // buffer for writing and reading data
	
	// Set the register address
	buffer[0] = AD7414_REG_CONF;
	
	if (state) {
		// Full power down
		buffer[1] = COMMAND_POWER_DOWN;
	} else {
		// Power on
		buffer[1] = COMMAND_POWER_ON;
	}

	// Send data over the interface
       	status = I2C_HAL_Write(&I2C_HALStruct, buffer, 1, slaveAddress);

	return status;
}

int16_t Sensor_AD7414::tempFromReg(uint16_t reg)
{
	// Check if the tempearture is negative or positive
	if (reg & 0x200) {
		return (int16_t)(((reg & 0x1FF) - 512) >> 2); // Remove the 10th bit from the calculation of the negative value
	} else {
		return (int16_t)(reg >> 2); // Calculate the positive temperature
	}
}
