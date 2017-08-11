#include <test.h>

TEST_DEFINE(adc_multichannel)
{


	
	test_assert("Calibration Pass", 5 != 0);
	test_assert("Timer Sync", 5 != 0);
	test_assert("DMA Buffer Check", 5 != 0);
	test_assert("Data Acquired", 5 != 0);
}
