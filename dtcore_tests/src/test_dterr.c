#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <dtcore/dterr.h>
#include <dtcore/dtunittest.h>
#include <dtcore_tests.h>

#define ASSERT_TRUE(ASSERTION)                                                                                                 \
    if (!(ASSERTION))                                                                                                          \
    {                                                                                                                          \
        printf("assertion failed: %s (file: %s, line: %d, func: %s)\n", #ASSERTION, __FILE__, __LINE__, __func__);             \
        goto cleanup;                                                                                                          \
    }

/* Helpers for dterr_each() examples */
typedef struct
{
    int codes[16];
    int count;
} error_collection_t;

static void
collect_error_code(dterr_t* err, void* context)
{
    error_collection_t* collection = (error_collection_t*)context;
    collection->codes[collection->count++] = err->error_code;
}

static void
count_error_calls(dterr_t* err, void* context)
{
    (void)err;
    int* counter = (int*)context;
    (*counter)++;
}

/* Small helper used by the guard-and-cleanup example. */
static dterr_t*
example_checked_op(void* must_not_be_null)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(must_not_be_null);
    /* pretend to do work here */
cleanup:
    return dterr;
}

/* Helpers to exercise DTERR_C traceback chaining. */
static dterr_t*
leaf_badarg_fail(void)
{
    return dterr_new(DTERR_BADARG, DTERR_LOC, NULL, "leaf: arg invalid");
}

static dterr_t*
mid_wrap_traceback(void)
{
    dterr_t* dterr = NULL;
    DTERR_C(leaf_badarg_fail());
cleanup:
    return dterr;
}

static dterr_t*
top_wrap_traceback(void)
{
    dterr_t* dterr = NULL;
    DTERR_C(mid_wrap_traceback());
cleanup:
    return dterr;
}

// ------------------------------------------------------------------------
// EXAMPLES (short, readable, self-contained)
// ------------------------------------------------------------------------

static dterr_t*
test_dtcore_dterr_example_guard_and_cleanup(void)
{
    /* Shows DTERR_ASSERT_NOT_NULL + cleanup pattern. */
    dterr_t* dterr = example_checked_op(NULL);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_ARGUMENT_NULL); /* disposes and NULLs it */

cleanup:
    return NULL;
}

// ------------------------------------------------------------------------

static dterr_t*
test_dtcore_dterr_example_chain_logging(void)
{
    /* Build a causal chain and walk it inner -> outer. */
    error_collection_t seen = { .count = 0 };
    dterr_t* inner = dterr_new(111, DTERR_LOC, NULL, "inner %d", 1);
    dterr_t* outer = dterr_new(222, DTERR_LOC, inner, "outer");

    dterr_each(outer, collect_error_code, &seen);

    ASSERT_TRUE(seen.count == 2);
    ASSERT_TRUE(seen.codes[0] == 111);
    ASSERT_TRUE(seen.codes[1] == 222);

    dterr_dispose(outer);

cleanup:
    return NULL;
}

// ------------------------------------------------------------------------

static dterr_t*
test_dtcore_dterr_example_append_tail(void)
{
    /* Append a separate chain onto another and verify order. */
    error_collection_t seen = { .count = 0 };

    dterr_t* a1 = dterr_new(10, DTERR_LOC, NULL, "A1");
    dterr_t* a2 = dterr_new(20, DTERR_LOC, a1, "A2"); /* chain A: 10 -> 20 (outer=a2) */

    dterr_t* b1 = dterr_new(30, DTERR_LOC, NULL, "B1"); /* chain B: 30 (outer=b1) */

    dterr_append(a2, b1); /* result: 10 -> 20 -> 30 (outer=a2) */

    dterr_each(a2, collect_error_code, &seen);

    ASSERT_TRUE(seen.count == 3);
    ASSERT_TRUE(seen.codes[0] == 30);
    ASSERT_TRUE(seen.codes[1] == 10);
    ASSERT_TRUE(seen.codes[2] == 20);

    dterr_dispose(a2);

cleanup:
    return NULL;
}

