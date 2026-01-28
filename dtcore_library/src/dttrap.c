#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dterr.h>
#include <dtcore/dtlog.h>
#include <dtcore/dtstr.h>

#include <dtcore/dttrap.h>

#define TAG "dttrap"

// -------------------------------------------------------------------------------
dterr_t*
dttrap_init(dttrap_t* self)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);
    memset(self, 0, sizeof(dttrap_t));

    self->stop_trapping_after_count = 10;

cleanup:
    return dterr;
}

// -------------------------------------------------------------------------------
dterr_t*
dttrap_count(dttrap_t* self)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);

    self->count += 1;

    if (self->stop_trapping_after_count > 0 && self->count == self->stop_trapping_after_count)
    {
        if (self->callback != NULL)
        {
            self->callback(self, self->callback_user_context);
        }
    }

cleanup:
    return dterr;
}

// -------------------------------------------------------------------------------
dterr_t*
dttrap_append(dttrap_t* self, const char* format, ...)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);
    if (self->stop_trapping_after_count > 0 && self->count >= self->stop_trapping_after_count)
    {
        return NULL;
    }

    va_list args;
    va_start(args, format);

    if (self->lines == NULL)
        self->lines = dtstr_format_va(format, args);
    else
        self->lines = dtstr_concat_format_va(self->lines, "\n", format, args);

    va_end(args);

cleanup:
    return dterr;
}

// -------------------------------------------------------------------------------
dterr_t*
dttrap_debug(dttrap_t* self)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);

    dtlog_debug(TAG, "%s", self->lines);

cleanup:
    return dterr;
}

// -------------------------------------------------------------------------------
dterr_t*
dttrap_set_callback(dttrap_t* self, dttrap_callback_fn callback, void* user_context)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);

    self->callback = callback;
    self->callback_user_context = user_context;

cleanup:
    return dterr;
}

// -------------------------------------------------------------------------------
void
dttrap_dispose(dttrap_t* self)
{
    if (self == NULL)
        return;

    if (self->lines != NULL)
        dtstr_dispose(self->lines);

    memset(self, 0, sizeof(*self));
}