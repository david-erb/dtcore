#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dtbuffer.h>
#include <dtcore/dterr.h>
#include <dtcore/dtlog.h>
#include <dtcore/dtpackx.h>

#include <dtcore/dtchunker.h>

#define TAG "dtchunker"

static inline int32_t
dt_min_i32(int32_t a, int32_t b)
{
    return (a < b) ? a : b;
}

/* ------------------------------------------------------------------------ */
dterr_t*
dtchunker_init(dtchunker_t* self, int32_t max_chunk_size)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);

    if (max_chunk_size < 8)
    {
        dterr = dterr_new(DTERR_BADARG, DTERR_LOC, NULL, "invalid max_chunk_size=%" PRId32, max_chunk_size);
        goto cleanup;
    }

    memset(self, 0, sizeof(*self));
    self->max_chunk_size = max_chunk_size;

    DTERR_C(dtchunker_reset(self));

    /* header = magic(int32) + source_length(int32) */
    self->header_length = 0;
    self->header_length += dtpackx_pack_int32_length(); /* magic */
    self->header_length += dtpackx_pack_int32_length(); /* source_buffer_length */

cleanup:
    if (dterr != NULL)
    {
        dtchunker_dispose(self);
        dterr = dterr_new(dterr->error_code, DTERR_LOC, dterr, "failed to initialize chunker");
    }
    return dterr;
}

// ------------------------------------------------------------------------
dterr_t*
dtchunker_reset(dtchunker_t* self)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);

    self->working_index = 0;
    self->mode = DTCHUNKER_MODE_UNKNOWN;
    /* Keep working_buffer allocation for reuse; wrapped_buffer aliases it. */

cleanup:
    return dterr;
}

// ------------------------------------------------------------------------
dterr_t*
dtchunker_export(dtchunker_t* self, dtbuffer_t* source_buffer, dtbuffer_t** chunk_buffer)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);
    DTERR_ASSERT_NOT_NULL(source_buffer);
    DTERR_ASSERT_NOT_NULL(chunk_buffer);

    if (self->mode == DTCHUNKER_MODE_IMPORT)
    {
        dterr = dterr_new(DTERR_STATE, DTERR_LOC, NULL, "cannot export while in import mode");
        goto cleanup;
    }

    /* All math in int32_t. Allow zero-length. Reject negative. */
    int32_t src_len = (int32_t)source_buffer->length;
    if (src_len < 0)
    {
        dterr = dterr_new(DTERR_BADARG, DTERR_LOC, NULL, "negative source length");
        goto cleanup;
    }

    /* First export step emits header chunk. Subsequent calls emit payload slices. */
    if (self->mode == DTCHUNKER_MODE_UNKNOWN)
    {
        if (self->working_buffer == NULL || self->working_buffer->length < self->max_chunk_size)
        {
            dtbuffer_dispose(self->working_buffer);
            DTERR_C(dtbuffer_create(&self->working_buffer, self->max_chunk_size));
        }

        self->mode = DTCHUNKER_MODE_EXPORT;

        /* Header-only first chunk */
        dtchunker_header_t header;
        header.magic = DTCHUNKER_MAGIC;
        header.source_buffer_length = src_len;

        int32_t p = 0;
        int32_t n = 0;
        int32_t buflen = self->working_buffer->length;

        n = dtpackx_pack_int32(header.magic, (uint8_t*)self->working_buffer->payload, p, buflen);
        if (n == -1)
        {
            dterr = dterr_new(DTERR_FAIL, DTERR_LOC, NULL, "failed to pack header.magic");
            goto cleanup;
        }
        p += n;

        n = dtpackx_pack_int32(header.source_buffer_length, (uint8_t*)self->working_buffer->payload, p, buflen);
        if (n == -1)
        {
            dterr = dterr_new(DTERR_FAIL, DTERR_LOC, NULL, "failed to pack header.source_buffer_length");
            goto cleanup;
        }
        p += n;

        /* Wrap header slice. Valid until next export() call. */
        DTERR_C(dtbuffer_wrap(&self->wrapped_buffer, self->working_buffer->payload, p));
        *chunk_buffer = &self->wrapped_buffer;
    }
    else
    {
        int32_t idx = self->working_index;
        int32_t bytes_remaining = (src_len > idx) ? (src_len - idx) : 0;

        if (bytes_remaining == 0)
        {
            *chunk_buffer = NULL;
            DTERR_C(dtchunker_reset(self)); /* ready for a new session */
        }
        else
        {
            int32_t bytes_in_chunk = dt_min_i32(bytes_remaining, self->max_chunk_size);

            memcpy(self->working_buffer->payload, (const uint8_t*)source_buffer->payload + idx, (size_t)bytes_in_chunk);

            idx += bytes_in_chunk;
            self->working_index = idx;

            DTERR_C(dtbuffer_wrap(&self->wrapped_buffer, self->working_buffer->payload, bytes_in_chunk));
            *chunk_buffer = &self->wrapped_buffer;
        }
    }

