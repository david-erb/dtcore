/*
 * dtarray_int32 -- Fixed-capacity int32 array with binary serialization.
 *
 * Allocates instance and element storage as a single contiguous block for
 * a fixed element count.  Provides pack and unpack helpers that serialize
 * the count followed by each element as little-endian int32 values, with
 * graceful handling of element count mismatches during deserialization.
 *
 * cdox v1.0.2
 */
#pragma once

// See markdown documentation at the end of this file.

// Fixed-size int32 array with simple serialization helpers.

#include <dtcore/dterr.h>
#include <dtcore/dtpackx.h>

//
// The instance and its elements are allocated as one contiguous block by
// dtarray_int32_create(). The `count` is the fixed capacity (and logical
// length) for pack/unpack operations.
typedef struct
{
  int32_t count; ///< Total element count / capacity for `items`. Must be > 0
                 ///< after create.

  // Pointer to the first element; points into the same allocation as the
  // struct. Not individually heap-owned; do **not** free this pointer.
  int32_t* items;
} dtarray_int32_t;

//
// Allocates one contiguous block that stores the header followed by `count`
// elements and initializes `items` to that block. On success, `*array_int32`
// is set to the new object; on failure, it is not modified.
//
//
// Warning: The returned object owns its storage. Dispose it with
//          dtarray_int32_dispose(); do not free `items` directly.
extern dterr_t*
dtarray_int32_create(dtarray_int32_t** array_int32, int32_t count);

//
// Safe to call with `NULL`.
//
extern void
dtarray_int32_dispose(dtarray_int32_t* self);

// Helpers for computing lengths and serializing to/from byte buffers.

//
// Encodes as `[count:int32]` followed by `count` elements as `int32`.
//
//
// Note: Use this to size your buffer before dtarray_int32_pack().
extern int32_t
dtarray_int32_pack_length(const dtarray_int32_t* self);

//
// Layout: `[count:int32][items...]`, each as packed `int32`. Writes starting
// at `output[offset]` and does not write past `buflen`.
//
//
extern int32_t
dtarray_int32_pack(const dtarray_int32_t* self,
                   uint8_t* output,
                   int32_t offset,
                   int32_t buflen);

//
// Expects `[count:int32][items...]`. Advances from `offset` without overrunning
// `buflen`. Behavior on length mismatch:
// - If payload has **more** than `self->count` items, extras are consumed to
//   keep the stream in sync and **discarded**.
// - If payload has **fewer** items, only the provided elements are stored; the
//   remaining entries in `items` are **zeroed**.
//
//
extern int32_t
dtarray_int32_unpack(dtarray_int32_t* self,
                     const uint8_t* input,
                     int32_t offset,
                     int32_t buflen);

#if MARKDOWN_DOCUMENTATION
// clang-format off
// --8<-- [start:markdown-documentation]

# dtarray_int32

Fixed-capacity int32 array with pack and unpack helpers.

This group of functions provides fixed-capacity int32 storage with byte-level serialization.
Use it where array size is known at creation and serialized I/O binary is deterministic.

## Mini-guide

- Creates instances with a fixed element count by allocating one contiguous block.
- Access the `items` pointer directly for element storage.
- Dispose instances when done.
- Serialize by computing pack length before writing to a buffer.
- Deserialize by unpacking into an existing array capacity.

## Example

```c
dtarray_int32_t* arr = NULL;
dterr_t* err = dtarray_int32_create(&arr, 4);
arr->items[0] = 10;
arr->items[1] = 20;

if (!err)
{
    int32_t len = dtarray_int32_pack_length(arr);
    uint8_t buffer[64];

    dtarray_int32_pack(arr, buffer, 0, sizeof(buffer));
    dtarray_int32_unpack(arr, buffer, 0, sizeof(buffer));

    dtarray_int32_dispose(arr);
}
```

## Data structures

### dtarray_int32_t

Represents a fixed-capacity array of int32 values with contiguous storage.

Members:

> `int32_t count` Total element count and logical capacity.  
>
> `int32_t* items` Pointer to the first element within the same allocation.  


## Functions

### dtarray_int32_create

Allocates and initializes a fixed-capacity array instance.

Params:

> `dtarray_int32_t** array_int32` Output pointer that receives the new instance.  
>
> `int32_t count` Number of elements to allocate as fixed capacity.  

Return: `dterr_t*` Error object on failure, or null on success.

### dtarray_int32_dispose

Releases all storage owned by an array instance.

Params:

> `dtarray_int32_t* self` Instance to dispose, or null.  

Return: `void`  No return value.

### dtarray_int32_pack

Serializes the array into a byte buffer starting at a given offset.

Params:

> `const dtarray_int32_t* self` Source array to serialize.  
>
> `uint8_t* output` Destination buffer for packed bytes.  
>
> `int32_t offset` Starting offset into the destination buffer.  
>
> `int32_t buflen` Total size of the destination buffer.  

Return: `int32_t` Number of bytes written, or a negative value on failure.

### dtarray_int32_pack_length

Computes the number of bytes required to serialize the array.

Params:

> `const dtarray_int32_t* self` Source array whose packed length is queried.  

Return: `int32_t` Required byte count, or a negative value on invalid input.

### dtarray_int32_unpack

Deserializes array contents from a byte buffer starting at a given offset.

Params:

> `dtarray_int32_t* self` Destination array with fixed capacity.  
>
> `const uint8_t* input` Source buffer containing packed data.  
>
> `int32_t offset` Starting offset into the source buffer.  
>
> `int32_t buflen` Total size of the source buffer.  

Return: `int32_t` Number of bytes consumed, or a negative value on failure.

<!-- FG_IDC: 9d833db6-7d9d-4e31-8a88-a6e93ad7d77f | FG_UTC: 2026-01-17T09:37:40Z | FG_MAN=yes -->

// --8<-- [end:markdown-documentation]
// clang-format on
#endif
