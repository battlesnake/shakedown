#include <FreeRTOS.h>
#include <misc.h>
#include <stdio.h>
#include <metadata.h>
#include <test.h>

#if defined TEST_MAIN_NOARGS
int main()
{
#else
int main(int argc, char *argv[])
{
	(void) argc;
	(void) argv;
#endif
	SystemInit();

	print_build_configuration();
	printf("Dummy program, run by default when not invoking the test runner\n");
	printf("\n");
	printf("Contains demo shell to show how to provide access to the tests from the main program\n");
	printf("\n");
	printf("Use command 'h' for shell help\n");
	printf("\n");
	test_shell();
	return 0;
}
