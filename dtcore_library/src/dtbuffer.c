#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dtbuffer.h>
#include <dtcore/dterr.h>
#include <dtcore/dtledger.h>
#include <dtcore/dtlog.h>

DTLEDGER_REGISTER(dtbuffer);

#define TAG "dtbuffer"

// -------------------------------------------------------------------------------
dterr_t*
dtbuffer_wrap(dtbuffer_t* self, void* payload, int32_t length)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);
    DTERR_ASSERT_NOT_NULL(payload);

    if (length < 0)
        return dterr_new(DTERR_BADARG, DTERR_LOC, NULL, "buffer length must be non-negative");

    self->payload = payload;
    self->length = length;
    self->flags = DTBUFFER_WRAPPED;

cleanup:
    return dterr;
}

// -------------------------------------------------------------------------------
dterr_t*
dtbuffer_create(dtbuffer_t** self, int32_t length)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);

    int total = sizeof(dtbuffer_t) + length;

    *self = (dtbuffer_t*)malloc(total);
    if (*self == NULL)
        return dterr_new(DTERR_NOMEM, DTERR_LOC, NULL, "failed to allocate %d bytes for buffer", total);

    (*self)->length = length;
    (*self)->flags = DTBUFFER_CREATED;
    (*self)->payload = (void*)(*self + 1);

    DTLEDGER_INCREMENT(dtbuffer, total);

cleanup:
    return dterr;
}

// -------------------------------------------------------------------------------
void
dtbuffer_dispose(dtbuffer_t* self)
{
    if (self != NULL)
    {
        int total = sizeof(dtbuffer_t) + self->length;
        bool created = (self->flags & DTBUFFER_CREATED) != 0;
        memset(self, 0, sizeof(dtbuffer_t));
        if (created)
        {
            free(self);

            DTLEDGER_DECREMENT(dtbuffer, total);
        }
    }
}