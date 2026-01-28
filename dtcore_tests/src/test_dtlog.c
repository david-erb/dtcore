#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <dtcore/dterr.h>
#include <dtcore/dtlog.h>
#include <dtcore/dtunittest.h>
#include <dtcore_tests.h>

/* Simple capture sink used by several tests. */
static char g_emitted[512];

static void
emit_to_buffer(void* object, const char* message)
{
    (void)object;
    if (!message)
        return;
    strncpy(g_emitted, message, sizeof(g_emitted));
    g_emitted[sizeof(g_emitted) - 1] = '\0';
}

/* Hook that can suppress emission (returns false). */
static bool
hook_drop(void* ctx, dtlog_level_t level, const char* tag, const char* message)
{
    (void)ctx;
    (void)level;
    (void)tag;
    (void)message;
    return false;
}

/* Helper to format via va_list path explicitly. */
static void
log_info_via_va(const char* tag, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    dtlog_va(DTLOG_INFO, tag, fmt, args);
    va_end(args);
}

// ------------------------------------------------------------------------
// Example: minimal usage with default logger and custom sink
static dterr_t*
test_dtcore_dtlog_example_basic()
{
    dterr_t* dterr = NULL;

    g_emitted[0] = '\0';
    dtlogger_reset_default();
    DTUNITTEST_ASSERT_TRUE(dtlogger_init(dtlogger_default) == NULL);

    dtlogger_default->emit = emit_to_buffer;

    dtlog_info("EX", "Hello %s", "World");

    DTUNITTEST_ASSERT_EQUAL_STRING(g_emitted, "[INFO ] EX: Hello World");

cleanup:
    return dterr;
}

// ------------------------------------------------------------------------
// Example: NULL tag is allowed and prints as an empty classifier
static dterr_t*
test_dtcore_dtlog_example_null_tag()
{
    dterr_t* dterr = NULL;

    g_emitted[0] = '\0';
    dtlogger_reset_default();
    DTUNITTEST_ASSERT_TRUE(dtlogger_init(dtlogger_default) == NULL);

    dtlogger_default->emit = emit_to_buffer;

    dtlog_info(NULL, "value=%d", 42);

    /* Prefix format is "[%-5s] %s: "; empty tag yields a single space then ":" */
    DTUNITTEST_ASSERT_EQUAL_STRING(g_emitted, "[INFO ] : value=42");

cleanup:
    return dterr;
}

// ------------------------------------------------------------------------
// Example: hook can drop messages before they reach emit()
static dterr_t*
test_dtcore_dtlog_example_hook_suppresses()
{
    dterr_t* dterr = NULL;

    strcpy(g_emitted, "UNCHANGED");
    dtlogger_reset_default();
    DTUNITTEST_ASSERT_TRUE(dtlogger_init(dtlogger_default) == NULL);

    dtlogger_default->emit = emit_to_buffer;
    dtlogger_default->hookfunc = hook_drop;
    dtlogger_default->hookfunc_context = NULL;

    dtlog_error("TAG", "should not appear");

    DTUNITTEST_ASSERT_EQUAL_STRING(g_emitted, "UNCHANGED");

cleanup:
    return dterr;
}

// ------------------------------------------------------------------------
// Example: using the va_list path directly via ::dtlog_va
static dterr_t*
test_dtcore_dtlog_example_va_forwarding()
{
    dterr_t* dterr = NULL;

    g_emitted[0] = '\0';
    dtlogger_reset_default();
    DTUNITTEST_ASSERT_TRUE(dtlogger_init(dtlogger_default) == NULL);

    dtlogger_default->emit = emit_to_buffer;

    log_info_via_va("VATEST", "X=%d Y=%s", 7, "ok");

    DTUNITTEST_ASSERT_EQUAL_STRING(g_emitted, "[INFO ] VATEST: X=7 Y=ok");

cleanup:
    return dterr;
}

