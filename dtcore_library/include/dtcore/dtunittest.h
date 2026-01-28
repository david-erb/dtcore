#pragma once
// See markdown documentation at the end of this file.

// Minimal C unit-test harness: suites, tests, assertions, and error/report plumbing.

#include <sys/types.h>

#include <inttypes.h>
#include <math.h> // For fabs
#include <stdbool.h>

#include <dtcore/dterr.h>

//
//
// Most APIs take a pointer to this structure to share configuration,
// counters, and optional callbacks across a suite run.
//
typedef struct dtunittest_control_t dtunittest_control_t;

//  Hook points invoked around suite and test execution.
//
//
//
// Use to allocate suite-scoped fixtures or reset shared state.
//
typedef void (*dtunittest_suite_setup_f)(dtunittest_control_t* unittest_control);
//
//
// Use to tear down suite-scoped fixtures.
//
typedef void (*dtunittest_suite_teardown_f)(dtunittest_control_t* unittest_control);
typedef void (*dtunittest_test_setup_f)(dtunittest_control_t* unittest_control);
typedef void (*dtunittest_test_teardown_f)(dtunittest_control_t* unittest_control);

//
//
// Ownership: created/owned by the caller that orchestrates test runs.
// The runner updates pass/fail counts; your code can toggle printing and
// provide ledgers/callbacks for richer diagnostics.
//
typedef struct dtunittest_control_t
{
    const char* pattern;
    int suite_fail_count;
    int suite_pass_count;
    int total_fail_count;
    int total_pass_count;
    bool test_passed;

    bool should_print_tests;
    bool should_print_suites;
    bool should_print_errors;

    //
    //
    // Convention: a NULL-terminated array of `dtledger_t*`. When present, the runner
    // resets and checks each ledger to catch leaks or negative low-water marks.
    // The type is declared in the ledger module and referenced here to avoid a hard dependency.
    //
    dtledger_t** ledgers;

    dtunittest_suite_setup_f suite_setup;
    dtunittest_suite_teardown_f suite_teardown;
    dtunittest_test_setup_f test_setup;
    dtunittest_test_teardown_f test_teardown;

    void* callback_context;
} dtunittest_control_t;

//
//
// Typical implementation calls ::DTUNITTEST_RUN_TEST() repeatedly.
//
typedef void (*dtunittest_suitefunc_f)(dtunittest_control_t* unittest_control);

//
// Warning: On failure, return a live ::dterr_t*; the runner will print and dispose it.
//          On success, return `NULL`.
//
typedef dterr_t* (*dtunittest_testfunc_f)(void);

//
//
// Example:
// static void my_suite(DTUNITTEST_SUITE_ARGS) { ... }
//
#define DTUNITTEST_SUITE_ARGS dtunittest_control_t* unittest_control
#define DTUNITTEST_SUITE_PARAMS unittest_control

//
//
// summarize and print final line in test main
//
extern void
dtunittest_print_final(dtunittest_control_t* unittest_control);

//
//
// ::dtunittest_suite_teardown_f (if set). Per-suite pass/fail counts are
// reset before running.
//
extern void
dtunittest_run_suite(dtunittest_control_t* unittest_control, const char* test_name, dtunittest_suitefunc_f suitefunc);

//
//
// Honors ::dtunittest_control_t::pattern filtering and invokes per-test hooks.
// If ledgers are provided, the runner clears them before the test and audits
// them afterward to detect leaks.
//
extern void
dtunittest_run_test(dtunittest_control_t* unittest_control, const char* test_name, dtunittest_testfunc_f testfunc);

extern void
dtunittest_each_error_callback(dterr_t* dterr, void* context);

