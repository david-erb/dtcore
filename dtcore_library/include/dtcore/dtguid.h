/*
 * dtguid -- 16-byte identifier with deterministic generation and serialization.
 *
 * Provides comparison, copy, zero-check, and three levels of text formatting
 * for 128-bit identifiers.  Generation supports sequential process-local
 * counters, hash-derived values from byte inputs or strings, and integer
 * seeds.  Pack and unpack functions integrate with the dtpackx wire format.
 *
 * cdox v1.0.2
 */
#pragma once
// See markdown documentation at the end of this file.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DTGUID_STRING_SIZE 37
#define DTGUID_STRING_SHORT_SIZE 10
#define DTGUID_STRING_TINY_SIZE 5

typedef struct
{
  uint8_t bytes[16];
} dtguid_t;

extern void
dtguid_init(dtguid_t* guid);
extern int
dtguid_cmp(const dtguid_t* guid1, const dtguid_t* guid2);
extern bool
dtguid_is_equal(const dtguid_t* guid1, const dtguid_t* guid2);
extern bool
dtguid_is_zero(const dtguid_t* guid);
extern void
dtguid_zero(dtguid_t* guid);
extern void
dtguid_copy(dtguid_t* dest, const dtguid_t* src);
extern void
dtguid_generate_sequential(dtguid_t* guid);
extern void
dtguid_generate_from_input(dtguid_t* guid, const uint8_t* input, size_t length);
extern void
dtguid_generate_from_string(dtguid_t* guid, const char* input);
extern void
dtguid_generate_from_int32(dtguid_t* guid, int32_t input);
extern void
dtguid_generate_from_int64(dtguid_t* guid, int64_t input);
extern void
dtguid_to_string(const dtguid_t* guid, char* buffer, size_t buffer_size);
extern void
dtguid_to_string_short(const dtguid_t* guid, char* buffer, size_t buffer_size);
extern void
dtguid_to_string_tiny(const dtguid_t* guid, char* buffer, size_t buffer_size);

// Packing/Unpacking functions for serialization
extern int32_t
dtguid_pack_length(void);
extern int32_t
dtguid_pack(const dtguid_t* guid,
            uint8_t* output,
            int32_t offset,
            int32_t length);
extern int32_t
dtguid_unpack(dtguid_t* guid,
              const uint8_t* input,
              int32_t offset,
              int32_t length);

#if MARKDOWN_DOCUMENTATION
// clang-format off
// --8<-- [start:markdown-documentation]

# dtguid

Small GUID values with deterministic generation and formatting.

This group of functions provides 16-byte GUID utilities for local identifiers and serialization. 
It is intended for modules that need stable values for lookup keys, logs, and wire formats.
It is able to generate values from counters or inputs and convert them to text or bytes.

## Mini-guide

- Use `dtguid_init` or `dtguid_zero` to set all bytes to zero by requiring a valid `dtguid_t*`.
- Use `dtguid_generate_sequential` when local uniqueness is sufficient by generating from a process-local counter.
- Use `dtguid_generate_from_input` or `dtguid_generate_from_string` when a stable mapping is required by hashing the input bytes.

## Example

```c
#include <dtcore/dtguid.h>
#include <stdio.h>

void demo(void)
{
    dtguid_t a;
    dtguid_t b;
    dtguid_t c;

    dtguid_generate_sequential(&a);
    dtguid_generate_from_string(&b, "sensor-42");

    char sa[DTGUID_STRING_SIZE];
    char sb[DTGUID_STRING_SIZE];
    dtguid_to_string(&a, sa, sizeof sa);
    dtguid_to_string(&b, sb, sizeof sb);

    printf("a=%s\n", sa);
    printf("b=%s\n", sb);

    uint8_t buf[32];
    int32_t off = 0;
    off += dtguid_pack(&a, buf, off, (int32_t)sizeof buf);

    dtguid_zero(&c);
    (void)dtguid_unpack(&c, buf, 0, off);
}
```

## Data structures

### dtguid_t

Fixed-size 16-byte identifier value.

Members:

> `uint8_t bytes[16]` Raw identifier bytes.  

## Macros

### DTGUID_STRING_SHORT_SIZE

Size of the short formatted string buffer including the NUL terminator.

### DTGUID_STRING_SIZE

Size of the full formatted string buffer including the NUL terminator.

### DTGUID_STRING_TINY_SIZE

Size of the tiny formatted string buffer including the NUL terminator.

## Functions

### dtguid_cmp

