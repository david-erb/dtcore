/*
 * dtarray_float -- Fixed-capacity float array with binary serialization.
 *
 * Provides heap-allocated, fixed-size storage for float values with copy,
 * equality comparison, and pack/unpack serialization to byte buffers.
 * The instance and element storage are allocated as a single contiguous
 * block, keeping allocation and disposal simple.
 *
 * cdox v1.0.2
 */
#pragma once

// See markdown documentation at the end of this file.

#include <stdbool.h>
#include <stdint.h>

#include <dtcore/dterr.h>

typedef struct dtarray_float_t
{
  int32_t count;
  float* items;
} dtarray_float_t;

extern dterr_t*
dtarray_float_create(dtarray_float_t** array_float, int count);

extern void
dtarray_float_dispose(dtarray_float_t* self);

extern void
dtarray_float_copy(const dtarray_float_t* src, dtarray_float_t* dest);
extern bool
dtarray_float_equals(const dtarray_float_t* a, const dtarray_float_t* b);

// Packing/Unpacking functions for serialization

extern int32_t
dtarray_float_pack_length( //
  const dtarray_float_t* self);

extern int32_t
dtarray_float_pack( //
  const dtarray_float_t* self,
  uint8_t* output,
  int32_t offset,
  int32_t length);

extern int32_t
dtarray_float_unpack( //
  dtarray_float_t* self,
  const uint8_t* input,
  int32_t offset,
  int32_t length);

#if MARKDOWN_DOCUMENTATION
// clang-format off
// --8<-- [start:markdown-documentation]

# dtarray_float

Fixed-size float array with packing support

This group of functions provides fixed-size float array management.
It supports copying, comparison, and stream packing.
The implementation has single-allocation ownership and requires explicit disposal.

## Mini-guide

- Create an instance by calling the allocator with a positive element count.
- Access the `items` pointer directly for element storage.
- Dispose the instance when done to free memory.
- Serialize by computing pack length before writing to a buffer.
- Deserialize by unpacking into an existing array capacity.

## Example

```c
dtarray_float_t* a = NULL;
dtarray_float_t* b = NULL;

if (dtarray_float_create(&a, 4) == NULL &&
    dtarray_float_create(&b, 4) == NULL)
{
    a->items[0] = 1.0f;
    a->items[1] = 2.0f;

    dtarray_float_copy(a, b);

    bool same = dtarray_float_equals(a, b);
    (void)same;
}

dtarray_float_dispose(a);
dtarray_float_dispose(b);
```

## Data structures

### dtarray_float_t

Represents a fixed-size array of float elements.

Members:

> `int32_t count` Number of elements in the array.  
> `float* items` Pointer to the element storage.  

## Functions

### dtarray_float_copy

Copies elements from a source array into a destination array.

Params:

> `const dtarray_float_t* src` Source array to read from.  
> `dtarray_float_t* dest` Destination array to write into.  

Return: `void`  No return value.

### dtarray_float_create

Allocates and initializes a float array with a fixed element count.

Params:

> `dtarray_float_t** array_float` Receives the allocated array pointer.  
> `int count` Number of elements to allocate.  

Return: `dterr_t*` Error object on failure, or NULL on success.

### dtarray_float_dispose

Releases memory owned by a float array.

Params:

> `dtarray_float_t* self` Array instance to dispose.  

Return: `void`  No return value.

### dtarray_float_equals

Compares two float arrays for equal size and contents.

Params:

> `const dtarray_float_t* a` First array to compare.  
> `const dtarray_float_t* b` Second array to compare.  

Return: `bool` True if arrays are equal.

### dtarray_float_pack

Serializes the array into a byte buffer.

Params:

> `const dtarray_float_t* self` Array to serialize.  
> `uint8_t* output` Output buffer to write into.  
> `int32_t offset` Starting offset in the buffer.  
> `int32_t length` Total length of the buffer.  

Return: `int32_t` Number of bytes written, or negative on error.

### dtarray_float_pack_length

Computes the number of bytes required to serialize the array.

Params:

> `const dtarray_float_t* self` Array to measure.  

Return: `int32_t` Required byte length, or negative on error.

### dtarray_float_unpack

Deserializes array contents from a byte buffer.

Params:

> `dtarray_float_t* self` Array to populate.  
> `const uint8_t* input` Input buffer to read from.  
> `int32_t offset` Starting offset in the buffer.  
> `int32_t length` Total length of the buffer.  

Return: `int32_t` Number of bytes consumed, or negative on error.

<!-- FG_IDC: b63390c2-6507-43d6-862e-6df0906917c9 | FG_UTC: 2026-01-18T05:27:04Z | FG_MAN=yes -->

// --8<-- [end:markdown-documentation]
// clang-format on
#endif
