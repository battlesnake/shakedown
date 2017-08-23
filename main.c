#include <FreeRTOS.h>
#include <misc.h>
#include <stdio.h>
#include <metadata.h>
#include <test.h>
#include <task.h>

#if defined TEST_MAIN_NOARGS
int main()
{
#else
int main(int argc, char *argv[])
{
	(void) argc;
	(void) argv;
#endif
	int v = 0;
	
	//__asm__("BKPT");

	v = 1;
	
	SystemInit();

	//__asm__("BKPT");

	v++;

	test_checklist_execute(4);

	test_checklist_execute(4);
	
	//print_build_configuration();
	//printf("Dummy program, run by default when not invoking the test runner\n");
	//printf("\n");
	//printf("Contains demo shell to show how to provide access to the tests from the main program\n");
	//printf("\n");
	//printf("Use command 'h' for shell help\n");
	//printf("\n");
	//test_shell();
	return 0;
}

 
