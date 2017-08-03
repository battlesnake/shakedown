#include <test.h>

TEST_DEFINE(integer_multiply)
{
	test_assert("Associative", (2 * 3) * 4 == 2 * (3 * 4));
	test_assert("Commutative", 2 * 3 == 3 * 2);
	test_assert("Distributive", 2 * (3 + 4) == 2 * 3 + 2 * 4);
}
