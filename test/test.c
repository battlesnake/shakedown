#include "test.h"
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

/* Generate list of test references */
#define X(name) extern struct TestSuite test_suite_##name;
TEST_LIST_XMACRO
#undef X

/* Generate list of tests */
#define X(name) &test_suite_##name,
struct TestSuite *test_suites[] = {
	TEST_LIST_XMACRO
	NULL
};
#undef X

static struct TestSuite *current_suite = NULL;

bool test_log_quiet = false;

static void test_counter_reset(struct TestCounter *counter)
{
	counter->total = 0;
	counter->pass = 0;
}

static void test_counter_write(struct TestCounter *counter, bool value)
{
	if (value) {
		counter->pass++;
	}
	counter->total++;
}

static void test_suite_reset(struct TestSuite *suite)
{
	test_counter_reset(&suite->counter);
}

void test_suite_run(struct TestSuite *suite)
{
	test_suite_reset(suite);
	current_suite = suite;
	suite->func(suite->arg);
	current_suite = NULL;
}

bool test_suites_run_all(bool stop_on_error)
{
	unsigned suite_total = 0;
	unsigned suite_pass = 0;
	unsigned suite_skip = 0;
	unsigned suite_fail = 0;
	unsigned total = 0;
	unsigned pass = 0;
	bool skip_all = false;
	test_log_nl();
	for (struct TestSuite **it = test_suites; *it != NULL; ++it) {
		struct TestSuite *suite = *it;
		suite_total++;
		if (suite->skip || skip_all) {
			test_log(TEST_ANSI_BOLD(TEST_ANSI_STRIKE("%s")) TEST_ANSI_FG_YELLOW(" - skipped"), suite->name);
			test_log_nl();
			suite_skip++;
			continue;
		}
		test_log(TEST_ANSI_BOLD("%s"), suite->name);
		test_suite_run(suite);
		test_log_nl();
		total += suite->counter.total;
		pass += suite->counter.pass;
		if (suite->counter.total == suite->counter.pass) {
			suite_pass++;
		} else {
			suite_fail++;
		}
		if (stop_on_error && pass < total) {
			test_log(TEST_ANSI_FG_RED("Suite failed, bailing"));
			skip_all = true;
		}
	}
	test_log("%u cases, " TEST_ANSI_FG_GREEN("%u") " passed, " TEST_ANSI_FG_RED("%u") " failed", total, pass, total - pass);
	test_log("%u suites, " TEST_ANSI_FG_GREEN("%u") " passed, " TEST_ANSI_FG_RED("%u") " failed, " TEST_ANSI_FG_YELLOW("%u") " skipped", suite_total, suite_pass, suite_fail, suite_skip);
	test_log_nl();
	return suite_fail == 0;
}

void _test_pass(const char *file, const int line, const char *func, const char *name, const char *description)
{
	if (current_suite) {
		test_counter_write(&current_suite->counter, true);
	}
	if (!test_log_quiet) {
		_test_log(file, line, func, " %s %s", TEST_PASS_SYMBOL, name);
	}
}

void _test_fail(const char *file, const int line, const char *func, const char *name, const char *description)
{
	if (current_suite) {
		test_counter_write(&current_suite->counter, false);
	}
	_test_log(file, line, func, " %s %s", TEST_FAIL_SYMBOL, name);
	_test_log(file, line, func, "   " TEST_ANSI_FG_YELLOW("%s"), description);
}

bool _test_assert(const char *file, const int line, const char *func, const char *name, const char *expression, bool value)
{
	if (value) {
		_test_pass(file, line, func, name, expression);
	} else {
		_test_fail(file, line, func, name, expression);
	}
	return value;
}

static const char *shorten_filename(const char *file)
{
#if TEST_LOG_SHORT_FILENAME > 0
	const size_t length = strlen(file);
	if (length <= 1) {
		return file;
	}
	unsigned parts = 0;
	for (const char *it = file + length - 1; it != file; --it) {
		if (*it == '/') {
			if (++parts == TEST_LOG_SHORT_FILENAME) {
				return it + 1;
			}
		}
	}
	return file;
#else
	return file;
#endif
}

