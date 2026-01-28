#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dterr.h>
#include <dtcore/dtstr.h>
#include <dtcore/dtunittest.h>
#include <dtcore_tests.h>

// ------------------------------------------------------------------------
// Example: Small, readable "happy path" showing basic formatting & cleanup.
static dterr_t*
test_dtcore_dtstr_example_basic_formatting(void)
{
    dterr_t* dterr = NULL;

    char* s = dtstr_format("Hello, %s! You are %d.", "Alice", 7);
    DTUNITTEST_ASSERT_NOT_NULL(s);
    DTUNITTEST_ASSERT_EQUAL_STRING(s, "Hello, Alice! You are 7.");

cleanup:
    dtstr_dispose(s);
    return dterr;
}

// ------------------------------------------------------------------------
// Example: Building a message incrementally with a separator (append pattern).
static dterr_t*
test_dtcore_dtstr_example_concat_append_pattern(void)
{
    dterr_t* dterr = NULL;

    char* s = dtstr_format("name=%s", "sensorA");
    DTUNITTEST_ASSERT_NOT_NULL(s);

    s = dtstr_concat_format(s, ", ", "value=%d", 42);
    DTUNITTEST_ASSERT_NOT_NULL(s);

    s = dtstr_concat_format(s, ", ", "status=%s", "ok");
    DTUNITTEST_ASSERT_NOT_NULL(s);

    DTUNITTEST_ASSERT_EQUAL_STRING(s, "name=sensorA, value=42, status=ok");

cleanup:
    dtstr_dispose(s);
    return dterr;
}

// ------------------------------------------------------------------------
// Example: Using a small wrapper to pass a va_list (common gotcha).
static char*
local_va_wrapper(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char* out = dtstr_format_va(fmt, ap);
    va_end(ap);
    return out;
}

static dterr_t*
test_dtcore_dtstr_example_va_wrapper(void)
{
    dterr_t* dterr = NULL;

    char* s = local_va_wrapper("[%s:%d]=%.1f", "chan", 3, 2.5);
    DTUNITTEST_ASSERT_NOT_NULL(s);
    DTUNITTEST_ASSERT_EQUAL_STRING(s, "[chan:3]=2.5");

cleanup:
    dtstr_dispose(s);
    return dterr;
}

// ------------------------------------------------------------------------
// Example: Safe duplication + disposal pattern.
static dterr_t*
test_dtcore_dtstr_example_dup_and_dispose(void)
{
    dterr_t* dterr = NULL;

    const char* src = "immutable";
    char* dup = dtstr_dup(src);
    DTUNITTEST_ASSERT_NOT_NULL(dup);
    DTUNITTEST_ASSERT_EQUAL_STRING(dup, "immutable");

cleanup:
    dtstr_dispose(dup);
    // also verify that disposing NULL is a no-op (should not crash)
    dtstr_dispose(NULL);
    return dterr;
}

// ------------------------------------------------------------------------
// An empty format string should produce an empty string.
static dterr_t*
test_dtcore_dtstr_empty_format(void)
{
    dterr_t* dterr = NULL;

    char* s = dtstr_format("");
    DTUNITTEST_ASSERT_NOT_NULL(s);
    DTUNITTEST_ASSERT_EQUAL_STRING(s, "");

cleanup:
    dtstr_dispose(s);
    return dterr;
}

// ------------------------------------------------------------------------
// A format string with no specifiers should return an identical copy.
static dterr_t*
test_dtcore_dtstr_simple_string(void)
{
    dterr_t* dterr = NULL;

    const char* input = "Hello, world!";
    char* s = dtstr_format(input);
    DTUNITTEST_ASSERT_NOT_NULL(s);
    DTUNITTEST_ASSERT_EQUAL_STRING(s, input);

cleanup:
    dtstr_dispose(s);
    return dterr;
}

// ------------------------------------------------------------------------
// A format string with one string specifier.
static dterr_t*
test_dtcore_dtstr_string_format(void)
{
    dterr_t* dterr = NULL;

    char* s = dtstr_format("Hello, %s!", "Alice");
    DTUNITTEST_ASSERT_NOT_NULL(s);
    DTUNITTEST_ASSERT_EQUAL_STRING(s, "Hello, Alice!");

cleanup:
    dtstr_dispose(s);
    return dterr;
}

// ------------------------------------------------------------------------
// A format string with an integer specifier.
static dterr_t*
test_dtcore_dtstr_integer_format(void)
{
    dterr_t* dterr = NULL;

    char* s = dtstr_format("Number: %d", 42);
    DTUNITTEST_ASSERT_NOT_NULL(s);
    DTUNITTEST_ASSERT_EQUAL_STRING(s, "Number: 42");

cleanup:
    dtstr_dispose(s);
    return dterr;
}

// ------------------------------------------------------------------------
// A format string with a floating point specifier.
static dterr_t*
test_dtcore_dtstr_floating_point_format(void)
{
    dterr_t* dterr = NULL;

    char* s = dtstr_format("Pi: %.2f", 3.14159);
    DTUNITTEST_ASSERT_NOT_NULL(s);
    DTUNITTEST_ASSERT_EQUAL_STRING(s, "Pi: 3.14");

cleanup:
    dtstr_dispose(s);
    return dterr;
}

// ------------------------------------------------------------------------
// A format string with multiple specifiers.
static dterr_t*
test_dtcore_dtstr_multiple_specifiers(void)
{
    dterr_t* dterr = NULL;

    char* s = dtstr_format("Coordinates: (%d, %.1f)", 10, 3.2);
    DTUNITTEST_ASSERT_NOT_NULL(s);
    DTUNITTEST_ASSERT_EQUAL_STRING(s, "Coordinates: (10, 3.2)");

cleanup:
    dtstr_dispose(s);
    return dterr;
}

