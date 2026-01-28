#pragma once

// See markdown documentation at the end of this file.
// Lightweight logging with pluggable sink and simple lazy init.

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#include <dtcore/dterr.h>

//
// Expands to: `"[file] function():line"`. Requires a `TAG` identifier in scope.
// Note: Handy in unit tests and ad-hoc debugging to trace control flow.
#define DTLOG_HERE()                                                           \
  dtlog_debug(                                                                 \
    TAG, "here in file %s at function %s():%d", __FILE__, __func__, __LINE__)

// Ignores the ::dtlog_printf_enabled flag. Useful when the logging
// infrastructure is not yet initialized.
#define DTLOG_PRINTF_HERE()                                                    \
  printf("*here in file %s at function %s():%d\n", __FILE__, __func__, __LINE__)

// When `true`, ::DTLOG_PRINTF forwards to `printf`; when `false`, it does
// nothing. Warning: This flag does not control the main logger output.
extern bool dtlog_printf_enabled;

// Note: This macro bypasses the ::dtlogger_t path entirely.
#define DTLOG_PRINTF(...)                                                      \
  do {                                                                         \
    if (dtlog_printf_enabled)                                                  \
      printf(__VA_ARGS__);                                                     \
  } while (0)

typedef enum
{
  DTLOG_OFF,   ///< Logging disabled.
  DTLOG_TRACE, ///< Very detailed diagnostics (high volume).
  DTLOG_DEBUG, ///< Developer-focused information.
  DTLOG_INFO,  ///< Routine informational messages.
  DTLOG_WARN,  ///< Potential issues; program continues.
  DTLOG_ERROR, ///< Errors that allow the program to continue.
  DTLOG_FATAL, ///< Critical failures; program may terminate.
} dtlog_level_t;

//
typedef void (*dtlog_emitfunc_t)(void* context, const char* message);

//
// Returning `true` allows emission to proceed; `false` suppresses the message.
typedef bool (*dtlog_hookfunc_t)(void* object,
                                 dtlog_level_t level,
                                 const char* tag,
                                 const char* message);

//
// A ::dtlogger_t formats messages and writes them via ::dtlog_emitfunc_t.
// The default global instance (see ::dtlogger_default) writes to `stdout`.
//
// Warning: ::dispose does not close file; the caller owns the `FILE*`.
typedef struct dtlogger_t
{
  void (*emit)(void*, const char*); ///< Sink for fully formatted lines.
  void (*format_variadic)(void*,
                          dtlog_level_t,
                          const char*,
                          const char*,
                          ...); ///< Variadic front-end.
  void (*format_va_args)(void*,
                         dtlog_level_t,
                         const char*,
                         const char*,
                         va_list); ///< `va_list` variant.
  const char* (*level_to_string)(
    struct dtlogger_t*,
    dtlog_level_t level); ///< Convert severity to uppercase text.
  void (*dispose)(
    struct dtlogger_t*); ///< Reset this logger (does not close file).

  dtlog_hookfunc_t hookfunc; ///< Optional pre-emit hook (may be `NULL`).
  void* hookfunc_context;    ///< Opaque context passed to `hookfunc`.
  FILE* file; ///< Output stream used by ::emit (ownership stays with caller).
} dtlogger_t;

//
// Warning: @p this must outlive any subsequent logging calls that use it.
dterr_t*
dtlogger_init(dtlogger_t* this);

//
// After reset, the next use of ::dtlog() (or friends) lazily re-initializes it.
extern void
dtlogger_reset_default(void);

//
// Lazily initialized on first use. You may tweak fields (e.g.,
// ::dtlogger_t::file) after initialization to redirect output.
extern dtlogger_t* dtlogger_default;

// Note: Lazily initializes ::dtlogger_default on first call.
extern void
dtlog(dtlog_level_t, const char* tag, const char* format, ...);

extern void
dtlog_debug(const char* tag, const char* format, ...);

extern void
dtlog_info(const char* tag, const char* format, ...);

extern void
dtlog_warn(const char* tag, const char* format, ...);

extern void
dtlog_error(const char* tag, const char* format, ...);

extern void
dtlog_dterr(const char* tag, dterr_t* dterr);

// Note: If you need @p args afterwards, pass a copy made with `va_copy`.
extern void
dtlog_va(dtlog_level_t, const char* tag, const char* format, va_list args);

