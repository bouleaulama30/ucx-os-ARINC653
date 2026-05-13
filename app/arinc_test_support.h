#ifndef ARINC_TEST_SUPPORT_H
#define ARINC_TEST_SUPPORT_H

typedef struct {
	const char *suite_name;
	unsigned int total_checks;
	unsigned int passed_checks;
} arinc_test_suite_result_t;

static inline void arinc_test_suite_begin(arinc_test_suite_result_t *result, const char *suite_name)
{
	result->suite_name = suite_name;
	result->total_checks = 0;
	result->passed_checks = 0;
	printf("\n--- START SUITE %s ---\n", suite_name);
}

static inline void arinc_test_suite_check(arinc_test_suite_result_t *result, const char *case_name, int passed)
{
	result->total_checks++;
	if (passed) {
		result->passed_checks++;
	} else {
		printf("[ARINC_TEST] suite=%s case=%s result=FAIL\n",
		       result->suite_name,
		       case_name);
	}
}

static inline void arinc_test_suite_end(const arinc_test_suite_result_t *result)
{
	printf("--- END SUITE %s: %u/%u PASS ---\n\n",
	       result->suite_name,
	       result->passed_checks,
	       result->total_checks);
}

static inline int arinc_test_check_int(long expected, long actual)
{
	return expected == actual;
}

static inline int arinc_test_check_str(const char *expected, const char *actual)
{
	return strcmp(expected, actual) == 0;
}

#endif