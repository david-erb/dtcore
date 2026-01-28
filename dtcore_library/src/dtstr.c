#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dtstr.h>

// for ledger tracking of mallocs/frees
#include <dtcore/dtledger.h>
DTLEDGER_REGISTER(dtstr)

// ----------------------------------------------------------------
// return a malloc'ed string made from sprintf of the variable argument list

char*
dtstr_format(const char* format, ...)
{
    if (format == NULL)
        return NULL;

    va_list args;
    va_start(args, format);
    char* new_string = dtstr_format_va(format, args);
    va_end(args);

    return new_string;
}

// ----------------------------------------------------------------
// return a malloc'ed string made from sprintf of the variadic argument list

char*
dtstr_format_va(const char* format, va_list args)
{
    if (format == NULL)
        return NULL;

    // Measure length
    va_list args2;
    va_copy(args2, args);
    int length = vsnprintf(NULL, 0, format, args2);
    va_end(args2);

    if (length < 0)
        return NULL;

    size_t buf_size = (size_t)length + 1;
    char* new_string = (char*)malloc(buf_size);
    if (new_string == NULL)
        return NULL;

    // Format using a fresh copy of the va_list
    va_list args3;
    va_copy(args3, args);
    (void)vsnprintf(new_string, buf_size, format, args3);
    va_end(args3);

    DTLEDGER_INCREMENT(dtstr, buf_size);

    return new_string;
}

// ----------------------------------------------------------------
// Return a malloc'ed string made from sprintf of the variable argument list.

char*
dtstr_concat_format(char* existing, const char* separator, const char* format, ...)
{
    if (format == NULL)
        return NULL;

    va_list args;
    va_start(args, format);

    char* new_string;
    // caller is giving no starting string?
    if (existing == NULL)
    {
        // just make a completely new one
        new_string = dtstr_format_va(format, args);
    }
    else
    {
        // append to the existing one, with separator if given
        new_string = dtstr_concat_format_va(existing, separator, format, args);
    }

    va_end(args);

    return new_string;
}

// ----------------------------------------------------------------
// return a malloc'ed string made from sprintf of the variadic argument list, concatenated
// onto the existing string, with separator if given.

char*
dtstr_concat_format_va(char* existing, const char* separator, const char* format, va_list args)
{
    if (existing == NULL)
        return NULL;
    if (format == NULL)
        return NULL;

    // Measure length of formatted suffix
    va_list args2;
    va_copy(args2, args);
    int add_len = (int)vsnprintf(NULL, 0, format, args2);
    va_end(args2);

    if (add_len < 0)
        return NULL;

    size_t existing_length = strlen(existing);
    size_t separator_length = (separator != NULL) ? strlen(separator) : 0;

    // total bytes including trailing '\0'
    size_t total_length = existing_length + separator_length + (size_t)add_len + 1;

    // Realloc without touching ledger until success; do NOT free on failure
    char* resized = (char*)realloc(existing, total_length);
    if (resized == NULL)
    {
        // existing remains valid and owned by caller
        return NULL;
    }

    // Copy separator if present
    if (separator_length > 0)
    {
        memcpy(resized + existing_length, separator, separator_length);
    }

    // Format the suffix into place using a fresh copy of the va_list
    va_list args3;
    va_copy(args3, args);
    (void)vsnprintf(resized + existing_length + separator_length,
      (size_t)add_len + 1, // space for suffix + NUL
      format,
      args3);
    va_end(args3);

    // Adjust ledger: old string (existing_length+1) replaced by new (total_length)
    DTLEDGER_DECREMENT(dtstr, existing_length + 1);
    DTLEDGER_INCREMENT(dtstr, total_length);

    return resized;
}

// ----------------------------------------------------------------
char*
dtstr_dup(const char* str)
{
    if (str == NULL)
        return NULL;

    size_t len = strlen(str);
    char* dup = (char*)malloc(len + 1);
    if (dup == NULL)
        return NULL;

    memcpy(dup, str, len);
    dup[len] = '\0';

    DTLEDGER_INCREMENT(dtstr, len + 1);

    return dup;
}

// ----------------------------------------------------------------
void
dtstr_dispose(char* str)
{
    if (str == NULL)
        return;

    size_t total = strlen(str) + 1;
    free(str);

    DTLEDGER_DECREMENT(dtstr, total);
}
