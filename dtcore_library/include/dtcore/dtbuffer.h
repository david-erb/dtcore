/*
 * dtbuffer -- Heap-backed buffer descriptor with explicit ownership semantics.
 *
 * Allocates or wraps a byte payload behind a lightweight descriptor that
 * tracks length and ownership flags.  Only buffers created via
 * dtbuffer_create() free their payload on disposal; wrapped buffers leave
 * lifecycle management to the caller.  All allocations are tracked through
 * the dtbuffer ledger channel.
 *
 * cdox v1.0.2
 */
#pragma once
// See markdown documentation at the end of this file.

// Heap buffer utilities for small, explicit data moves.

#include <dtcore/dterr.h>
#include <dtcore/dtledger.h>

#define DTBUFFER_CREATED 0x00000001
#define DTBUFFER_WRAPPED 0x00000002

// Declare a ledger channel named "dtbuffer" for allocation accounting.
DTLEDGER_DECLARE(dtbuffer);

struct dtbuffer_t;
typedef struct dtbuffer_t dtbuffer_t;

struct dtbuffer_t
{
  void* payload;  ///< Owned or borrowed payload bytes; NULL when empty.
  int32_t length; ///< Number of bytes currently exposed through `payload`.
  int32_t flags;  ///< Bitwise flags describing how the buffer was initialized.
};

// Note: Dispose the buffer with dtbuffer_dispose() to release both the
// descriptor and payload.
extern dterr_t*
dtbuffer_create(dtbuffer_t** self, int32_t length);

// Warning: dtbuffer_dispose() will not free `payload`; manage its lifetime
// separately.
extern dterr_t*
dtbuffer_wrap(dtbuffer_t* self, void* payload, int32_t length);

// Note: Only buffers created via dtbuffer_create() free their payload during
// disposal. Note: Always dispose with to keep the ledger accurate.
extern void
dtbuffer_dispose(dtbuffer_t* self);

#if MARKDOWN_DOCUMENTATION
// clang-format off
// --8<-- [start:markdown-documentation]

# dtbuffer

Heap-backed buffer descriptor for explicit ownership control

This group of functions provides heap buffer utilities for small, explicit data moves.
It is intended for code that needs to model ownership and lifetime of byte payloads explicitly.
The implementation has simple contiguous allocation with optional payload wrapping.

Each create/dispose pair updates a ledger channel named "dtbuffer" to track current
and peak memory usage.

## Mini-guide

- Create an owning buffer by calling `dtbuffer_create`, to allocate a single heap block containing both descriptor and payload by allocating heap memory.
- Wrap an existing payload by calling `dtbuffer_wrap`, which borrows the caller-provided memory by storing the pointer without taking ownership.
- Dispose every buffer with `dtbuffer_dispose` to ensure memory release and update of the ledger.
- Treat the `payload` field as mutable storage, because the API exposes a writable pointer and does not perform internal copying.

## Example

```c
dtbuffer_t* buf = NULL;
dterr_t* err = dtbuffer_create(&buf, 128);
memset(buf->payload, 0, buf->length);
if (err == NULL)
{
    /* use buf->payload and buf->length */
    dtbuffer_dispose(buf);
}
```

## Data structures

### dtbuffer_t

Describes a byte buffer with explicit ownership semantics.

Members:

> `void* payload` Owned or borrowed payload bytes; NULL when empty.  
> `int32_t length` Number of bytes currently exposed through `payload`.  
> `int32_t flags` Bitwise flags describing how the buffer was initialized.  


## Functions

### dtbuffer_create

Allocates a buffer descriptor and payload as a single heap block.
This is the alternate to `dtbuffer_wrap`, which borrows caller-managed memory.

Params:

> `dtbuffer_t** self` Receives the newly allocated buffer descriptor.  
> `int32_t length` Number of payload bytes to allocate.  

Return: `dterr_t*` Error object on failure, or NULL on success.

### dtbuffer_dispose

Releases buffer resources according to how the buffer was initialized.  Updates ledger.

Params:

> `dtbuffer_t* self` Buffer to dispose.  

Return: `void`  No return value.

### dtbuffer_wrap

Initializes a buffer descriptor to reference an existing payload.
This is the alternate to `dtbuffer_create`, which allocates both descriptor and payload together.

Params:

> `dtbuffer_t* self` Buffer descriptor to initialize.  
> `void* payload` Caller-managed payload memory.  
> `int32_t length` Number of bytes exposed through the payload.  

Return: `dterr_t*` Error object on failure, or NULL on success.

<!-- FG_IDC: 412b642c-4a0f-40c6-bb78-2334e1f1c207 | FG_UTC: 2026-01-17T09:29:52Z | FG_MAN=yes -->

// --8<-- [end:markdown-documentation]
// clang-format on
#endif
