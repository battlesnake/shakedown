#include "test.h"

TEST_DEFINE(integer_add)
{
	test_assert("Associative", (2 + 3) + 4 == 2 + (3 + 4));
	test_assert("Commutative", 2 + 3 == 3 + 2);
	/* Should fail */
	test_assert("Distributive", 2 + (3 + 4) == 2 + 3 + 2 + 4);
}
