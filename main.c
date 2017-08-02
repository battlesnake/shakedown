#include <stdio.h>
#include "test/test.h"

int main(int argc, char *argv[])
{
	printf("Dummy program, run by default when not invoking the test runner\n");
	if (argc == -1) {
		test_suites_run_all(false);
	}
	return 0;
}