Compares two identifiers using byte order over 16 bytes.

Params:

> `const dtguid_t* guid1` First identifier to compare.  
> `const dtguid_t* guid2` Second identifier to compare.  

Return: `int` Negative, zero, or positive comparison result.  

### dtguid_copy

Copies 16 bytes from one identifier to another.

Params:

> `dtguid_t* dest` Destination identifier to write.  
> `const dtguid_t* src` Source identifier to read.  

Return: `void`  No return value.  

### dtguid_generate_from_input

Generates a deterministic identifier from an input byte stream.

Params:

> `dtguid_t* guid` Destination identifier to write.  
> `const uint8_t* input` Input bytes to hash.  
> `size_t length` Number of bytes to hash.  

Return: `void`  No return value.  

### dtguid_generate_from_int32

Generates a deterministic identifier from a 32-bit integer value.

Params:

> `dtguid_t* guid` Destination identifier to write.  
> `int32_t input` Integer value to hash.  

Return: `void`  No return value.  

### dtguid_generate_from_int64

Generates a deterministic identifier from a 64-bit integer value.

Params:

> `dtguid_t* guid` Destination identifier to write.  
> `int64_t input` Integer value to hash.  

Return: `void`  No return value.  

### dtguid_generate_from_string

Generates a deterministic identifier from a NUL-terminated string.

Params:

> `dtguid_t* guid` Destination identifier to write.  
> `const char* input` Input string bytes to hash.  

Return: `void`  No return value.  

### dtguid_generate_sequential

Generates a sequential identifier using a process-local counter.

Params:

> `dtguid_t* guid` Destination identifier to write.  

Return: `void`  No return value.  

### dtguid_init

Initializes an identifier value to all zeros.

Params:

> `dtguid_t* guid` Identifier to initialize.  

Return: `void`  No return value.  

### dtguid_is_equal

Checks whether two identifiers have identical bytes.

Params:

> `const dtguid_t* guid1` First identifier to compare.  
> `const dtguid_t* guid2` Second identifier to compare.  

Return: `bool` True when all 16 bytes match.  

### dtguid_is_zero

Checks whether an identifier is all zeros.

Params:

> `const dtguid_t* guid` Identifier to test.  

Return: `bool` True when all 16 bytes are zero.  

### dtguid_pack

Serializes an identifier into a byte buffer at an offset.

Params:

> `const dtguid_t* guid` Identifier to serialize.  
> `uint8_t* output` Output buffer to write.  
> `int32_t offset` Byte offset to start writing at.  
> `int32_t length` Total available bytes in the buffer.  

Return: `int32_t` Bytes written on success, or zero on insufficient space.  

### dtguid_pack_length

Returns the serialized length in bytes.

Params:

Param: `void`  No parameters.  

Return: `int32_t` Serialized length in bytes.  

### dtguid_to_string

Formats an identifier as a full hyphenated hex string.

Params:

> `const dtguid_t* guid` Identifier to format.  
> `char* buffer` Output string buffer to write.  
> `size_t buffer_size` Size of `buffer` in bytes.  

Return: `void`  No return value.  

### dtguid_to_string_short

Formats an identifier as a short hex string.

Params:

> `const dtguid_t* guid` Identifier to format.  
> `char* buffer` Output string buffer to write.  
> `size_t buffer_size` Size of `buffer` in bytes.  

Return: `void`  No return value.  

### dtguid_to_string_tiny

Formats an identifier as a tiny hex string.

Params:

> `const dtguid_t* guid` Identifier to format.  
> `char* buffer` Output string buffer to write.  
> `size_t buffer_size` Size of `buffer` in bytes.  

Return: `void`  No return value.  

### dtguid_unpack

Deserializes an identifier from a byte buffer at an offset.

Params:

> `dtguid_t* guid` Destination identifier to write.  
> `const uint8_t* input` Input buffer to read.  
> `int32_t offset` Byte offset to start reading at.  
> `int32_t length` Total available bytes in the buffer.  

Return: `int32_t` Bytes read on success, or zero on insufficient data.  

### dtguid_zero

Sets an identifier value to all zeros.

Params:

> `dtguid_t* guid` Identifier to clear.  

Return: `void`  No return value.  

<!-- FG_IDC: bcdb849e-b3f0-46c0-96ca-7a171d844ceb | FG_UTC: 2026-01-17T14:51:38Z | FG_MAN=yes -->

// --8<-- [end:markdown-documentation]
// clang-format on
#endif
