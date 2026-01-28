#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dterr.h>
#include <dtcore/dtheaper.h>
#include <dtcore/dtlog.h>

// for ledger tracking of mallocs/frees
#include <dtcore/dtledger.h>
DTLEDGER_REGISTER(dtheaper)

#define DTHEAPER_MAGIC1 0xDEADBEEF
#define DTHEAPER_MAGIC2 0xBAADF00D

#define TAG "dtheaper"
#define dtlog_debug(TAG, ...)

// ----------------------------------------------------------------
// 16-byte header to preserve alignment and check for memory corruption
// the 16 bytes are supposed to be invisible to the user of the payload
// we use 16 bytes to keep the promised payload 16-byte aligned if that is what malloc returned
typedef struct dtheaper_header_t
{
    uint32_t magic1;
    int32_t length;
    int32_t length_negated;
    uint32_t magic2;
} dtheaper_header_t;

// ----------------------------------------------------------------
// initialize the header at the front of the buffer
static void
dtheaper__initialize_header(dtheaper_header_t* header, int32_t length)
{
    header->magic1 = DTHEAPER_MAGIC1;
    header->length = length;
    header->length_negated = -length;
    header->magic2 = DTHEAPER_MAGIC2;
}

// ----------------------------------------------------------------
// validate we have a good header
static bool
dtheaper__validate_header(dtheaper_header_t* header)
{
    if (header == NULL)
        return false;
    if (header->magic1 != DTHEAPER_MAGIC1)
        return false;
    if (header->magic2 != DTHEAPER_MAGIC2)
        return false;
    if (header->length <= 0)
        return false;
    if (header->length_negated != -header->length)
        return false;

    return true;
}

// ----------------------------------------------------------------
// return a malloc'ed buffer like dtheaper_alloc, but filled with zeros

dterr_t*
dtheaper_alloc_and_zero(int32_t length, const char* why, void** payload)
{
    dterr_t* dterr = NULL;
    DTERR_C(dtheaper_alloc(length, why, payload));
    DTERR_C(dtheaper_fill(*payload, 0));
cleanup:
    return dterr;
}

// ----------------------------------------------------------------
// return a malloc'ed buffer, with length stored in extra space allocated at the front
// 'why' is a descriptive string for logging purposes
// returned payload is 16-byte aligned

dterr_t*
dtheaper_alloc(int32_t length, const char* why, void** payload)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(payload);
    DTERR_ASSERT_NOT_NULL(why);
    *payload = NULL;

    if (length <= 0)
    {
        return dterr_new(DTERR_BADARG, DTERR_LOC, NULL, "length must be > 0, got %" PRId32, length);
    }

    dtheaper_header_t* header = (dtheaper_header_t*)malloc(sizeof(dtheaper_header_t) + (size_t)length);
    if (header == NULL)
    {
        dterr = dterr_new(DTERR_NOMEM,
          DTERR_LOC,
          NULL,
          "failed to allocate %" PRId32 " + %" PRId32 " bytes for %s",
          length,
          (int32_t)(sizeof(dtheaper_header_t)),
          why);
        goto cleanup;
    }

    // store the header at the front of the buffer
    dtheaper__initialize_header(header, length);

    DTLEDGER_INCREMENT(dtheaper, length);

    *payload = (void*)(header + 1); // return pointer to payload area

    dtlog_debug(TAG, "%s(): allocated %" PRId32 " bytes at %p for %s", __func__, length, *payload, why);

cleanup:
    return dterr;
}

// ----------------------------------------------------------------
// clear the alloced'ed buffer to value using length stored at front
dterr_t*
dtheaper_fill(void* payload, uint8_t fill)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(payload);

    dtheaper_header_t* header = (dtheaper_header_t*)payload - 1;

    if (!dtheaper__validate_header(header))
    {
        dterr = dterr_new(DTERR_FAIL, DTERR_LOC, NULL, "invalid dtheaper header detected");
        goto cleanup;
    }

    memset(payload, fill, (size_t)header->length);
cleanup:
    return dterr;
}

// ----------------------------------------------------------------
// free the alloced'ed buffer, first zeroing the length at front
void
dtheaper_free(void* payload)
{
    if (payload == NULL)
        return;

    dtheaper_header_t* header = (dtheaper_header_t*)payload - 1;

    // if it doesn't look like a valid header, then don't touch it
    if (!dtheaper__validate_header(header))
        return;

    DTLEDGER_DECREMENT(dtheaper, header->length);

    dtlog_debug(TAG, "%s(): freeing %" PRId32 " bytes at %p", __func__, header->length, payload);

    // clear the header to zero for safety
    memset(header, 0, sizeof(dtheaper_header_t));

    free(header);
}