static const char *remove_ext(const char *file)
{
	const size_t length = strlen(file);
	if (length <= 1) {
		return file;
	}
	const char *end = file + length;
	for (const char *it = end - 1; it != file; --it) {
		if (*it == '.') {
			return it;
		}
	}
	return end;
}

#if ! defined TEST_LOG_CUSTOM

static void print_prefix(const char *format, const char *file, const int line, const char *func)
{
	const char *f_begin = shorten_filename(file);
	const char *f_end = remove_ext(file);
	const size_t f_len = f_end - f_begin;
	char f_buf[f_len + 1];
	snprintf(f_buf, f_len + 1, "%s", f_begin);
	fprintf(TEST_LOG_TARGET, format, f_buf, line, func);
}

void _test_log(const char *file, const int line, const char *func, const char *format, ...)
{
	print_prefix(TEST_LOG_FORMAT_STR, file, line, func);

	va_list ap;
	va_start(ap, format);
	vfprintf(TEST_LOG_TARGET, format, ap);
	va_end(ap);

	fprintf(TEST_LOG_TARGET, "\n");
}

void _test_log_nl(const char *file, const int line, const char *func)
{
	fprintf(TEST_LOG_TARGET, "\n");
}

#endif

void _test_error(const char *file, const int line, const char *func, const char *format, ...)
{
	print_prefix(TEST_LOG_ERROR_FORMAT_STR, file, line, func);

	va_list ap;
	va_start(ap, format);
	vfprintf(TEST_LOG_TARGET, format, ap);
	va_end(ap);

	fprintf(TEST_LOG_TARGET, "\n");
}

struct TestSuite *test_find_by_name(const char *name)
{
	for (struct TestSuite **it = test_suites; *it != NULL; ++it) {
		struct TestSuite *suite = *it;
		if (strcmp(name, suite->name) == 0) {
			return suite;
		}
	}
	return NULL;
}

void test_checklist_print(const char *title)
{
	printf("\n");
	printf(TEST_ANSI_BOLD("%s") "\n", title);
	for (struct TestSuite **it = test_suites; *it != NULL; ++it) {
		const unsigned idx = it - test_suites;
		struct TestSuite *suite = *it;
		const char *symbol;
		if (suite->counter.total) {
			if (suite->counter.pass == suite->counter.total) {
				symbol = TEST_PASS_SYMBOL;
			} else {
				symbol = TEST_FAIL_SYMBOL;
			}
		} else {
			symbol = TEST_SKIP_SYMBOL;
		}
		printf(" %3u. [%s] ", idx, symbol);
		if (suite->skip) {
			printf(TEST_ANSI_DARK(TEST_ANSI_STRIKE("%s")), suite->name);
		} else {
			printf("%s", suite->name);
		}
		printf("\n");
	}
	printf("\n");
}

void test_checklist_clear(int index)
{
	if (index == -1) {
		for (int i = 0; test_suites[i] != NULL; ++i) {
			test_checklist_clear(i);
		}
	} else {
		test_counter_reset(&test_suites[index]->counter);
	}
}

void test_checklist_select(int index)
{
	if (index == -1) {
		for (int i = 0; test_suites[i] != NULL; ++i) {
			test_checklist_select(i);
		}
	} else {
		test_suites[index]->skip = false;
	}
}

void test_checklist_deselect(int index)
{
	if (index == -1) {
		for (int i = 0; test_suites[i] != NULL; ++i) {
			test_checklist_deselect(i);
		}
	} else {
		test_suites[index]->skip = true;
	}
}

int test_main(int argc, char *argv[])
{
	/* Select all tests if none were specified; otherwise, deselect all */
	for (struct TestSuite **it = test_suites; *it != NULL; ++it) {
		struct TestSuite *suite = *it;
		suite->skip = argc > 1;
	}
	/* Select named tests */
	for (int i = 1; i < argc; ++i) {
		const char *name = argv[i];
		struct TestSuite *suite = test_find_by_name(name);
		if (suite == NULL) {
			test_error("Test \"%s\" does not exist", name);
		}
		suite->skip = false;
	}
	/* Run all unskipped tests */
	test_checklist_print("Checklist pre-run");
	int result = test_suites_run_all(getenv("STOP_ON_ERROR") != NULL) ? 0 : 1;
	test_checklist_print("Checklist post-run");
	return result;
}
