#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dterr.h>
#include <dtcore/dtlog.h>
#include <dtcore/dtstr.h>

bool dtlog_printf_enabled = false; // currently unused; retained to avoid ABI/symbol surprises

// ----------------------------------------------------------------
// emit a fully-formatted log line; add newline and flush.
// Safe even if file is NULL: falls back to stdout.
static void
emit(void* object, const char* message)
{
    dtlogger_t* this = (dtlogger_t*)object;
    if (this == NULL || message == NULL)
        return;

    FILE* out = this->file ? this->file : stdout;
    (void)fprintf(out, "%s\n", message);
    (void)fflush(out);
}

// ----------------------------------------------------------------
// stringify level
static const char*
level_to_string(dtlogger_t* this, dtlog_level_t level)
{
    (void)this;
    switch (level)
    {
        case DTLOG_TRACE:
            return "TRACE";
        case DTLOG_DEBUG:
            return "DEBUG";
        case DTLOG_INFO:
            return "INFO";
        case DTLOG_WARN:
            return "WARN";
        case DTLOG_ERROR:
            return "ERROR";
        case DTLOG_FATAL:
            return "FATAL";
        case DTLOG_OFF:
            return "OFF";
        default:
            return "UNKNOWN";
    }
}

// ----------------------------------------------------------------
// format method to emit log messages from a va_list.
// Builds the decorated line using dtstr_format_va + dtstr_concat.
// - Formats the message body using a private va_copy.
// - Prefix is "[%-5s] tag: " (tag may be NULL -> treated as "").
static void
format_va_args(void* object, dtlog_level_t level, const char* tag, const char* format, va_list args)
{
    dtlogger_t* this = (dtlogger_t*)object;
    if (this == NULL || format == NULL)
        return;

    char* s = NULL;
    char* t = NULL;

    const char* safe_tag = (tag != NULL) ? tag : "";
    s = dtstr_format("[%-5s] %s: ", this->level_to_string(this, level), safe_tag);
    if (s == NULL)
        return;

    // format the body using a private copy of args
    va_list body_args;
    va_copy(body_args, args);
    s = dtstr_concat_format_va(s, t, format, body_args);
    va_end(body_args);

    if (s == NULL)
        return;

    // 3) Hook gate
    bool hook_result = true;
    if (this->hookfunc != NULL)
        hook_result = this->hookfunc(this->hookfunc_context, level, safe_tag, s);

    if (hook_result)
        this->emit(object, s);

    dtstr_dispose(s);
}

// ----------------------------------------------------------------
// variadic front-end that forwards to format_va_args
static void
format_variadic(void* object, dtlog_level_t level, const char* tag, const char* format, ...)
{
    if (object == NULL || format == NULL)
        return;

    va_list args;
    va_start(args, format);
    format_va_args(object, level, tag, format, args);
    va_end(args);
}

// ----------------------------------------------------------------
static void
dispose(dtlogger_t* this)
{
    if (this == NULL)
        return;

    // NOTE: we don't close FILE*; ownership is external unless your API says otherwise
    memset(this, 0, sizeof(*this));
}

// ----------------------------------------------------------------
dterr_t*
dtlogger_init(dtlogger_t* this)
{
    if (this == NULL)
        return dterr_new(DTERR_FAIL, DTERR_LOC, NULL, "this is NULL");

    memset(this, 0, sizeof(*this));

    this->file = stdout;
    this->emit = emit;
    this->format_variadic = format_variadic;
    this->format_va_args = format_va_args;
    this->level_to_string = level_to_string;
    this->dispose = dispose;

    return NULL;
}

// ----------------------------------------------------------------
// a global object for convenience as a default logger
static dtlogger_t _dtlogger_default = { 0 };
dtlogger_t* dtlogger_default = &_dtlogger_default;

void
dtlogger_reset_default(void)
{
    dtlogger_default = &_dtlogger_default;
    memset(&_dtlogger_default, 0, sizeof(_dtlogger_default));
}

// ----------------------------------------------------------------
// default-logger helpers (lazy init, best-effort; no threads)
void
dtlog_va(dtlog_level_t level, const char* tag, const char* format, va_list args)
{
    if (dtlogger_default->dispose == NULL)
    {
        // Best-effort lazy init; if it fails, we still have safe emit fallback
        (void)dtlogger_init(dtlogger_default);
    }
    dtlogger_default->format_va_args(dtlogger_default, level, tag, format, args);
}

// ----------------------------------------------------------------
void
dtlog(dtlog_level_t level, const char* tag, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    dtlog_va(level, tag, format, args);
    va_end(args);
}

// ----------------------------------------------------------------
void
dtlog_debug(const char* tag, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    dtlog_va(DTLOG_DEBUG, tag, format, args);
    va_end(args);
}

// ----------------------------------------------------------------
void
dtlog_info(const char* tag, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    dtlog_va(DTLOG_INFO, tag, format, args);
    va_end(args);
}

// ----------------------------------------------------------------
void
dtlog_warn(const char* tag, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    dtlog_va(DTLOG_WARN, tag, format, args);
    va_end(args);
}

// ----------------------------------------------------------------
void
dtlog_error(const char* tag, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    dtlog_va(DTLOG_ERROR, tag, format, args);
    va_end(args);
}

// --------------------------------------------------------------------------------------
static void
_each_error_log(dterr_t* dterr, void* context)
{
    const char* tag = (const char*)context;
    dtlog_error(tag, "%s@%ld in %s: %s", dterr->source_file, (long)dterr->line_number, dterr->source_function, dterr->message);
}

// ----------------------------------------------------------------
// log dterr chain
void
dtlog_dterr(const char* tag, dterr_t* dterr)
{
    if (dterr != NULL)
    {
        dterr_each(dterr, _each_error_log, (void*)tag);
    }
}

// ----------------------------------------------------------------
// simple classifiers used by tests (format must remain stable)
bool
dtlog_message_is_debug(const char* message)
{
    const char* p = strstr(message, "[DEBUG]");
    return p != NULL;
}

bool
dtlog_message_is_info(const char* message)
{
    const char* p = strstr(message, "[INFO ]"); // padded to width 5
    return p != NULL;
}

bool
dtlog_message_is_error(const char* message)
{
    const char* p = strstr(message, "[ERROR]");
    return p != NULL;
}
