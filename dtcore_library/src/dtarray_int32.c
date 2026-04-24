#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dtarray_int32.h>
#include <dtcore/dterr.h>
#include <dtcore/dtheaper.h>
#include <dtcore/dtpackx.h>

// ------------------------------------------------------------------------------------

dterr_t*
dtarray_int32_create(dtarray_int32_t** array_int32, int32_t count)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(array_int32);

    if (count <= 0)
    {
        dterr = dterr_new(DTERR_BADARG, DTERR_LOC, NULL, "invalid count=%" PRId32, count);
        goto cleanup;
    }

    int32_t bytes = sizeof(dtarray_int32_t) + count * sizeof(int32_t);

    // zero-initialize the elements for predictable state
    dtarray_int32_t* a = NULL;
    DTERR_C(dtheaper_alloc_and_zero(bytes, "dtarray_int32_t items", (void**)&a));

    a->count = count;
    a->items = (int32_t*)(a + 1); /* items follow struct in the same allocation */

    *array_int32 = a;

cleanup:
    if (dterr != NULL)
        dterr = dterr_new(dterr->error_code, DTERR_LOC, dterr, "unable to create int32 array of count %" PRId32, count);

    return dterr;
}

// ------------------------------------------------------------------------------------
void
dtarray_int32_dispose(dtarray_int32_t* self)
{
    if (!self)
        return;

    dtheaper_free(self);
}

// ------------------------------------------------------------------------------------
// return number of bytes required to pack this array
int32_t
dtarray_int32_pack_length(const dtarray_int32_t* self)
{
    if (!self || self->count < 0)
        return -1;

    /* [count:int32] + count * [element:int32] */
    const int32_t elen = dtpackx_pack_int32_length();
    return elen + (int32_t)((int64_t)self->count * elen);
}

// ------------------------------------------------------------------------------------
// Serialize: [count][items...]
int32_t
dtarray_int32_pack(const dtarray_int32_t* self, uint8_t* output, int32_t offset, int32_t buflen)
{
    if (!self || !output || offset < 0 || buflen < 0 || self->count < 0)
        return -1;

    int32_t p = offset;

    int32_t n = dtpackx_pack_int32(self->count, output, p, buflen);
    if (n < 0)
        return -1;
    p += n;

    for (int32_t i = 0; i < self->count; ++i)
    {
        n = dtpackx_pack_int32(self->items[i], output, p, buflen);
        if (n < 0)
            return -1;
        p += n;
    }

    return p - offset;
}

// ------------------------------------------------------------------------------------
// Deserialize: [count][items...]
//    Behavior:
//    - If payload count < capacity, remaining elements are zeroed.
//    - If payload count > capacity, extra elements are read and discarded to keep the stream in sync.
int32_t
dtarray_int32_unpack(dtarray_int32_t* self, const uint8_t* input, int32_t offset, int32_t buflen)
{
    if (!self || !input || offset < 0 || buflen < 0 || self->count < 0)
        return -1;

    int32_t p = offset;

    int32_t count = 0;
    int32_t n = dtpackx_unpack_int32(input, p, buflen, &count);
    if (n < 0)
        return -1;
    p += n;

    /* Reject malformed negative counts */
    if (count < 0)
        return -1;

    /* Store up to capacity; read-and-discard the rest to advance the cursor. */
    const int32_t store_count = (count < self->count) ? count : self->count;

    for (int32_t i = 0; i < count; ++i)
    {
        int32_t v = 0;
        n = dtpackx_unpack_int32(input, p, buflen, &v);
        if (n < 0)
            return -1;
        p += n;

        if (i < store_count)
        {
            self->items[i] = v;
        }
    }

    /* If incoming had fewer elements than capacity, zero the tail to avoid stale data. */
    if (count < self->count)
    {
        memset(&self->items[count], 0, (size_t)(self->count - count) * sizeof(int32_t));
    }

    return p - offset;
}

// 2026-04-11 crev v2.0.1
