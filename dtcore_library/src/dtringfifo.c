#include <stdatomic.h>
#include <string.h>

#include <dtcore/dterr.h>
#include <dtcore/dtlog.h>

#include <dtcore/dtringfifo.h> // adjust include path to your layout

// --------------------------------------------------------------------------------------
dterr_t*
dtringfifo_init(dtringfifo_t* self)
{
    dterr_t* dterr = NULL;

    DTERR_ASSERT_NOT_NULL(self);

    memset(self, 0, sizeof(dtringfifo_t));

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------
dterr_t*
dtringfifo_configure(dtringfifo_t* self, const dtringfifo_config_t* cfg)
{
    dterr_t* dterr = NULL;

    DTERR_ASSERT_NOT_NULL(self);
    DTERR_ASSERT_NOT_NULL(cfg);
    DTERR_ASSERT_NOT_NULL(cfg->buffer);

    if (cfg->capacity < 2)
    {
        dterr = dterr_new(DTERR_BADARG, DTERR_LOC, NULL, "dtringfifo capacity must be >= 2 to support lock-free SPSC");
        goto cleanup;
    }

    self->buffer = cfg->buffer;
    self->capacity = cfg->capacity;
    self->head = 0;
    self->tail = 0;

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------
void
dtringfifo_reset(dtringfifo_t* self)
{
    if (!self)
        return;

    // Caller must ensure no concurrent push/pop while resetting.
    self->head = 0;
    self->tail = 0;
}

// --------------------------------------------------------------------------------------
int32_t
dtringfifo_push(dtringfifo_t* self, const uint8_t* src, int32_t src_len)
{
    if (!self || !self->buffer || self->capacity <= 1 || !src || src_len <= 0)
        return 0;

    int32_t capacity = self->capacity;

    int32_t head = atomic_load_explicit(&self->head, memory_order_relaxed);
    int32_t tail = atomic_load_explicit(&self->tail, memory_order_acquire);

    int32_t free_space;
    if (head >= tail)
        free_space = (capacity - (head - tail) - 1);
    else
        free_space = (tail - head - 1);

    if (free_space <= 0)
        return 0;

    int32_t to_write = (src_len < free_space) ? src_len : free_space;

    // linear copy up to end
    int32_t first_chunk = capacity - head;
    if (first_chunk > to_write)
        first_chunk = to_write;

    memcpy(&self->buffer[head], src, first_chunk);
    head += first_chunk;
    if (head >= capacity)
        head = 0;

    int32_t remaining = to_write - first_chunk;
    if (remaining > 0)
    {
        memcpy(&self->buffer[0], src + first_chunk, remaining);
        head = remaining;
    }

    // Publish the new head with release semantics
    atomic_store_explicit(&self->head, head, memory_order_release);

    return to_write;
}

// --------------------------------------------------------------------------------------
int32_t
dtringfifo_pop(dtringfifo_t* self, uint8_t* dest, int32_t dest_len)
{
    if (!self || !self->buffer || self->capacity <= 1 || !dest || dest_len <= 0)
        return 0;

    int32_t capacity = self->capacity;

    int32_t head = atomic_load_explicit(&self->head, memory_order_acquire);
    int32_t tail = atomic_load_explicit(&self->tail, memory_order_relaxed);

    int32_t available;
    if (head >= tail)
        available = head - tail;
    else
        available = capacity - (tail - head);

    if (available <= 0)
        return 0;

    int32_t to_read = (available < dest_len) ? available : dest_len;

    // linear copy
    int32_t first_chunk = capacity - tail;
    if (first_chunk > to_read)
        first_chunk = to_read;

    memcpy(dest, &self->buffer[tail], first_chunk);
    tail += first_chunk;
    if (tail >= capacity)
        tail = 0;

    int32_t remaining = to_read - first_chunk;
    if (remaining > 0)
    {
        memcpy(dest + first_chunk, &self->buffer[0], remaining);
        tail = remaining;
    }

    // Publish new tail with release semantics
    atomic_store_explicit(&self->tail, tail, memory_order_release);

    return to_read;
}
