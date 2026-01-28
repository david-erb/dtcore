#include <stdlib.h>
#include <string.h>

#include <dtcore/dterr.h>

#include <dtcore/dtflipper.h>

// -------------------------------------------------------------------------------
dterr_t*
dtflipper_init(dtflipper_t* self, void* upper_buffer, void* lower_buffer)
{
    dterr_t* dterr = NULL;
    self->upper_buffer = upper_buffer;
    self->lower_buffer = lower_buffer;
    self->upper_busy = 0;
    self->lower_busy = 0;
    return dterr;
}

// -------------------------------------------------------------------------------
// flip suitable for a critical section, does not access system resources
dtflipper_flip_result_e
dtflipper_flip_critical(dtflipper_t* self)
{
    if (self->upper_busy)
        return UPPER_BUSY;

    if (self->lower_busy)
        return LOWER_BUSY;

    void* temp = self->upper_buffer;
    self->upper_buffer = self->lower_buffer;
    self->lower_buffer = temp;

    return OK;
}

// -------------------------------------------------------------------------------
dterr_t*
dtflipper_flip(dtflipper_t* self)
{
    dterr_t* dterr = NULL;

    dtflipper_flip_result_e flip_result = dtflipper_flip_critical(self);

    if (flip_result == UPPER_BUSY)
    {
        dterr = dterr_new(DTERR_BUSY, DTERR_LOC, NULL, "upper buffer is busy so cannot flip buffers");
        goto cleanup;
    }

    if (flip_result == LOWER_BUSY)
    {
        dterr = dterr_new(DTERR_BUSY, DTERR_LOC, NULL, "lower buffer is busy so cannot flip buffers");
        goto cleanup;
    }

cleanup:
    return dterr;
}

// -------------------------------------------------------------------------------
dterr_t*
dtflipper_control_upper(dtflipper_t* self, void** payload)
{
    dterr_t* dterr = NULL;

    if (self->upper_busy)
    {
        dterr = dterr_new(DTERR_BUSY, DTERR_LOC, NULL, "upper buffer is busy so cannot take control");
        goto cleanup;
    }

    self->upper_busy = 1;

    *payload = self->upper_buffer;

cleanup:
    return dterr;
}

// -------------------------------------------------------------------------------
dterr_t*
dtflipper_release_upper(dtflipper_t* self)
{
    dterr_t* dterr = NULL;

    self->upper_busy = 0;

    return dterr;
}

// -------------------------------------------------------------------------------
dterr_t*
dtflipper_control_lower(dtflipper_t* self, void** payload)
{
    dterr_t* dterr = NULL;

    if (self->lower_busy)
    {
        dterr = dterr_new(DTERR_BUSY, DTERR_LOC, NULL, "lower buffer is busy so cannot take control");
        goto cleanup;
    }

    self->lower_busy = 1;

    *payload = self->lower_buffer;

cleanup:
    return dterr;
}

// -------------------------------------------------------------------------------
dterr_t*
dtflipper_release_lower(dtflipper_t* self)
{
    dterr_t* dterr = NULL;

    self->lower_busy = 0;

    return dterr;
}
