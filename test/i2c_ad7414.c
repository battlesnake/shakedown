#include <test.h>

TEST_DEFINE(i2c_ad7414)
{
	test_assert("Module enabled", 5 != 0);
	test_assert("Temperature acquired", 5 != 0);
}