// ------------------------------------------------------------------------
// Verify that dtstr_format_va produces the expected result.
static dterr_t*
test_dtcore_dtstr_va_list_function(void)
{
    dterr_t* dterr = NULL;

    char* s = local_va_wrapper("Name: %s, Age: %d", "Bob", 25);
    DTUNITTEST_ASSERT_NOT_NULL(s);
    DTUNITTEST_ASSERT_EQUAL_STRING(s, "Name: Bob, Age: 25");

cleanup:
    dtstr_dispose(s);
    return dterr;
}

// ------------------------------------------------------------------------
// Verify that dtstr_concat_format with no existing string produces a new string.
static dterr_t*
test_dtcore_dtstr_cat_string(void)
{
    dterr_t* dterr = NULL;

    char* s = dtstr_concat_format(NULL, NULL, "Name: %s, Age: %d", "Bob", 25);
    DTUNITTEST_ASSERT_NOT_NULL(s);
    DTUNITTEST_ASSERT_EQUAL_STRING(s, "Name: Bob, Age: 25");

cleanup:
    dtstr_dispose(s);
    return dterr;
}

// ------------------------------------------------------------------------
// Verify that dtstr_concat_format appends to existing string with separator.
static dterr_t*
test_dtcore_dtstr_cat_string_with_separator(void)
{
    dterr_t* dterr = NULL;

    char* s = dtstr_format("Name: %s", "Bob");
    DTUNITTEST_ASSERT_NOT_NULL(s);

    s = dtstr_concat_format(s, ", ", "Age: %d", 25);
    DTUNITTEST_ASSERT_NOT_NULL(s);
    DTUNITTEST_ASSERT_EQUAL_STRING(s, "Name: Bob, Age: 25");

cleanup:
    dtstr_dispose(s);
    return dterr;
}

// ------------------------------------------------------------------------
// Verify that dtstr_concat_format appends with no separator when NULL is given.
static dterr_t*
test_dtcore_dtstr_cat_string_no_separator(void)
{
    dterr_t* dterr = NULL;

    char* s = dtstr_format("A");
    DTUNITTEST_ASSERT_NOT_NULL(s);

    s = dtstr_concat_format(s, NULL, "B");
    DTUNITTEST_ASSERT_NOT_NULL(s);
    DTUNITTEST_ASSERT_EQUAL_STRING(s, "AB");

cleanup:
    dtstr_dispose(s);
    return dterr;
}

// ------------------------------------------------------------------------
// Error path: dtstr_format(NULL) must return NULL.
static dterr_t*
test_dtcore_dtstr_format_null_returns_null(void)
{
    dterr_t* dterr = NULL;

    char* s = dtstr_format(NULL);
    DTUNITTEST_ASSERT_NULL(s);

cleanup:
    dtstr_dispose(s); // safe even if NULL
    return dterr;
}

// ------------------------------------------------------------------------
// Error path: dtstr_concat_format_va requires a non-NULL 'existing'.
static char*
local_concat_va_wrapper(char* existing, char* sep, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char* out = dtstr_concat_format_va(existing, sep, fmt, ap);
    va_end(ap);
    return out;
}

static dterr_t*
test_dtcore_dtstr_concat_va_existing_null_is_error(void)
{
    dterr_t* dterr = NULL;

    char* out = local_concat_va_wrapper(NULL, ", ", "x=%d", 1);
    DTUNITTEST_ASSERT_NULL(out);

cleanup:
    dtstr_dispose(out);
    return dterr;
}

// ------------------------------------------------------------------------
void
test_dtcore_dtstr(DTUNITTEST_SUITE_ARGS)
{
    // Examples first (teachable, self-contained).
    DTUNITTEST_RUN_TEST(test_dtcore_dtstr_example_basic_formatting);
    DTUNITTEST_RUN_TEST(test_dtcore_dtstr_example_concat_append_pattern);
    DTUNITTEST_RUN_TEST(test_dtcore_dtstr_example_va_wrapper);
    DTUNITTEST_RUN_TEST(test_dtcore_dtstr_example_dup_and_dispose);

    // Existing coverage (mainline & prior cases).
    DTUNITTEST_RUN_TEST(test_dtcore_dtstr_empty_format);
    DTUNITTEST_RUN_TEST(test_dtcore_dtstr_simple_string);
    DTUNITTEST_RUN_TEST(test_dtcore_dtstr_string_format);
    DTUNITTEST_RUN_TEST(test_dtcore_dtstr_integer_format);
    DTUNITTEST_RUN_TEST(test_dtcore_dtstr_floating_point_format);
    DTUNITTEST_RUN_TEST(test_dtcore_dtstr_multiple_specifiers);
    DTUNITTEST_RUN_TEST(test_dtcore_dtstr_va_list_function);
    DTUNITTEST_RUN_TEST(test_dtcore_dtstr_cat_string);
    DTUNITTEST_RUN_TEST(test_dtcore_dtstr_cat_string_with_separator);

    // Additional edge/error paths for completeness.
    DTUNITTEST_RUN_TEST(test_dtcore_dtstr_cat_string_no_separator);
    DTUNITTEST_RUN_TEST(test_dtcore_dtstr_format_null_returns_null);
    DTUNITTEST_RUN_TEST(test_dtcore_dtstr_concat_va_existing_null_is_error);
}