// ------------------------------------------------------------------------
// CORE TESTS (retain and clarify coverage)
// ------------------------------------------------------------------------

static dterr_t*
test_dtcore_dterr_01_new_basic(void)
{
    const int error_code = 100;
    const int line = __LINE__;
    const char* file = __FILE__;
    const char* function = __func__;
    const char* expected_message = "Test error message";

    dterr_t* err = dterr_new(error_code, line, file, function, NULL, "%s", expected_message);
    assert(err != NULL);
    assert(err->error_code == error_code);
    assert(err->line_number == line);
    assert(err->source_file == file);
    assert(err->source_function == function);
    assert(err->inner_err == NULL);
    assert(strcmp(err->message, expected_message) == 0);

    ASSERT_TRUE(dterr_ledger->count_balance == 1);
    ASSERT_TRUE(dterr_ledger->count_high_water_mark == 1);
    ASSERT_TRUE(dterr_ledger->count_low_water_mark == 0);

    dterr_dispose(err);

    ASSERT_TRUE(dterr_ledger->count_balance == 0);
    ASSERT_TRUE(dterr_ledger->count_high_water_mark == 1);
    ASSERT_TRUE(dterr_ledger->count_low_water_mark == 0);

cleanup:
    return NULL;
}

// ------------------------------------------------------------------------

static dterr_t*
test_dtcore_dterr_02_new_nested(void)
{
    const int error_code = 100;
    const int line = __LINE__;
    const char* file = __FILE__;
    const char* function = __func__;
    const char* expected_message = "Test error message";
    const char* expected_inner_message = "Inner error message";

    dterr_t* inner_err = dterr_new(error_code, line, file, function, NULL, "%s", expected_inner_message);
    assert(inner_err != NULL);

    dterr_t* err = dterr_new(error_code, line, file, function, inner_err, "%s", expected_message);
    assert(err != NULL);
    assert(err->inner_err == inner_err);
    assert(strcmp(err->message, expected_message) == 0);

    dterr_dispose(err);

    return NULL;
}

// ------------------------------------------------------------------------

static dterr_t*
test_dtcore_dterr_03_new_formatting(void)
{
    const int error_code = 101;
    const char* expected_message = "Value: 42, string: hello";

    dterr_t* err = dterr_new(
      error_code, __LINE__, "test_file.c", "test_dtcore_dterr_03_new_formatting", NULL, "Value: %d, string: %s", 42, "hello");
    assert(err != NULL);
    assert(strcmp(err->message, expected_message) == 0);

    dterr_dispose(err);
    return NULL;
}

// ------------------------------------------------------------------------

static dterr_t*
test_dtcore_dterr_04_new_with_inner_error(void)
{
    dterr_t* inner_err = dterr_new(200, __LINE__, "inner_file.c", "inner_function", NULL, "Inner error occurred");
    assert(inner_err != NULL);

    dterr_t* outer_err =
      dterr_new(201, __LINE__, "outer_file.c", "outer_function", inner_err, "Outer error wrapping inner error");
    assert(outer_err != NULL);
    assert(outer_err->inner_err == inner_err);

    dterr_dispose(outer_err);
    return NULL;
}

// ------------------------------------------------------------------------

static dterr_t*
test_dtcore_dterr_05_dispose_function_pointer(void)
{
    dterr_t* err = dterr_new(
      300, __LINE__, "file.c", "test_dtcore_dterr_05_dispose_function_pointer", NULL, "Dispose function pointer test");
    assert(err != NULL);
    assert(err->dispose != NULL);
    assert(err->dispose == dterr_dispose);

    err->dispose(err);
    return NULL;
}

// ------------------------------------------------------------------------

