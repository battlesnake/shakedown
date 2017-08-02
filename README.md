# Shakedown

Naming: "shakedown" = "sea trials" ≈ C testing

## Motivation

I wanted a way to add test suites to embedded C software such that they can be compiled into the normal image, then be accessed via network or by serial terminal.

The runner should produce nicely formatted test reports with plenty of diagnostic data, and where a serial terminal is available the runner should permit the user to select/deselect tests and to run/re-run individual tests as needed, providing a checklist of previous test results.

The compilater configuration for tests should be as similar as possible to the configuration for normal running.

This implementation does not require the build system to provide any magic preprocessor definitions or odd linker configuration, so should integrate into other Makefile projects quite easily.  Generation of the separate test-only binary (via objcopy) simply excludes the program `main` function, and redefines the `test_main` function to be called `main`.

It does however have issues with link-time optimisation - so to use LTO one would need to resort to `-Dtest_main=main` preprocessor trickery rather than relying on `objcopy`.
Doing this would cause name collision with the real `main` when linking the `program`.
Solving this would typically be done by excluding the tests from the main build.
Another solution would be to not generate the test binary, and leave `test_main` so-called.
This is the solution currently chosen.
One can test this by running `make clean`, `make O=3` to build with optimisation level 3 (for which the makefile enables LTO).
The program is generated, but not the test binary in this case.
Tests must be accessed via some interface (not currently implemented) in the program binary since there is no test binary.

## Demo

This is demonstrated with a simple makefile build system.

Running "make" will:

 * compile all the C files

 * link all the objects together to form the program binary (which includes the tests unless eliminated by the linker)

 * copy `test/test.o` to `test_main.o`, renaming symbol `test_main` to `main`

 * link all the objects excluding `main.o`, `test/test.o` and instead including `test_main.o`, to produce the test-only binary.

One can then run the default output image `bin/program` which is the software intended to run on the target and which includes the test runner (via `test_shell` in the case of this demo).

One can also run the test-runner image `bin/test` instead, which contains only the test suites.

Since the `objcopy` symbol redefinition will not work if LTO is enabled, the build system simply will not build a test image if LTO is enabled.

## Convention

### Define your test

`test/my_test.c`

```c
#include "test.h"

TEST_DEFINE(my_test)
{
	test_assert("assertion name", conditon == true);
	test_assert("another assertion name", ...);
}
```

### Let the runner know about your test

The order of tests in the XMACRO is the order in which they are run.
A test may be specified multiple times e.g. if it is part of a sequence of operations where it must be run multiple times.

`test/test.h`

```c
#define TEST_LIST_XMACRO \
	X(my_test) \
	X(...) \
	X(...)

```

### Build

```sh
make
```

### Run

`./bin/program` (if program provides access to the test runner e.g. via a CLI).

`./bin/test` (to run all tests)

`./bin/test test-name-1 [test-name-2] ...` (to run specific tests, by name)
