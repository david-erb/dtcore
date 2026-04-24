/*
 * dtheaper -- Heap allocator with embedded length metadata and ledger tracking.
 *
 * Wraps malloc with a length prefix stored ahead of the payload, enabling
 * fill and release operations that are self-contained and safe.  All
 * allocations and frees are counted through the dtheaper ledger channel,
 * making leaks straightforward to detect in tests.
 *
 * cdox v1.0.2
 */
#pragma once

#include <stdint.h>

#include <dtcore/dterr.h>

// for ledger tracking of mallocs/frees
#include <dtcore/dtledger.h>

// Declare a ledger channel named "dtheaper" for allocation accounting.
DTLEDGER_DECLARE(dtheaper);

// ----------------------------------------------------------------
// return a malloc'ed buffer like dtheaper_alloc, but filled with zeros

extern dterr_t*
dtheaper_alloc_and_zero(int32_t length, const char* why, void** payload);

// ----------------------------------------------------------------
// return a malloc'ed buffer, with length stored in extra space allocated at the
// front

extern dterr_t*
dtheaper_alloc(int32_t length, const char* why, void** buffer);

// ----------------------------------------------------------------
// fill the alloced'ed buffer to value using length stored at front
extern dterr_t*
dtheaper_fill(void* buffer, uint8_t value);

// ----------------------------------------------------------------
// free the alloced'ed buffer, first zeroing the length at front
extern void
dtheaper_free(void* buffer);

#if MARKDOWN_DOCUMENTATION
// clang-format off
// --8<-- [start:markdown-documentation]

# dtheaper

Heap allocation with embedded length and validation.

This group of functions provides heap allocation with tracked length for raw buffers.
It is used inside other modules when they need explicit ownership and size validation of allocations.
It serves to centralize allocation, clearing, and release behind a small API.

Each alloc/free pair updates a ledger channel named "dtheaper" to track current and peak memory usage.

## Mini-guide

- Allocate buffers through `dtheaper_alloc()` to get heap memory for your program to use.
- Use the zeroing allocator when callers require cleared memory by performing fill at allocation.
- Free buffers only via `dtheaper_free()` to maintain ledger consistency.

## Example

```c
void* buffer = NULL;
dterr_t* err = NULL;

err = dtheaper_alloc_and_zero(128, "example buffer", &buffer);
if (err != NULL)
{
    dtheaper_free(buffer);
    return;
}

err = dtheaper_fill(buffer, 0xAA);
if (err != NULL)
{
    dtheaper_free(buffer);
    return;
}

dtheaper_free(buffer);
```

## Data structures

## Functions

### dtheaper_alloc

Allocates a heap buffer with embedded length metadata and returns the payload pointer.

Params:

> `int32_t length` Number of payload bytes to allocate.  
> `const char* why` Descriptive string used for allocation tracking.  
> `void** payload` Receives the allocated payload pointer.  

Return: `dterr_t*` Error object on failure, or NULL on success.

### dtheaper_alloc_and_zero

Allocates a heap buffer and initializes its contents to zero.

Params:

> `int32_t length` Number of payload bytes to allocate.  
> `const char* why` Descriptive string used for allocation tracking.  
> `void** payload` Receives the allocated payload pointer.  

Return: `dterr_t*` Error object on failure, or NULL on success.

### dtheaper_fill

Fills an allocated buffer using the stored length metadata.

Params:

> `void* buffer` Payload pointer previously returned by the allocator.  
> `uint8_t value` Byte value to fill the buffer with.

Return: `dterr_t*` Error object if the buffer header is invalid, or NULL on success.

### dtheaper_free

Releases an allocated buffer after clearing its metadata.

Params:

> `void* buffer` Payload pointer previously returned by the allocator.  

Return: `void`  No return value.

<!-- FG_IDC: fc944936-27de-47d8-8094-65f879685f92 | FG_UTC: 2026-01-17T13:00:45Z FG_MAN=yes -->

// --8<-- [end:markdown-documentation]
// clang-format on
#endif
