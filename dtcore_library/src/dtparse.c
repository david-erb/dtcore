#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <dtcore/dtparse.h>

// ----------------------------------------------------------------
bool
dtparse_int32(const char* string, int32_t* out_value)
{
    if (string == NULL)
        return false;

    const char* p = string;

    // Optional minus sign only
    bool negative = false;
    if (*p == '-')
    {
        negative = true;
        p++;
    }

    // Must have at least one digit
    if (*p < '0' || *p > '9')
        return false;

    // Parse using manual accumulation to enforce int32 range precisely
    int64_t acc = 0;

    while (*p >= '0' && *p <= '9')
    {
        acc = acc * 10 + (*p - '0');

        // Check overflow early using int64 accumulator
        if (!negative && acc > INT32_MAX)
            return false;
        if (negative && -acc < INT32_MIN)
            return false;

        p++;
    }

    // No trailing characters allowed
    if (*p != '\0')
        return false;

    // Store result if caller wants it
    if (out_value != NULL)
    {
        int32_t result = (negative ? -(int32_t)acc : (int32_t)acc);
        *out_value = result;
    }

    return true;
}