// ------------------------------------------------------------------------
// 01: level_to_string() mapping is stable
static dterr_t*
test_dtcore_dtlog_01_level_to_string()
{
    dterr_t* dterr = NULL;
    dtlogger_t logger;

    dterr_t* err = dtlogger_init(&logger);
    DTUNITTEST_ASSERT_TRUE(err == NULL);

    DTUNITTEST_ASSERT_EQUAL_STRING(logger.level_to_string(&logger, DTLOG_TRACE), "TRACE");
    DTUNITTEST_ASSERT_EQUAL_STRING(logger.level_to_string(&logger, DTLOG_DEBUG), "DEBUG");
    DTUNITTEST_ASSERT_EQUAL_STRING(logger.level_to_string(&logger, DTLOG_INFO), "INFO");
    DTUNITTEST_ASSERT_EQUAL_STRING(logger.level_to_string(&logger, DTLOG_WARN), "WARN");
    DTUNITTEST_ASSERT_EQUAL_STRING(logger.level_to_string(&logger, DTLOG_ERROR), "ERROR");
    DTUNITTEST_ASSERT_EQUAL_STRING(logger.level_to_string(&logger, DTLOG_FATAL), "FATAL");
    DTUNITTEST_ASSERT_EQUAL_STRING(logger.level_to_string(&logger, DTLOG_OFF), "OFF");
    DTUNITTEST_ASSERT_EQUAL_STRING(logger.level_to_string(&logger, (dtlog_level_t)231), "UNKNOWN");

cleanup:
    return dterr;
}

// ------------------------------------------------------------------------
// 02: dtlogger_init() establishes default callbacks and stdout
static dterr_t*
test_dtcore_dtlog_02_init_defaults()
{
    dterr_t* dterr = NULL;
    dtlogger_t logger;

    dterr_t* err = dtlogger_init(&logger);
    DTUNITTEST_ASSERT_TRUE(err == NULL);
    DTUNITTEST_ASSERT_TRUE(logger.file == stdout);
    DTUNITTEST_ASSERT_TRUE(logger.emit != NULL);
    DTUNITTEST_ASSERT_TRUE(logger.format_variadic != NULL);
    DTUNITTEST_ASSERT_TRUE(logger.format_va_args != NULL);
    DTUNITTEST_ASSERT_TRUE(logger.dispose != NULL);

cleanup:
    return dterr;
}

// ------------------------------------------------------------------------
// 03: first use lazily initializes default logger
static dterr_t*
test_dtcore_dtlog_03_lazy_init_default()
{
    dterr_t* dterr = NULL;

    dtlogger_reset_default();
    dtlog(DTLOG_INFO, "TEST", "Lazy init check");
    DTUNITTEST_ASSERT_TRUE(dtlogger_default != NULL);
    DTUNITTEST_ASSERT_TRUE(dtlogger_default->format_va_args != NULL);

cleanup:
    return dterr;
}

// ------------------------------------------------------------------------
// 04: custom sink path formats exactly as expected
static dterr_t*
test_dtcore_dtlog_04_custom_emit_buffer()
{
    dterr_t* dterr = NULL;

    g_emitted[0] = '\0';
    dtlogger_reset_default();
    DTUNITTEST_ASSERT_TRUE(dtlogger_init(dtlogger_default) == NULL);

    dtlogger_default->emit = emit_to_buffer;

    dtlog_info("TEST_TAG", "Hello %s", "World");

    DTUNITTEST_ASSERT_EQUAL_STRING(g_emitted, "[INFO ] TEST_TAG: Hello World");

cleanup:
    dtlogger_reset_default();
    return dterr;
}

// ------------------------------------------------------------------------
void
test_dtcore_dtlog(DTUNITTEST_SUITE_ARGS)
{
    /* Examples first (short, readable patterns) */
    DTUNITTEST_RUN_TEST(test_dtcore_dtlog_example_basic);
    DTUNITTEST_RUN_TEST(test_dtcore_dtlog_example_null_tag);
    DTUNITTEST_RUN_TEST(test_dtcore_dtlog_example_hook_suppresses);
    DTUNITTEST_RUN_TEST(test_dtcore_dtlog_example_va_forwarding);

    /* Coverage-preserving originals (renamed & ordered) */
    DTUNITTEST_RUN_TEST(test_dtcore_dtlog_01_level_to_string);
    DTUNITTEST_RUN_TEST(test_dtcore_dtlog_02_init_defaults);
    DTUNITTEST_RUN_TEST(test_dtcore_dtlog_03_lazy_init_default);
    DTUNITTEST_RUN_TEST(test_dtcore_dtlog_04_custom_emit_buffer);
}
