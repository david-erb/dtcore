#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>

#include <dtcore/dterr.h>
#include <dtcore/dtstr.h>

#include <dtcore/dtledger.h>
DTLEDGER_REGISTER(dterr)

// ----------------------------------------------------------------
dterr_t*
dterr_new(int32_t error_code,
  int32_t line_number,
  const char* source_file,
  const char* source_function,
  dterr_t* inner_err,
  const char* format,
  ...)
{
    if (format == NULL)
        return NULL;

    dterr_t* self = (dterr_t*)malloc(sizeof(dterr_t));
    if (self == NULL)
        return NULL;

    // Initialize early so dispose is safe on any early-return path.
    self->error_code = error_code;
    self->line_number = line_number;
    self->source_file = source_file;
    self->source_function = source_function;
    self->inner_err = inner_err;
    self->message = NULL;
    self->dispose = dterr_dispose;

    va_list args;
    va_start(args, format);
    self->message = dtstr_format_va(format, args);
    va_end(args);

    if (self->message == NULL)
    {
        // Treat OOM/format failure as construction failure.
        free(self);
        return NULL;
    }

    DTLEDGER_INCREMENT(dterr, (int)sizeof(dterr_t));
    return self;
}

// ----------------------------------------------------------------
void
dterr_each(dterr_t* self, dterr_each_callback_t callback, void* context)
{
    if (self == NULL || callback == NULL)
        return;

    // Preserve original bottom-up order: inner first, then self.
    if (self->inner_err != NULL)
    {
        dterr_each(self->inner_err, callback, context);
    }
    callback(self, context);
}

// ----------------------------------------------------------------
dterr_t*
dterr_append(dterr_t* self, dterr_t* that)
{
    // nothing to append to, so just return 'that' as the new chain head
    if (self == NULL)
        return that;

    if (that == NULL)
        return self;

    // Prevent cycles: if 'that' is already somewhere in 'self' chain, do nothing.
    for (dterr_t* scan = self; scan != NULL; scan = scan->inner_err)
    {
        if (scan == that)
        {
            return self; // would create a cycle
        }
    }

    // Iterative tail-append (avoid deep recursion).
    dterr_t* tail = self;
    while (tail->inner_err != NULL)
    {
        tail = tail->inner_err;
    }
    tail->inner_err = that;
    return self;
}

// ----------------------------------------------------------------
void
dterr_dispose(dterr_t* self)
{
    if (self == NULL)
        return;

    if (self->inner_err != NULL && self->inner_err->dispose != NULL)
    {
        self->inner_err->dispose(self->inner_err);
        self->inner_err = NULL;
    }

    if (self->message != NULL)
    {
        dtstr_dispose(self->message);
        self->message = NULL;
    }

    DTLEDGER_DECREMENT(dterr, (int)sizeof(dterr_t));
    free(self);
}
