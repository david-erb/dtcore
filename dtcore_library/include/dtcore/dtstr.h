#pragma once
// See markdown documentation at the end of this file.

// String formatting and concatenation helpers with heap allocation.

#include <stdarg.h>

#include <dtcore/dtledger.h>

// Declare a ledger channel named "dtstr" for allocation accounting.
DTLEDGER_DECLARE(dtstr);

// Note: Free with ::dtstr_dispose() when no longer needed.
extern char*
dtstr_format(const char* format, ...);

// Note: This function copies @p args internally; the caller must still pair
// their own `va_start`/`va_end`.
extern char*
dtstr_format_va(const char* format, va_list args);

// Warning: On failure the original @p existing pointer remains valid and owned
// by the caller; do not lose it.
extern char*
dtstr_concat_format(char* existing,
                    const char* separator,
                    const char* format,
                    ...);

// Warning: On failure the original @p existing remains unchanged and still
// allocated; keep a backup until success.
extern char*
dtstr_concat_format_va(char* existing,
                       const char* separator,
                       const char* format,
                       va_list args);
// Note: Dispose with ::dtstr_dispose() to keep the ledger accurate.
extern char*
dtstr_dup(const char* str);

// Warning: Call exactly once per allocation; double-free or freeing non-module
// memory violates the ledger.
extern void
dtstr_dispose(char* str);

#if MARKDOWN_DOCUMENTATION
// clang-format off
// --8<-- [start:markdown-documentation]

# dtstr

Heap-allocated string formatting and concatenation helpers.

This group of functions provides formatted string construction for heap-allocated buffers.
I provides printf-style formatting with explicit ownership.
It also supports appending formatted text to existing allocations with optional separators.
The implementation favors explicit allocation tracking via a ledger.

## Mini-guide

- Allocate formatted strings by calling the formatting helpers and disposing them explicitly.
- Append formatted text to an existing allocation by using the concat helpers.
- Preserve ownership on failure by checking for NULL and retaining the original pointer.
- Pair all successful allocations with exactly one dispose call to keep the ledger accurate and avoid memory leaks.

## Example

```c
char* s = dtstr_format("value=%d", 42);
s = dtstr_concat_format(s, ", ", "status=%s", "ok");
dtstr_dispose(s);
```


## Functions

### dtstr_concat_format

Appends formatted text to an existing allocation with an optional separator.

Params:

> `char* existing` Existing heap-allocated string to extend.  
> `const char* separator` Optional separator inserted before the formatted suffix.  
> `const char* format` printf-style format string.  

Return: `char*` Pointer to the resized allocation on success, or NULL on failure.

### dtstr_concat_format_va

Appends variadic formatted text to an existing allocation with an optional separator.

Params:

> `char* existing` Existing heap-allocated string to extend.  
> `const char* separator` Optional separator inserted before the formatted suffix.  
> `const char* format` printf-style format string.  
> `va_list args` Variadic argument list for formatting.  

Return: `char*` Pointer to the resized allocation on success, or NULL on failure.

### dtstr_dispose

Releases a string allocation created by this module and updates the ledger.

Params:

> `char* str` Heap-allocated string to release.  

Return: `void`  No return value.

### dtstr_dup

Duplicates a C string into a tracked heap allocation.

Params:

> `const char* str` Source string to duplicate.  

Return: `char*` Newly allocated duplicate, or NULL on failure.

### dtstr_format

Formats a string into a newly allocated buffer.

Params:

> `const char* format` printf-style format string.  

Return: `char*` Newly allocated formatted string, or NULL on failure.

### dtstr_format_va

Formats a string into a newly allocated buffer using a variadic list.

Params:

> `const char* format` printf-style format string.  
> `va_list args` Variadic argument list for formatting.  

Return: `char*` Newly allocated formatted string, or NULL on failure.

<!-- FG_IDC: 22d9f67c-ed19-438a-a962-d1dbc705a8ef | FG_UTC: 2026-01-17T13:18:14Z FG_MAN=yes -->

// --8<-- [end:markdown-documentation]
// clang-format on
#endif