cleanup:
    if (dterr != NULL)
    {
        dterr = dterr_new(dterr->error_code, DTERR_LOC, dterr, "failed to export chunk");
    }
    return dterr;
}

// ------------------------------------------------------------------------
dterr_t*
dtchunker_import(dtchunker_t* self, dtbuffer_t* chunk_buffer, dtbuffer_t** final_buffer)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);
    DTERR_ASSERT_NOT_NULL(chunk_buffer);
    DTERR_ASSERT_NOT_NULL(final_buffer);

    if (self->mode == DTCHUNKER_MODE_EXPORT)
    {
        dterr = dterr_new(DTERR_STATE, DTERR_LOC, NULL, "cannot import while in export mode");
        goto cleanup;
    }

    int32_t chunk_len = (int32_t)chunk_buffer->length;
    if (chunk_len < 0)
    {
        dterr = dterr_new(DTERR_FAIL, DTERR_LOC, NULL, "negative chunk length");
        goto cleanup;
    }

    if (self->mode == DTCHUNKER_MODE_UNKNOWN)
    {
        if (chunk_len < self->header_length)
        {
            dterr = dterr_new(
              DTERR_FAIL, DTERR_LOC, NULL, "invalid header: have=%" PRId32 " need=%" PRId32, chunk_len, self->header_length);
            goto cleanup;
        }

        /* Unpack header */
        dtchunker_header_t header;
        int32_t p = 0;
        int32_t n = 0;

        n = dtpackx_unpack_int32((const uint8_t*)chunk_buffer->payload, p, chunk_len, &header.magic);
        if (n == -1)
        {
            dterr = dterr_new(DTERR_FAIL, DTERR_LOC, NULL, "failed to unpack header.magic");
            goto cleanup;
        }
        p += n;

        n = dtpackx_unpack_int32((const uint8_t*)chunk_buffer->payload, p, chunk_len, &header.source_buffer_length);
        if (n == -1)
        {
            dterr = dterr_new(DTERR_FAIL, DTERR_LOC, NULL, "failed to unpack header.source_buffer_length");
            goto cleanup;
        }
        p += n;

        if (header.magic != DTCHUNKER_MAGIC)
        {
            dterr = dterr_new(DTERR_FAIL, DTERR_LOC, NULL, "invalid header magic");
            goto cleanup;
        }
        if (header.source_buffer_length < 0)
        {
            dterr = dterr_new(DTERR_FAIL, DTERR_LOC, NULL, "negative source length in header");
            goto cleanup;
        }

        /* Prepare destination buffer of exact final size */
        dtbuffer_dispose(self->working_buffer);
        DTERR_C(dtbuffer_create(&self->working_buffer, header.source_buffer_length));

        self->mode = DTCHUNKER_MODE_IMPORT;
        self->working_index = 0;
        *final_buffer = NULL;

        /* If this first chunk also carries payload, consume it now. */
        int32_t rem = chunk_len - self->header_length;
        if (rem > 0)
        {
            int32_t can_copy = dt_min_i32(rem, self->working_buffer->length);
            memcpy((uint8_t*)self->working_buffer->payload,
              (const uint8_t*)chunk_buffer->payload + self->header_length,
              (size_t)can_copy);

            self->working_index = can_copy;

            if (self->working_index == self->working_buffer->length)
            {
                /* complete in the first packet */
                *final_buffer = self->working_buffer;
                self->working_buffer = NULL; /* transfer ownership */
                DTERR_C(dtchunker_reset(self));
            }
            else
            {
                *final_buffer = NULL; /* more to come */
            }
        }
    }
    else
    {
        /* Append payload chunk */
        int32_t idx = self->working_index;
        int32_t cap = self->working_buffer->length;

        if (chunk_len < 0 || idx < 0 || cap < 0 || idx > cap - chunk_len)
        {
            dterr = dterr_new(DTERR_FAIL, DTERR_LOC, NULL, "accumulated chunks exceed source length");
            goto cleanup;
        }

        memcpy((uint8_t*)self->working_buffer->payload + idx, chunk_buffer->payload, (size_t)chunk_len);

        idx += chunk_len;
        self->working_index = idx;

        if (idx < cap)
        {
            *final_buffer = NULL; /* still incomplete */
        }
        else
        {
            *final_buffer = self->working_buffer; /* done */
            self->working_buffer = NULL;          /* transfer ownership */
            DTERR_C(dtchunker_reset(self));
        }
    }

cleanup:
    if (dterr != NULL)
    {
        dterr = dterr_new(dterr->error_code, DTERR_LOC, dterr, "failed to import chunk");
    }
    return dterr;
}

// ------------------------------------------------------------------------
void
dtchunker_dispose(dtchunker_t* self)
{
    if (self == NULL)
        return;

    dtbuffer_dispose(self->working_buffer);
    /* wrapped_buffer aliases working_buffer->payload; nothing to free. */

    memset(self, 0, sizeof(*self));
}
