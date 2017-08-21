extern "C" {
#include <test.h>
#include <stdio.h>
}
#include <Sensor_AD7414.hpp>
#define MAX_TEMP 125
#define MIN_TEMP -40

TEST_DEFINE(i2c_ad7414)
{
	Status_t status;
	Sensor_AD7414 ad7414;
	int16_t temperature;
	char strBuf[80];
	
	status = ad7414.initialize();
       
	test_assert("AD7414 initialized", status);

	status = ad7414.getTemperature(&temperature);

	sprintf(strBuf, "Temperature acquired: %d C", (int)temperature);
	test_assert(strBuf, status && temperature > MIN_TEMP && temperature < MAX_TEMP);

	status = ad7414.deinitialize();

	test_assert("AD7414 deinitialized", status);
}