extern bool
dtlog_message_is_debug(const char* message);

extern bool
dtlog_message_is_info(const char* message);

extern bool
dtlog_message_is_error(const char* message);

#if MARKDOWN_DOCUMENTATION
// clang-format off
// --8<-- [start:markdown-documentation]

# dtlog

Lightweight logging with pluggable sink and lazy setup

This group of functions provides logging and message formatting for C modules.
It is intended for small to medium codebases that need tagged, leveled output with minimal setup.


## Mini-guide

- Initialize a logger explicitly with `dtlogger_init` when you need control over the output sink.
- Use `dtlog` or the level-specific helpers to emit messages by passing a severity and tag.
- Rely on lazy initialization of the default logger when no explicit setup is required.
- Attach a hook function to filter or suppress messages by returning false.
- Reset the default logger with `dtlogger_reset_default` to force reinitialization on next use.

## Example

```c
#define TAG "example"

dtlog_info(TAG, "starting operation");

dtlog_debug(TAG, "value=%d", value);

if (error != NULL)
{
    dtlog_dterr(TAG, error);
}
```

## Data structures

### dtlog_level_t

Defines the severity level associated with a log message.

### dtlog_emitfunc_t

Defines the function signature used to emit a formatted log line.

### dtlog_hookfunc_t

Defines the function signature used to filter log messages before emission.

### dtlogger_t

Holds function pointers and state for formatting and emitting log messages.

Members:

> `void (*emit)(void*, const char*)` Sink for fully formatted lines.  
> `void (*format_variadic)(void*, dtlog_level_t, const char*, const char*, ...)` Variadic formatting entry point.  
> `void (*format_va_args)(void*, dtlog_level_t, const char*, const char*, va_list)` Formatting entry point using a `va_list`.  
> `const char* (*level_to_string)(struct dtlogger_t*, dtlog_level_t level)` Converts a severity level to text.  
> `void (*dispose)(struct dtlogger_t*)` Resets the logger state without closing the file.  
> `dtlog_hookfunc_t hookfunc` Optional pre-emit hook.  
> `void* hookfunc_context` Context pointer passed to the hook function.  
> `FILE* file` Output stream used by the emitter.

## Macros

### DTLOG_HERE

Emits a debug message that identifies the current source location.

## Functions

### dtlog

Emits a formatted log message at the specified severity level.

Params:

> `dtlog_level_t` Severity level to associate with the message.  
> `const char* tag` Short tag identifying the message source.  
> `const char* format` `printf`-style format string.  
> `...` Format arguments.

Return: `void`  No return value.

### dtlog_debug

Emits a formatted log message at debug level.

Params:

> `const char* tag` Short tag identifying the message source.  
> `const char* format` `printf`-style format string.  
> `...` Format arguments.

Return: `void`  No return value.

### dtlog_dterr

Emits each entry of an error chain as an error-level message.

Params:

> `const char* tag` Short tag identifying the message source.  
> `dterr_t* dterr` Error chain to emit.

Return: `void`  No return value.

### dtlog_error

Emits a formatted log message at error level.

Params:

> `const char* tag` Short tag identifying the message source.  
> `const char* format` `printf`-style format string.  
> `...` Format arguments.

Return: `void`  No return value.

### dtlog_info

Emits a formatted log message at informational level.

Params:

> `const char* tag` Short tag identifying the message source.  
> `const char* format` `printf`-style format string.  
> `...` Format arguments.

Return: `void`  No return value.


### dtlog_va

Emits a formatted log message using an existing `va_list`.

Params:

> `dtlog_level_t` Severity level to associate with the message.  
> `const char* tag` Short tag identifying the message source.  
> `const char* format` `printf`-style format string.  
> `va_list args` Argument list to consume.

Return: `void`  No return value.

### dtlogger_init

Initializes a logger instance for subsequent use.

Params:

> `dtlogger_t* this` Logger instance to initialize.

Return: `dterr_t*` Error object on failure or `NULL` on success.

### dtlogger_reset_default

Resets the default logger so it will be reinitialized on next use.

Param: `void`  No parameters.

Return: `void`  No return value.

<!-- FG_IDC: 724c5211-1e5c-4ac9-a05c-b6e28cba4cae | FG_UTC: 2026-01-17T14:41:50Z | FG_MAN=yes -->

// --8<-- [end:markdown-documentation]
// clang-format on
#endif
