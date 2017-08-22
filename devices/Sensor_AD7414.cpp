#include "Sensor_AD7414.hpp"

/*
 * Sensor default settings
 */
Sensor_AD7414::Sensor_AD7414()
{
	slaveAddress = AD7414_ADDRESS << 1; // The address must be shifted one bit left
	powerMode = powerOn; // Use power on mode by default
	interfaceId = 1; // 1st instance of the interface
	clockSpeed = 5000; // Set default clock speed
	dutyCycle = dutyCycle_2; // Set interface duty cycle
	temperature = 0; // Initialize the temperature value
	valid = 0; // Set the temperature valid flag
	useAlarm = DISABLE; // Don't use the alarm by default
}

/*
 * Initialization of the sensor
 */
Status_t Sensor_AD7414::initialize()
{
	// Check if parameters are properly initialized
	if (!AD7414_IS_ID(interfaceId) || \
	    !AD7414_IS_MODE(powerMode)) {
		return Error;
	}
	
	// Initialize the HAL structure with default settings
	I2C_HAL_InitStruct(&I2C_HALStruct, interfaceId);
	
	// Alter some of the interface settings
	I2C_HALStruct.id = interfaceId;
	I2C_HALStruct.clockSpeed = clockSpeed;
	I2C_HALStruct.dutyCycle = dutyCycle;

	// Start the interface (including enable clocks and gpio)
	I2C_HAL_Init(&I2C_HALStruct);

	// Initlialize the sensor power mode
	if (powerMode == powerDown) {
		if (fullPowerDown(ENABLE) == Error) {
			return Error;
		}
	}

	// Set up the low and high boundaries of the alarm
	if (useAlarm == ENABLE) {
		if (setMinMax() == Error) {
			return Error;
		}
	}
	
	return Success;
}

/*
 * Read temperature from the sensor
 */
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

/*
 * Deinitialization of the sensor
 */
Status_t Sensor_AD7414::deinitialize()
{
	I2C_HAL_DeInit(&I2C_HALStruct);
	
	return Success;
}

/*
 * Enter full power down mode (Not tested)
 */
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
       	status = I2C_HAL_Write(&I2C_HALStruct, buffer, 2, slaveAddress);

	return status;
}

/*
 * Set up minimum and maximum values of the temperature alarm
 */
Status_t Sensor_AD7414::setMinMax()
{
	Status_t status;
	uint8_t buffer[2]; // buffer for writing and reading data
	
	// Set the register address
	buffer[0] = AD7414_REG_T_HIGH;
	buffer[1] = tempHigh;

	// Send data over the interface
       	status = I2C_HAL_Write(&I2C_HALStruct, buffer, 2, slaveAddress);
	
	// Set the register address
	buffer[0] = AD7414_REG_T_LOW;
	buffer[1] = tempLow;

	// Send data over the interface
       	status = I2C_HAL_Write(&I2C_HALStruct, buffer, 2, slaveAddress);	

	return status;
}

/*
 * Convert the register temperature value into celsius
 */
int16_t Sensor_AD7414::tempFromReg(uint16_t reg)
{
	// Check if the tempearture is negative or positive
	if (reg & 0x200) {
		return (int16_t)(((reg & 0x1FF) - 512) >> 2); // Remove the 10th bit from the calculation of the negative value
	} else {
		return (int16_t)(reg >> 2); // Calculate the positive temperature
	}
}
