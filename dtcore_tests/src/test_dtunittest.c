#include <stdio.h>
#include <string.h>

#include <dtcore/dterr.h>
#include <dtcore/dtunittest.h>

#include <dtcore_tests.h>

// ------------------------------------------------------------------------
// Example: simplest passing test using the ASSERT_TRUE pattern.
static dterr_t*
test_dtcore_dtunittest_example_basic_pass(void)
{
    dterr_t* dterr = NULL;

    DTUNITTEST_ASSERT_TRUE(1 == 1);

cleanup:
    if (dterr != NULL)
    {
        dterr = dterr_new(DTERR_ASSERTION, __LINE__, __FILE__, __func__, dterr, "unexpected assertion failure");
    }
    return dterr;
}

// ------------------------------------------------------------------------
// Example: asserting an expected error object and code using DTERR helper.
static dterr_t*
test_dtcore_dtunittest_example_expected_error_macro(void)
{
    dterr_t* dterr = NULL;

    dterr_t* expected = dterr_new(DTERR_RANGE, DTERR_LOC, NULL, "range check");
    DTUNITTEST_ASSERT_DTERR(expected, DTERR_RANGE);

cleanup:
    if (dterr != NULL)
    {
        dterr = dterr_new(DTERR_ASSERTION, __LINE__, __FILE__, __func__, dterr, "unexpected assertion failure");
    }
    return dterr;
}

// ------------------------------------------------------------------------
// Example: byte-string equality check for compact binary assertions.
static dterr_t*
test_dtcore_dtunittest_example_equal_bytes(void)
{
    dterr_t* dterr = NULL;

    const unsigned char a[] = { 0xDE, 0xAD, 0xBE, 0xEF };
    const unsigned char b[] = { 0xDE, 0xAD, 0xBE, 0xEF };
    DTUNITTEST_ASSERT_EQUAL_BYTES(a, b, sizeof(a));

cleanup:
    if (dterr != NULL)
    {
        dterr = dterr_new(DTERR_ASSERTION, __LINE__, __FILE__, __func__, dterr, "unexpected assertion failure");
    }
    return dterr;
}

// ------------------------------------------------------------------------
// Failing path: ASSERT_TRUE should report failure and produce an error.
static dterr_t*
test_dtcore_dtunittest_01_assert_true_reports_failure(void)
{
    dterr_t* dterr = NULL;

    DTUNITTEST_ASSERT_TRUE(1 == 3);

cleanup:
    if (dterr == NULL)
    {
        dterr = dterr_new(DTERR_ASSERTION, __LINE__, __FILE__, __func__, NULL, "expected an assertion failure");
    }
    else
    {
        dterr_dispose(dterr);
        dterr = NULL;
    }
    return dterr;
}

// ------------------------------------------------------------------------
// Failing path: INT comparator should format a clear diagnostic message.
static dterr_t*
test_dtcore_dtunittest_02_assert_int_formats_message(void)
{
    dterr_t* dterr = NULL;

    int a = 1;
    int b = 2;
    DTUNITTEST_ASSERT_INT(a, ==, b);

cleanup:
    if (dterr == NULL)
    {
        dterr = dterr_new(DTERR_ASSERTION, __LINE__, __FILE__, __func__, NULL, "expected an assertion failure");
    }
    else
    {
        const char* expected = "assertion failed: got a=1 but expected == 2";
        if (strcmp(dterr->message, expected) != 0)
        {
            dterr = dterr_new(DTERR_ASSERTION,
              __LINE__,
              __FILE__,
              __func__,
              NULL,
              "expected message \"%s\" but got \"%s\"",
              expected,
              dterr->message);
        }
        else
        {
            dterr_dispose(dterr);
            dterr = NULL;
        }
    }
    return dterr;
}

// ------------------------------------------------------------------------
// Failing path: string equality should show both strings in the message.
static dterr_t*
test_dtcore_dtunittest_03_assert_equal_string_formats_message(void)
{
    dterr_t* dterr = NULL;

    char* a = (char*)"abc";
    char* b = (char*)"def";
    DTUNITTEST_ASSERT_EQUAL_STRING(a, b);

cleanup:
    if (dterr == NULL)
    {
        dterr = dterr_new(DTERR_ASSERTION, __LINE__, __FILE__, __func__, NULL, "expected an assertion failure");
    }
    else
    {
        const char* expected = "assertion failed: got a=\"abc\" but expected == \"def\"";
        if (strcmp(dterr->message, expected) != 0)
        {
            dterr = dterr_new(DTERR_ASSERTION,
              __LINE__,
              __FILE__,
              __func__,
              NULL,
              "expected message \"%s\" but got \"%s\"",
              expected,
              dterr->message);
        }
        else
        {
            dterr_dispose(dterr);
            dterr = NULL;
        }
    }
    return dterr;
}

// ------------------------------------------------------------------------
// Passing path: ASSERT_TRUE should not create errors.
static dterr_t*
test_dtcore_dtunittest_10_assert_true_passes(void)
{
    dterr_t* dterr = NULL;

    DTUNITTEST_ASSERT_TRUE(1 == 1);

cleanup:
    if (dterr != NULL)
    {
        dterr = dterr_new(DTERR_ASSERTION, __LINE__, __FILE__, __func__, dterr, "unexpected assertion failure");
    }
    return dterr;
}

// ------------------------------------------------------------------------
// Passing path: INT comparator equals.
static dterr_t*
test_dtcore_dtunittest_11_assert_int_passes(void)
{
    dterr_t* dterr = NULL;

    int a = 1;
    int b = 1;
    DTUNITTEST_ASSERT_INT(a, ==, b);

cleanup:
    if (dterr != NULL)
    {
        dterr = dterr_new(DTERR_ASSERTION, __LINE__, __FILE__, __func__, dterr, "unexpected assertion failure");
    }
    return dterr;
}

// ------------------------------------------------------------------------
// Passing path: string equality comparator equals.
static dterr_t*
test_dtcore_dtunittest_12_assert_equal_string_passes(void)
{
    dterr_t* dterr = NULL;

    char* a = (char*)"abc";
    char* b = (char*)"abc";
    DTUNITTEST_ASSERT_EQUAL_STRING(a, b);

cleanup:
    if (dterr != NULL)
    {
        dterr = dterr_new(DTERR_ASSERTION, __LINE__, __FILE__, __func__, dterr, "unexpected assertion failure");
    }
    return dterr;
}

// ------------------------------------------------------------------------
void
test_dtcore_dtunittest(DTUNITTEST_SUITE_ARGS)
{
    DTUNITTEST_RUN_TEST(test_dtcore_dtunittest_example_basic_pass);
    DTUNITTEST_RUN_TEST(test_dtcore_dtunittest_example_expected_error_macro);
    DTUNITTEST_RUN_TEST(test_dtcore_dtunittest_example_equal_bytes);

    DTUNITTEST_RUN_TEST(test_dtcore_dtunittest_01_assert_true_reports_failure);
    DTUNITTEST_RUN_TEST(test_dtcore_dtunittest_02_assert_int_formats_message);
    DTUNITTEST_RUN_TEST(test_dtcore_dtunittest_03_assert_equal_string_formats_message);

    DTUNITTEST_RUN_TEST(test_dtcore_dtunittest_10_assert_true_passes);
    DTUNITTEST_RUN_TEST(test_dtcore_dtunittest_11_assert_int_passes);
    DTUNITTEST_RUN_TEST(test_dtcore_dtunittest_12_assert_equal_string_passes);
}
