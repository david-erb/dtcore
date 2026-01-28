#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dterr.h>
#include <dtcore/dtledger.h>
#include <dtcore/dtlog.h>
#include <dtcore/dtstr.h>
#include <dtcore/dtunittest.h>

#define TAG "dtunittest"

/* --------------------------------------------------------------------- */
/* Defensive helpers for printing potentially-null fields */
static const char*
_nz(const char* s)
{
    return s ? s : "(null)";
}

/* --------------------------------------------------------------------- */
void
dtunittest_each_error_callback(dterr_t* dterr, void* context)
{
    (void)context;
    /* Be defensive: any of these could be NULL depending on producer */
    const char* file = dterr ? _nz(dterr->source_file) : "(null dterr)";
    const char* func = dterr ? _nz(dterr->source_function) : "(unknown)";
    const char* msg = dterr ? _nz(dterr->message) : "(no message)";
    long line = dterr ? (long)dterr->line_number : -1L;

    printf("%s@%ld in %s: %s\n", file, line, func, msg);
}

/* --------------------------------------------------------------------- */
void
dtunittest_run_suite(dtunittest_control_t* unittest_control, const char* suite_name, dtunittest_suitefunc_f suitefunc)
{
    /* clear per-suite counters */
    unittest_control->suite_fail_count = 0;
    unittest_control->suite_pass_count = 0;

    /* honor optional suite setup */
    if (unittest_control->suite_setup)
    {
        unittest_control->suite_setup(unittest_control);
    }

    suitefunc(unittest_control);

    /* honor optional suite teardown */
    if (unittest_control->suite_teardown)
    {
        unittest_control->suite_teardown(unittest_control);
    }

    if (unittest_control->should_print_suites)
    {
        if (unittest_control->suite_fail_count != 0)
        {
            printf("!!!! suite \"%s\" FAIL (%d/%d)\n",
              suite_name,
              unittest_control->suite_fail_count,
              unittest_control->suite_fail_count + unittest_control->suite_pass_count);
        }
        else if (unittest_control->suite_pass_count != 0)
        {
            printf("---- suite \"%s\" PASS (%d)\n", suite_name, unittest_control->suite_pass_count);
        }
    }
}

/* --------------------------------------------------------------------- */
static char*
dtunittest_check_ledger(dtledger_t* ledger)
{
    char* message = NULL;
    if (ledger->count_balance != 0)
    {
        message = dtstr_format("%s ledger count balance == %d but should be 0", ledger->class_name, ledger->count_balance);
    }
    else if (ledger->bytes_balance != 0)
    {
        message = dtstr_format("%s ledger bytes balance == %d but should be 0", ledger->class_name, ledger->bytes_balance);
    }
    else if (ledger->count_low_water_mark < 0)
    {
        message = dtstr_format("%s ledger low water mark == %d", ledger->class_name, ledger->count_low_water_mark);
    }

    return message;
}

/* --------------------------------------------------------------------- */
/* Post-test ledger audit:
 *  - On failure: record a test failure.
 *  - On success: do NOT increment pass counts (avoid double-counting).
 */
static void
dtunittest_check_ledgers(dtunittest_control_t* unittest_control, const char* test_name)
{
    if (unittest_control->ledgers == NULL)
        return;

    dtledger_t** ledgers = unittest_control->ledgers;
    char* message = NULL;

    for (int i = 0; ledgers[i] != NULL; i++)
    {
        message = dtunittest_check_ledger(ledgers[i]);
        if (message != NULL)
            break;
    }

    if (message != NULL)
    {
        if (unittest_control->should_print_errors)
        {
            dtlog_error(TAG, "after %s, %s", test_name, message);
        }
        dtstr_dispose(message);
        /* count as a failure for this test run */
        unittest_control->suite_fail_count++;
        unittest_control->total_fail_count++;
    }
    /* else: no-op on success to avoid inflating pass totals */
}

/* --------------------------------------------------------------------- */
static void
dtunittest_clear_ledgers(dtunittest_control_t* unittest_control)
{
    if (unittest_control->ledgers != NULL)
    {
        dtledger_t** ledgers = unittest_control->ledgers;
        for (int i = 0; ledgers[i] != NULL; i++)
        {
            const char* class_name = ledgers[i]->class_name;
            memset(ledgers[i], 0, sizeof(dtledger_t));
            ledgers[i]->class_name = class_name;
        }
    }
}

/* --------------------------------------------------------------------- */
void
dtunittest_run_test(dtunittest_control_t* unittest_control, const char* test_name, dtunittest_testfunc_f testfunc)
{
    if (unittest_control->pattern != NULL && strstr(test_name, unittest_control->pattern) == NULL)
    {
        return;
    }

    if (unittest_control->test_setup != NULL)
    {
        unittest_control->test_setup(unittest_control);
    }

    if (unittest_control->should_print_tests)
    {
        printf("--------------------------------\n");
        printf("%s test starting\n", test_name);
    }

    /* clear ledgers before running test */
    dtunittest_clear_ledgers(unittest_control);

    dterr_t* dterr = testfunc();
    unittest_control->test_passed = (dterr == NULL);

    if (dterr != NULL)
    {
        if (unittest_control->should_print_tests)
        {
            printf("%s FAIL\n", test_name);
        }

        if (unittest_control->should_print_errors)
        {
            dterr_each(dterr, dtunittest_each_error_callback, NULL);
        }
        dterr_dispose(dterr);
        unittest_control->suite_fail_count++;
        unittest_control->total_fail_count++;
    }
    else
    {
        if (unittest_control->should_print_tests)
        {
            printf("%s PASS\n", test_name);
        }
        unittest_control->suite_pass_count++;
        unittest_control->total_pass_count++;

        /* Post-pass ledger audit: only records failures. */
        dtunittest_check_ledgers(unittest_control, test_name);
    }

    if (unittest_control->test_teardown != NULL)
    {
        unittest_control->test_teardown(unittest_control);
    }
}

/* --------------------------------------------------------------------- */
// summarize and print final line in test main
void
dtunittest_print_final(dtunittest_control_t* unittest_control)
{
    if (unittest_control == NULL)
    {
        printf("unavailable to print final: unittest_control is NULL\n");
    }
    else
    {

        printf("---- total: %d passed, %d failed\n", unittest_control->total_pass_count, unittest_control->total_fail_count);
    }

    fflush(stdout);
}
