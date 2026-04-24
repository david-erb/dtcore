#include <dtcore/dtlog.h>

#include <dtcore_tests.h>

#define TAG "test_all"

int
main()
{
    dtunittest_control_t unittest_control = { 0 };
    unittest_control.should_print_suites = true;
    unittest_control.should_print_tests = false;
    unittest_control.should_print_errors = true;

    test_dtcore_matching(&unittest_control);

    // print summary as final line of test output
    dtunittest_print_final(&unittest_control);

    exit(unittest_control.total_fail_count == 0 ? 0 : 1);
}
