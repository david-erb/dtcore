#include <stdio.h>
#include <string.h>

#include <dtcore/dtbuffer.h>
#include <dtcore/dterr.h>
#include <dtcore/dtheaper.h>
#include <dtcore/dtledger.h>
#include <dtcore/dtlog.h>
#include <dtcore/dtstr.h>
#include <dtcore/dtunittest.h>

#include <dtcore_tests.h>

#define TAG "test_dtcore_matching"

// -------------------------------------------------------------------------------
void
test_dtcore_matching(DTUNITTEST_SUITE_ARGS)
{
    // ledgers we will check at end of each test
    dtledger_t* ledgers[10] = { 0 };
    {
        int i = 0;
        ledgers[i++] = dtstr_ledger;
        ledgers[i++] = dterr_ledger;
        ledgers[i++] = dtbuffer_ledger;
        ledgers[i++] = dtheaper_ledger;
    }

    unittest_control->ledgers = ledgers;

    DTUNITTEST_RUN_SUITE(test_dtcore_dtpackx);

    DTUNITTEST_RUN_SUITE(test_dtcore_dtstr);
    DTUNITTEST_RUN_SUITE(test_dtcore_dterr);
    DTUNITTEST_RUN_SUITE(test_dtcore_dtlog);
    DTUNITTEST_RUN_SUITE(test_dtcore_dtheaper);
    DTUNITTEST_RUN_SUITE(test_dtcore_dtunittest);
    DTUNITTEST_RUN_SUITE(test_dtcore_dtbuffer);
    DTUNITTEST_RUN_SUITE(test_dtcore_dtvtable);
    DTUNITTEST_RUN_SUITE(test_dtcore_dtkvp);

    DTUNITTEST_RUN_SUITE(test_dtguid);
    DTUNITTEST_RUN_SUITE(test_dtcore_dtrpc);
    DTUNITTEST_RUN_SUITE(test_dtcore_dtrpc_registry);

    DTUNITTEST_RUN_SUITE(test_dtcore_dtarray_int32);
    DTUNITTEST_RUN_SUITE(test_dtcore_dtarray_float);

    DTUNITTEST_RUN_SUITE(test_dtrandomizer_uniform);
    DTUNITTEST_RUN_SUITE(test_dtrandomizer_browngrav);
    DTUNITTEST_RUN_SUITE(test_dteventlogger);
    DTUNITTEST_RUN_SUITE(test_dtguidable_pool);
    DTUNITTEST_RUN_SUITE(test_dtflipper);

    DTUNITTEST_RUN_SUITE(test_dtcore_dtchunker);

    DTUNITTEST_RUN_SUITE(test_dtcore_dttrap);

    DTUNITTEST_RUN_SUITE(test_dtcore_dtringfifo);

    DTUNITTEST_RUN_SUITE(test_dtrgb565);
    DTUNITTEST_RUN_SUITE(test_dtraster_rgba8888);
    DTUNITTEST_RUN_SUITE(test_dtraster_rgb565);
    DTUNITTEST_RUN_SUITE(test_dtglyph_dos);

    unittest_control->test_setup = NULL;
    unittest_control->test_teardown = NULL;
}