static dterr_t*
test_dtcore_dterr_06_new_null_parameters(void)
{
    dterr_t* err = dterr_new(400, __LINE__, NULL, NULL, NULL, "No file or function provided");
    assert(err != NULL);
    assert(err->source_file == NULL);
    assert(err->source_function == NULL);

    dterr_dispose(err);
    return NULL;
}

// ------------------------------------------------------------------------

static dterr_t*
test_dtcore_dterr_07_new_nested_inner_errors(void)
{
    dterr_t* err1 = dterr_new(1, __LINE__, "file1.c", "func1", NULL, "Level 1 error");
    dterr_t* err2 = dterr_new(2, __LINE__, "file2.c", "func2", err1, "Level 2 error");
    dterr_t* err3 = dterr_new(3, __LINE__, "file3.c", "func3", err2, "Level 3 error");

    assert(err3->inner_err == err2);
    assert(err2->inner_err == err1);

    dterr_dispose(err3);
    return NULL;
}

// ------------------------------------------------------------------------

static dterr_t*
test_dtcore_dterr_08_each_single_error(void)
{
    error_collection_t collection = { .count = 0 };
    dterr_t* err = dterr_new(123, __LINE__, "test_file.c", "test_dtcore_dterr_08_each_single_error", NULL, "Single error test");

    dterr_each(err, collect_error_code, &collection);

    assert(collection.count == 1);
    assert(collection.codes[0] == 123);

    dterr_dispose(err);
    return NULL;
}

// ------------------------------------------------------------------------

static dterr_t*
test_dtcore_dterr_09_each_nested_errors(void)
{
    error_collection_t collection = { .count = 0 };

    dterr_t* inner_err = dterr_new(111, __LINE__, "inner_file.c", "inner_function", NULL, "Inner error");
    dterr_t* outer_err =
      dterr_new(222, __LINE__, "outer_file.c", "outer_function", inner_err, "Outer error wrapping inner error");

    dterr_each(outer_err, collect_error_code, &collection);

    assert(collection.count == 2);
    assert(collection.codes[0] == 111);
    assert(collection.codes[1] == 222);

    dterr_dispose(outer_err);
    return NULL;
}

// ------------------------------------------------------------------------

static dterr_t*
test_dtcore_dterr_10_each_three_errors(void)
{
    error_collection_t collection = { .count = 0 };

    dterr_t* err1 = dterr_new(10, __LINE__, "file1.c", "func1", NULL, "Level 1 error");
    dterr_t* err2 = dterr_new(20, __LINE__, "file2.c", "func2", err1, "Level 2 error");
    dterr_t* err3 = dterr_new(30, __LINE__, "file3.c", "func3", err2, "Level 3 error");

    dterr_each(err3, collect_error_code, &collection);

    assert(collection.count == 3);
    assert(collection.codes[0] == 10);
    assert(collection.codes[1] == 20);
    assert(collection.codes[2] == 30);

    dterr_dispose(err3);
    return NULL;
}

// ------------------------------------------------------------------------

static dterr_t*
test_dtcore_dterr_11_each_counter(void)
{
    int counter = 0;
    dterr_t* err1 = dterr_new(101, __LINE__, "file1.c", "func1", NULL, "Error 1");
    dterr_t* err2 = dterr_new(202, __LINE__, "file2.c", "func2", err1, "Error 2");

    dterr_each(err2, count_error_calls, &counter);
    assert(counter == 2);

    dterr_dispose(err2);
    return NULL;
}

// ------------------------------------------------------------------------

static dterr_t*
test_dtcore_dterr_12_new_null_format_returns_null(void)
{
    /* dterr_new returns NULL when format is NULL (per implementation). */
    dterr_t* err = dterr_new(500, DTERR_LOC, NULL, NULL, NULL, NULL);
    assert(err == NULL);
    return NULL;
}

// ------------------------------------------------------------------------

