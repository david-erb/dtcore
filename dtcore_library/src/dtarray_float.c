#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dtarray_float.h>
#include <dtcore/dterr.h>
#include <dtcore/dtheaper.h>
#include <dtcore/dtpackx.h>

// --------------------------------------------------------------------------------------
// API

dterr_t*
dtarray_float_create(dtarray_float_t** array_float, int count)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(array_float);

    if (count <= 0)
    {
        dterr = dterr_new(DTERR_BADARG, DTERR_LOC, NULL, "invalid count=%d", count);
        goto cleanup;
    }

    int32_t bytes = (int32_t)sizeof(dtarray_float_t) + (int32_t)count * (int32_t)sizeof(float);

    // zero-initialize the elements for predictable state
    dtarray_float_t* a = NULL;
    DTERR_C(dtheaper_alloc_and_zero(bytes, "dtarray_float_t items", (void**)&a));

    a->count = count;
    a->items = (float*)(a + 1); /* items follow struct in the same allocation */

    *array_float = a;

cleanup:
    if (dterr != NULL)
        dterr = dterr_new(dterr->error_code, DTERR_LOC, dterr, "unable to create float array of count %d", count);

    return dterr;
}

// --------------------------------------------------------------------------------------
void
dtarray_float_copy(const dtarray_float_t* src, dtarray_float_t* dest)
{
    if (!dest || !src)
        return;

    int32_t count = dest->count < src->count ? dest->count : src->count;
    memcpy(dest->items, src->items, sizeof(float) * (size_t)count);
}

// --------------------------------------------------------------------------------------
bool
dtarray_float_equals(const dtarray_float_t* a, const dtarray_float_t* b)
{
    if (!a || !b)
        return false;
    if (a->count != b->count)
        return false;
    return memcmp(a->items, b->items, sizeof(float) * (size_t)a->count) == 0;
}

// --------------------------------------------------------------------------------------
void
dtarray_float_dispose(dtarray_float_t* self)
{
    if (!self)
        return;

    dtheaper_free(self);
}

// --------------------------------------------------------------------------------------
// return number of bytes required to pack this array
int32_t
dtarray_float_pack_length(const dtarray_float_t* self)
{
    if (!self || self->count < 0)
        return -1;

    /* [count:int32] + count * [element:float] */
    return dtpackx_pack_int32_length() + (self->count * dtpackx_pack_float_length());
}

// --------------------------------------------------------------------------------------
// Serialize: [count][items...]
int32_t
dtarray_float_pack(const dtarray_float_t* self, uint8_t* output, int32_t offset, int32_t length)
{
    if (!self || !output || offset < 0 || length < 0 || self->count < 0)
        return -1;

    int32_t p = offset;

    int32_t n = dtpackx_pack_int32(self->count, output, p, length);
    if (n < 0)
        return -1;
    p += n;

    for (int32_t i = 0; i < self->count; ++i)
    {
        n = dtpackx_pack_float(self->items[i], output, p, length);
        if (n < 0)
            return -1;
        p += n;
    }

    return p - offset;
}

// --------------------------------------------------------------------------------------
// Deserialize: [count][items...]
//    Behavior:
//    - If payload count < capacity, remaining elements are zeroed.
//    - If payload count > capacity, extra elements are read and discarded to keep the stream in sync.
int32_t
dtarray_float_unpack(dtarray_float_t* self, const uint8_t* input, int32_t offset, int32_t length)
{
    if (!self || !input || offset < 0 || length < 0)
        return -1;

    int32_t p = offset;

    int32_t count = 0;
    int32_t n = dtpackx_unpack_int32(input, p, length, &count);
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
        float v = 0;
        n = dtpackx_unpack_float(input, p, length, &v);
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
        memset(&self->items[count], 0, (size_t)(self->count - count) * sizeof(float));
    }

    return p - offset;
}
