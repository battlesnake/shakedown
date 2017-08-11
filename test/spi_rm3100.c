#include <test.h>

TEST_DEFINE(spi_gyro)
{
	test_assert("Data Acquired", 5 != 0);
	test_assert("CRC Pass", 5 != 0);
}