#define DTUNITTEST_RUN_SUITE(TESTNAME)                                                                                         \
    extern void TESTNAME(DTUNITTEST_SUITE_ARGS);                                                                               \
    dtunittest_run_suite(unittest_control, #TESTNAME, TESTNAME);
#define DTUNITTEST_RUN_TEST(TESTNAME) dtunittest_run_test(unittest_control, #TESTNAME, TESTNAME);

//
//  All macros assume the calling function defines `dterr` (of type ::dterr_t*) and a
//  `cleanup:` label. On failure they set `dterr = dterr_new(::DTERR_ASSERTION, ::DTERR_LOC, ...)`
//  and `goto cleanup;`. On success they do nothing.
//

#define DTUNITTEST_ASSERT_TRUE(ASSERTION)                                                                                      \
    if (!(ASSERTION))                                                                                                          \
    {                                                                                                                          \
        dterr = dterr_new(DTERR_ASSERTION, DTERR_LOC, NULL, "assertion failed: %s", #ASSERTION);                               \
        goto cleanup;                                                                                                          \
    }

//
//
// Disposes the incoming error on success and clears the variable to NULL.
// On mismatch it replaces `dterr` with a new assertion error describing the discrepancy.
//
#define DTUNITTEST_ASSERT_DTERR(DTERR, ERROR_CODE)                                                                             \
    if ((DTERR) == NULL)                                                                                                       \
    {                                                                                                                          \
        dterr = dterr_new(DTERR_ASSERTION,                                                                                     \
          DTERR_LOC,                                                                                                           \
          NULL,                                                                                                                \
          "assertion failed: dterr was NULL but expected %d (%s)",                                                             \
          (int)(ERROR_CODE),                                                                                                   \
          #ERROR_CODE);                                                                                                        \
        goto cleanup;                                                                                                          \
    }                                                                                                                          \
    else if ((DTERR)->error_code != (ERROR_CODE))                                                                              \
    {                                                                                                                          \
        int error_code_got = (DTERR)->error_code;                                                                              \
        dterr_dispose((DTERR));                                                                                                \
        dterr = dterr_new(DTERR_ASSERTION,                                                                                     \
          DTERR_LOC,                                                                                                           \
          NULL,                                                                                                                \
          "assertion failed: got dterr %d but expected %d (%s)",                                                               \
          error_code_got,                                                                                                      \
          (int)(ERROR_CODE),                                                                                                   \
          #ERROR_CODE);                                                                                                        \
        goto cleanup;                                                                                                          \
    }                                                                                                                          \
    else                                                                                                                       \
    {                                                                                                                          \
        dterr_dispose((DTERR));                                                                                                \
        DTERR = NULL;                                                                                                          \
    }

//
// Note: Formats values as signed 32-bit for readability.
//
#define DTUNITTEST_ASSERT_INT(GOT, OPERATOR, EXPECTED)                                                                         \
    if (!((GOT)OPERATOR(EXPECTED)))                                                                                            \
    {                                                                                                                          \
        dterr = dterr_new(DTERR_ASSERTION,                                                                                     \
          __LINE__,                                                                                                            \
          __FILE__,                                                                                                            \
          __func__,                                                                                                            \
          NULL,                                                                                                                \
          "assertion failed: got %s=%d but expected %s %d",                                                                    \
          #GOT,                                                                                                                \
          (int)(GOT),                                                                                                          \
          #OPERATOR,                                                                                                           \
          (int)(EXPECTED));                                                                                                    \
        goto cleanup;                                                                                                          \
    }

#define DTUNITTEST_ASSERT_UINT8(GOT, OPERATOR, EXPECTED)                                                                       \
    if (!((GOT)OPERATOR(EXPECTED)))                                                                                            \
    {                                                                                                                          \
        dterr = dterr_new(DTERR_ASSERTION,                                                                                     \
          __LINE__,                                                                                                            \
          __FILE__,                                                                                                            \
          __func__,                                                                                                            \
          NULL,                                                                                                                \
          "assertion failed: got %s=%" PRIu8 " but expected %s %" PRIu8,                                                       \
          #GOT,                                                                                                                \
          (uint8_t)(GOT),                                                                                                      \
          #OPERATOR,                                                                                                           \
          (uint8_t)(EXPECTED));                                                                                                \
        goto cleanup;                                                                                                          \
    }

#define DTUNITTEST_ASSERT_UINT32(GOT, OPERATOR, EXPECTED)                                                                      \
    if (!((GOT)OPERATOR(EXPECTED)))                                                                                            \
    {                                                                                                                          \
        dterr = dterr_new(DTERR_ASSERTION,                                                                                     \
          __LINE__,                                                                                                            \
          __FILE__,                                                                                                            \
          __func__,                                                                                                            \
          NULL,                                                                                                                \
          "assertion failed: got %s=%" PRIu32 " but expected %s %" PRIu32,                                                     \
          #GOT,                                                                                                                \
          (uint32_t)(GOT),                                                                                                     \
          #OPERATOR,                                                                                                           \
          (uint32_t)(EXPECTED));                                                                                               \
        goto cleanup;                                                                                                          \
    }

#define DTUNITTEST_ASSERT_UINT64(GOT, OPERATOR, EXPECTED)                                                                      \
    if (!((GOT)OPERATOR(EXPECTED)))                                                                                            \
    {                                                                                                                          \
        dterr = dterr_new(DTERR_ASSERTION,                                                                                     \
          __LINE__,                                                                                                            \
          __FILE__,                                                                                                            \
          __func__,                                                                                                            \
          NULL,                                                                                                                \
          "assertion failed: got %s=%" PRIu64 " but expected %s %" PRIu64,                                                     \
          #GOT,                                                                                                                \
          (uint64_t)(GOT),                                                                                                     \
          #OPERATOR,                                                                                                           \
          (uint64_t)(EXPECTED));                                                                                               \
        goto cleanup;                                                                                                          \
    }

//
// Note: Use when you want `<=` or `<` style checks around @p EXPECTED within @p TOLERANCE.
//
#define DTUNITTEST_ASSERT_DOUBLE(GOT, OPERATOR, TOLERANCE, EXPECTED)                                                           \
    if (!(fabs((GOT) - (EXPECTED)) OPERATOR(TOLERANCE)))                                                                       \
    {                                                                                                                          \
        dterr = dterr_new(DTERR_ASSERTION,                                                                                     \
          __LINE__,                                                                                                            \
          __FILE__,                                                                                                            \
          __func__,                                                                                                            \
          NULL,                                                                                                                \
          "assertion failed: got %s=%g is not %s %g within %g",                                                                \
          #GOT,                                                                                                                \
          (double)(GOT),                                                                                                       \
          #OPERATOR,                                                                                                           \
          (double)(EXPECTED),                                                                                                  \
          (double)(TOLERANCE));                                                                                                \
        goto cleanup;                                                                                                          \
    }

#define DTUNITTEST_ASSERT_PTR(GOT, OPERATOR, EXPECTED)                                                                         \
    if (!((GOT)OPERATOR(EXPECTED)))                                                                                            \
    {                                                                                                                          \
        dterr = dterr_new(DTERR_ASSERTION,                                                                                     \
          __LINE__,                                                                                                            \
          __FILE__,                                                                                                            \
          __func__,                                                                                                            \
          NULL,                                                                                                                \
          "assertion failed: got pointer %s=0x%p but expected %s 0x%p",                                                        \
          #GOT,                                                                                                                \
          (void*)(GOT),                                                                                                        \
          #OPERATOR,                                                                                                           \
          (void*)(EXPECTED));                                                                                                  \
        goto cleanup;                                                                                                          \
    }

#define DTUNITTEST_ASSERT_NULL(GOT)                                                                                            \
    if ((GOT) != NULL)                                                                                                         \
    {                                                                                                                          \
        dterr = dterr_new(DTERR_ASSERTION,                                                                                     \
          __LINE__,                                                                                                            \
          __FILE__,                                                                                                            \
          __func__,                                                                                                            \
          NULL,                                                                                                                \
          "assertion failed: %s was not NULL but expected otherwise",                                                          \
          #GOT);                                                                                                               \
        goto cleanup;                                                                                                          \
    }

#define DTUNITTEST_ASSERT_NOT_NULL(GOT)                                                                                        \
    if ((GOT) == NULL)                                                                                                         \
    {                                                                                                                          \
        dterr = dterr_new(DTERR_ASSERTION, DTERR_LOC, NULL, "assertion failed: %s was NULL but expected otherwise", #GOT);     \
        goto cleanup;                                                                                                          \
    }

//
// Warning: Both @p GOT and @p EXPECTED must be non-NULL NUL-terminated strings.
//
#define DTUNITTEST_ASSERT_EQUAL_STRING(GOT, EXPECTED)                                                                          \
    if (strcmp((GOT), (EXPECTED)) != 0)                                                                                        \
    {                                                                                                                          \
        dterr = dterr_new(DTERR_ASSERTION,                                                                                     \
          __LINE__,                                                                                                            \
          __FILE__,                                                                                                            \
          __func__,                                                                                                            \
          NULL,                                                                                                                \
          "assertion failed: got %s=\"%s\" but expected == \"%s\"",                                                            \
          #GOT,                                                                                                                \
          (GOT),                                                                                                               \
          (EXPECTED));                                                                                                         \
        goto cleanup;                                                                                                          \
    }

//
// Warning: Both @p GOT and @p EXPECTED must be non-NULL NUL-terminated strings.
//
#define DTUNITTEST_ASSERT_HAS_SUBSTRING(GOT, SUBSTRING)                                                                        \
    if (strstr((GOT), (SUBSTRING)) == NULL)                                                                                    \
    {                                                                                                                          \
        dterr = dterr_new(DTERR_ASSERTION,                                                                                     \
          __LINE__,                                                                                                            \
          __FILE__,                                                                                                            \
          __func__,                                                                                                            \
          NULL,                                                                                                                \
          "assertion failed: got %s=\"%s\" but expected to have substring \"%s\"",                                             \
          #GOT,                                                                                                                \
          (GOT),                                                                                                               \
          (SUBSTRING));                                                                                                        \
        goto cleanup;                                                                                                          \
    }

//
// Warning: Both buffers must be non-NULL and valid for @p LEN bytes.
//
#define DTUNITTEST_ASSERT_EQUAL_BYTES(GOT, EXPECTED, LEN)                                                                      \
    if (memcmp((GOT), (EXPECTED), (LEN)) != 0)                                                                                 \
    {                                                                                                                          \
        dterr = dterr_new(DTERR_ASSERTION,                                                                                     \
          __LINE__,                                                                                                            \
          __FILE__,                                                                                                            \
          __func__,                                                                                                            \
          NULL,                                                                                                                \
          "assertion failed: memory %s != %s for %zu bytes",                                                                   \
          #GOT,                                                                                                                \
          #EXPECTED,                                                                                                           \
          (size_t)(LEN));                                                                                                      \
        goto cleanup;                                                                                                          \
    }

#if MARKDOWN_DOCUMENTATION
// clang-format off
// --8<-- [start:markdown-documentation]
# dtunittest

Minimal C unit-test harness for suites, tests, assertions, and error/report plumbing. Provides a tiny, dependency-light runner used across the dtack portfolio.

## Mini-guide

- Use `dtunittest_run_suite()` to group related `dtunittest_run_test()` calls into one reportable suite.
- Tests return `NULL` on success or a `dterr_t*` on failure. The runner prints and disposes failures.
- Every test body must define a `dterr_t* dterr = NULL;` and a `cleanup:` label. The assertion macros jump there on failure.
- Optional hooks: set `suite_setup`, `suite_teardown`, `test_setup`, `test_teardown` in `dtunittest_control_t` for fixture lifecycle.
- Filtering: set `pattern` in `dtunittest_control_t` to run only tests whose names contain that substring.
- Resource auditing: if you provide a NULL-terminated array of `dtledger_t*` in `ledgers`, the runner clears and checks them around each test.

## Example

Typical flow: define tests that use the assert macros, then run them inside a suite.

```c
#include <dtcore/dtunittest.h>
#include <dtcore/dterr.h>

static dterr_t* T01_addition_holds(void)
{
    dterr_t* dterr = NULL;

    int got = 2 + 2;
    int expected = 4;

    DTUNITTEST_ASSERT_INT(got, ==, expected);

cleanup:
    return dterr; /* NULL means pass */
}

static void math_suite(dtunittest_control_t* unittest_control)
{
    DTUNITTEST_RUN_TEST(T01_addition_holds);
}

int main(void)
{
    dtunittest_control_t uc = {0};
    uc.should_print_tests = true;
    uc.should_print_suites = true;
    uc.should_print_errors = true;

    dtunittest_run_suite(&uc, "math_suite", math_suite);
    return (uc.total_fail_count == 0) ? 0 : 1;
}
```

Notes:
- `DTUNITTEST_RUN_TEST(X)` expands to `dtunittest_run_test(unittest_control, "X", X);`.
- If you prefer not to use the macro, call `dtunittest_run_test` directly.
- For suite functions, `DTUNITTEST_SUITE_ARGS` expands to `dtunittest_control_t* unittest_control`.

## Functions

### dtunittest_run_suite

```c
void dtunittest_run_suite(dtunittest_control_t* unittest_control,
                          const char* test_name,
                          dtunittest_suitefunc_f suitefunc);
```
Runs a suite function and reports results. Resets per-suite counters, invokes optional `suite_setup` and `suite_teardown`, and accumulates totals. `unittest_control` must be non-NULL.

### dtunittest_run_test

```c
void dtunittest_run_test(dtunittest_control_t* unittest_control,
                         const char* test_name,
                         dtunittest_testfunc_f testfunc);
```
Runs a single test and updates pass/fail counts. Applies name filtering via `unittest_control->pattern`, invokes `test_setup`/`test_teardown`, audits `ledgers` if provided, prints errors if enabled, and disposes any returned `dterr_t*`.

### dtunittest_each_error_callback

```c
void dtunittest_each_error_callback(dterr_t* dterr, void* context);
```
Callback suitable for `dterr_each()` to print one node in an error chain. The default impl ignores `context`.

## Contracts and invariants

- Test contract: each test must define `dterr_t* dterr` and a `cleanup:` label; return `NULL` on success or a live `dterr_t*` on failure. The runner owns disposal of the returned error.
- Filtering: when `pattern` is non-NULL, only tests whose names contain the substring run.
- Ledgers: `ledgers` is a NULL-terminated array of `dtledger_t*`. When set, the runner resets before a test and verifies after, treating leaks or negative low-water marks as failures.
- Printing flags: `should_print_tests`, `should_print_suites`, and `should_print_errors` gate console output but do not affect pass/fail semantics.

## Assertion macros (overview)

Use the helpers to create readable failures and jump to `cleanup`:
- `DTUNITTEST_ASSERT_TRUE(expr)`
- `DTUNITTEST_ASSERT_DTERR(err, code)` (disposes on success)
- `DTUNITTEST_ASSERT_INT(got, OP, expected)` (signed int)
- `DTUNITTEST_ASSERT_UINT8/UINT32/UINT64(got, OP, expected)`
- `DTUNITTEST_ASSERT_DOUBLE(got, OP, tol, expected)`
- `DTUNITTEST_ASSERT_PTR(got, OP, expected)`
- `DTUNITTEST_ASSERT_NULL(ptr)` / `DTUNITTEST_ASSERT_NOT_NULL(ptr)`
- `DTUNITTEST_ASSERT_EQUAL_STRING(got, expected)`
- `DTUNITTEST_ASSERT_EQUAL_BYTES(got, expected, len)`

All macros set `dterr = dterr_new(DTERR_ASSERTION, DTERR_LOC, ...)` and `goto cleanup;` when the check fails.

// --8<-- [end:markdown-documentation]
// clang-format on
#endif