static dterr_t*
test_dtcore_dterr_13_append_prevents_cycle_self(void)
{
    /* Appending a node to itself should be a no-op (cycle prevention). */
    error_collection_t seen = { .count = 0 };
    dterr_t* one = dterr_new(77, DTERR_LOC, NULL, "one");

    /* This would create a cycle if not guarded; subsequent traversal would hang. */
    dterr_append(one, one);

    dterr_each(one, collect_error_code, &seen);
    assert(seen.count == 1);
    assert(seen.codes[0] == 77);

    dterr_dispose(one);
    return NULL;
}

// ------------------------------------------------------------------------

static dterr_t*
test_dtcore_dterr_14_traceback_chaining_DTERR_C(void)
{
    /* Demonstrates traceback wrapping created by DTERR_C at each propagation site. */
    dterr_t* dterr = top_wrap_traceback();

    /* Expect a failure with code DTERR_BADARG; do not dispose yet—inspect chain first. */
    assert(dterr != NULL);
    assert(dterr->error_code == DTERR_BADARG);
    assert(strcmp(dterr->message, "traceback") == 0);
    assert(dterr->source_function && strcmp(dterr->source_function, "top_wrap_traceback") == 0);

    /* inner 1: created by DTERR_C inside mid_wrap_traceback */
    dterr_t* inner1 = dterr->inner_err;
    assert(inner1 != NULL);
    assert(inner1->error_code == DTERR_BADARG);
    assert(strcmp(inner1->message, "traceback") == 0);
    assert(inner1->source_function && strcmp(inner1->source_function, "mid_wrap_traceback") == 0);

    /* inner 2: the original leaf error from leaf_badarg_fail */
    dterr_t* inner2 = inner1->inner_err;
    assert(inner2 != NULL);
    assert(inner2->error_code == DTERR_BADARG);
    assert(inner2->source_function && strcmp(inner2->source_function, "leaf_badarg_fail") == 0);
    assert(strstr(inner2->message, "leaf: arg invalid") != NULL);

    /* And nothing further. */
    assert(inner2->inner_err == NULL);

    dterr_dispose(dterr);
    return NULL;
}

// --------------------------------------------------------------------------------------------
// suite runner
void
test_dtcore_dterr(DTUNITTEST_SUITE_ARGS)
{
    /* Examples first */
    DTUNITTEST_RUN_TEST(test_dtcore_dterr_example_guard_and_cleanup);
    DTUNITTEST_RUN_TEST(test_dtcore_dterr_example_chain_logging);
    DTUNITTEST_RUN_TEST(test_dtcore_dterr_example_append_tail);

    /* Numbered suite */
    DTUNITTEST_RUN_TEST(test_dtcore_dterr_01_new_basic);
    DTUNITTEST_RUN_TEST(test_dtcore_dterr_02_new_nested);
    DTUNITTEST_RUN_TEST(test_dtcore_dterr_03_new_formatting);
    DTUNITTEST_RUN_TEST(test_dtcore_dterr_04_new_with_inner_error);
    DTUNITTEST_RUN_TEST(test_dtcore_dterr_05_dispose_function_pointer);
    DTUNITTEST_RUN_TEST(test_dtcore_dterr_06_new_null_parameters);
    DTUNITTEST_RUN_TEST(test_dtcore_dterr_07_new_nested_inner_errors);
    DTUNITTEST_RUN_TEST(test_dtcore_dterr_08_each_single_error);
    DTUNITTEST_RUN_TEST(test_dtcore_dterr_09_each_nested_errors);
    DTUNITTEST_RUN_TEST(test_dtcore_dterr_10_each_three_errors);
    DTUNITTEST_RUN_TEST(test_dtcore_dterr_11_each_counter);
    DTUNITTEST_RUN_TEST(test_dtcore_dterr_12_new_null_format_returns_null);
    DTUNITTEST_RUN_TEST(test_dtcore_dterr_13_append_prevents_cycle_self);
    DTUNITTEST_RUN_TEST(test_dtcore_dterr_14_traceback_chaining_DTERR_C);
}
