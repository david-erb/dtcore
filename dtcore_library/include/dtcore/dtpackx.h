/*
 * dtpackx -- Little-endian binary pack and unpack helpers for C types.
 *
 * Provides length-query, pack, and unpack functions for integers, floats,
 * booleans, bytes, float arrays, and NUL-terminated strings using a
 * consistent little-endian wire format.  Return values are bytes consumed
 * or written; negative indicates overflow.  Macros wrap calls with automatic
 * bounds checking and error jumps.
 *
 * cdox v1.0.2
 */
#pragma once

// See markdown documentation at the end of this file.

// Portable little-endian pack/unpack helpers for basic C types.

// Wire format: integers are serialized in little-endian;
// floats / doubles* are serialized as IEEE - 754 bit patterns in little-endian.

#include <stdbool.h>
#include <stdint.h>

// Note: Little-endian on the wire.
int32_t
dtpackx_pack_int32(int32_t input,
                   uint8_t* output,
                   int32_t offset,
                   int32_t buflen);

int32_t
dtpackx_unpack_int32(const uint8_t* input,
                     int32_t offset,
                     int32_t buflen,
                     int32_t* value);

int32_t
dtpackx_pack_int32_length(void);

int32_t
dtpackx_pack_int16(int16_t input,
                   uint8_t* output,
                   int32_t offset,
                   int32_t buflen);

int32_t
dtpackx_unpack_int16(const uint8_t* input,
                     int32_t offset,
                     int32_t buflen,
                     int16_t* value);

int32_t
dtpackx_pack_int16_length(void);

int32_t
dtpackx_pack_int64(int64_t input,
                   uint8_t* output,
                   int32_t offset,
                   int32_t buflen);

int32_t
dtpackx_unpack_int64(const uint8_t* input,
                     int32_t offset,
                     int32_t buflen,
                     int64_t* value);

int32_t
dtpackx_pack_int64_length(void);

int32_t
dtpackx_pack_bool(bool input, uint8_t* output, int32_t offset, int32_t buflen);

int32_t
dtpackx_unpack_bool(const uint8_t* input,
                    int32_t offset,
                    int32_t buflen,
                    bool* value);

int32_t
dtpackx_pack_bool_length(void);

// Note: Little-endian bit pattern on the wire (assumes IEEE-754).
int32_t
dtpackx_pack_double(double input,
                    uint8_t* output,
                    int32_t offset,
                    int32_t buflen);

int32_t
dtpackx_unpack_double(const uint8_t* input,
                      int32_t offset,
                      int32_t buflen,
                      double* value);

int32_t
dtpackx_pack_double_length(void);

// Note: Little-endian bit pattern on the wire (assumes IEEE-754).
int32_t
dtpackx_pack_float(float input,
                   uint8_t* output,
                   int32_t offset,
                   int32_t buflen);

int32_t
dtpackx_unpack_float(const uint8_t* input,
                     int32_t offset,
                     int32_t buflen,
                     float* value);

int32_t
dtpackx_pack_float_length(void);

// Layout is `[int32 count][count × float32]`. Count is stored as little-endian
// int32; each element is stored as an IEEE-754 float32 bit pattern in
// little-endian.
int32_t
dtpackx_pack_float_array(const float* input,
                         int32_t count,
                         uint8_t* output,
                         int32_t offset,
                         int32_t buflen);

// Reads an int32 element count followed by that many float32 values. On
// success,
// `*value` points to a heap allocation the caller must `free()`.
int32_t
dtpackx_unpack_float_array(const uint8_t* input,
                           int32_t offset,
                           int32_t buflen,
                           float** value);

int32_t
dtpackx_pack_byte(uint8_t input,
                  uint8_t* output,
                  int32_t offset,
                  int32_t buflen);

int32_t
dtpackx_unpack_byte(const uint8_t* input,
                    int32_t offset,
                    int32_t buflen,
                    uint8_t* value);

int32_t
dtpackx_pack_byte_length(void);

int32_t
dtpackx_pack_string(const char* input,
                    uint8_t* output,
                    int32_t offset,
                    int32_t buflen);

// Scans for the first NUL within the available bytes `buflen - offset`. On
// success,
// `*value` receives a heap-allocated duplicate the caller must
// `free((void*)*value)`.
int32_t
dtpackx_unpack_string(const uint8_t* input,
                      int32_t offset,
                      int32_t buflen,
                      char** value);

int32_t
dtpackx_pack_string_length(const char* input);

// ------------------------------------------------------------------------
// helper macro for packing with error handling when overflow occurs
#define DTPACKX_PACK(call, p, length)                                          \
  do {                                                                         \
    int32_t l = call;                                                          \
    if (l < 0) {                                                               \
      dterr =                                                                  \
        dterr_new(DTERR_OVERFLOW,                                              \
                  DTERR_LOC,                                                   \
                  NULL,                                                        \
                  "pack failed near position %" PRId32 " of length %" PRId32,  \
                  p,                                                           \
                  length);                                                     \
      goto cleanup;                                                            \
    }                                                                          \
    p += l;                                                                    \
  } while (0)
// ------------------------------------------------------------------------
// helper macro for unpacking with error handling when overflow occurs
#define DTPACKX_UNPACK(call, p, length)                                        \
  do {                                                                         \
    int32_t l = call;                                                          \
    if (l < 0) {                                                               \
      dterr = dterr_new(DTERR_OVERFLOW,                                        \
                        DTERR_LOC,                                             \
                        NULL,                                                  \
                        "unpack failed near position %" PRId32                 \
                        " of length %" PRId32,                                 \
                        p,                                                     \
                        length);                                               \
      goto cleanup;                                                            \
    }                                                                          \
    p += l;                                                                    \
  } while (0)

#if MARKDOWN_DOCUMENTATION
// clang-format off
// --8<-- [start:markdown-documentation]

# dtpackx

Packing helpers for wire buffers

This group of functions provides little-endian packing and unpacking for basic C types. 
Use it for serializing and deserializing byte-oriented wire buffers with explicit bounds checking. 

## Mini-guide

- Use the pack functions to append values to a buffer by tracking an explicit offset.  
- Use the unpack functions to read values in the same order by reusing the returned length.  
- Check for negative return values to detect bounds or allocation failures early.  
- Pre-compute space requirements with the length helpers before allocating buffers.  

## Example

```c
int32_t offset = 0;
uint8_t buffer[64];

offset += dtpackx_pack_int32(42, buffer, offset, sizeof buffer);
offset += dtpackx_pack_bool(true, buffer, offset, sizeof buffer);

int32_t value = 0;
bool flag = false;
int32_t read = 0;

read += dtpackx_unpack_int32(buffer, read, sizeof buffer, &value);
read += dtpackx_unpack_bool(buffer, read, sizeof buffer, &flag);
```

## Data structures

## Macros

### DTPACKX_PACK

`DTPACKX_PACK(call, p, length)`

Wraps a packing call and advances the position while converting overflow into an error jump.

### DTPACKX_UNPACK

`DTPACKX_UNPACK(call, p, length)`

Wraps an unpacking call and advances the position while converting overflow into an error jump.

## Functions

### dtpackx_pack_bool

Writes a boolean value as a single byte in little-endian form.

Params:

> `bool input` Value to pack.  
> `uint8_t* output` Destination buffer.  
> `int32_t offset` Start position in the buffer.  
> `int32_t buflen` Total buffer length.  

Return: `int32_t` Number of bytes written or a negative value on error.

### dtpackx_pack_bool_length

Returns the number of bytes required to pack a boolean value.

Params:

> `void`  No parameters.  

Return: `int32_t` Size in bytes of the packed value.

### dtpackx_pack_byte

Writes an unsigned byte value to the buffer.

Params:

> `uint8_t input` Value to pack.  
> `uint8_t* output` Destination buffer.  
> `int32_t offset` Start position in the buffer.  
> `int32_t buflen` Total buffer length.  

Return: `int32_t` Number of bytes written or a negative value on error.

### dtpackx_pack_byte_length

Returns the number of bytes required to pack a byte value.

Params:

> `void`  No parameters.  

Return: `int32_t` Size in bytes of the packed value.

### dtpackx_pack_double

Writes a double value as an IEEE-754 bit pattern in little-endian order.

Params:

> `double input` Value to pack.  
> `uint8_t* output` Destination buffer.  
> `int32_t offset` Start position in the buffer.  
> `int32_t buflen` Total buffer length.  

Return: `int32_t` Number of bytes written or a negative value on error.

### dtpackx_pack_double_length

Returns the number of bytes required to pack a double value.

Params:

> `void`  No parameters.  

Return: `int32_t` Size in bytes of the packed value.

### dtpackx_pack_float

Writes a float value as an IEEE-754 bit pattern in little-endian order.

Params:

> `float input` Value to pack.  
> `uint8_t* output` Destination buffer.  
> `int32_t offset` Start position in the buffer.  
> `int32_t buflen` Total buffer length.  

Return: `int32_t` Number of bytes written or a negative value on error.

### dtpackx_pack_float_array

Writes a counted array of float values using a fixed little-endian layout.

Params:

> `const float* input` Source array.  
> `int32_t count` Number of elements to write.  
> `uint8_t* output` Destination buffer.  
> `int32_t offset` Start position in the buffer.  
> `int32_t buflen` Total buffer length.  

Return: `int32_t` Number of bytes written or a negative value on error.

### dtpackx_pack_float_length

Returns the number of bytes required to pack a float value.

Params:

> `void`  No parameters.  

Return: `int32_t` Size in bytes of the packed value.

### dtpackx_pack_int16

Writes a 16-bit integer value in little-endian order.

Params:

> `int16_t input` Value to pack.  
> `uint8_t* output` Destination buffer.  
> `int32_t offset` Start position in the buffer.  
> `int32_t buflen` Total buffer length.  

Return: `int32_t` Number of bytes written or a negative value on error.

### dtpackx_pack_int16_length

Returns the number of bytes required to pack a 16-bit integer value.

Params:

> `void`  No parameters.  

Return: `int32_t` Size in bytes of the packed value.

### dtpackx_pack_int32

Writes a 32-bit integer value in little-endian order.

Params:

> `int32_t input` Value to pack.  
> `uint8_t* output` Destination buffer.  
> `int32_t offset` Start position in the buffer.  
> `int32_t buflen` Total buffer length.  

Return: `int32_t` Number of bytes written or a negative value on error.

### dtpackx_pack_int32_length

Returns the number of bytes required to pack a 32-bit integer value.

Params:

> `void`  No parameters.  

Return: `int32_t` Size in bytes of the packed value.

### dtpackx_pack_int64

Writes a 64-bit integer value in little-endian order.

Params:

> `int64_t input` Value to pack.  
> `uint8_t* output` Destination buffer.  
> `int32_t offset` Start position in the buffer.  
> `int32_t buflen` Total buffer length.  

Return: `int32_t` Number of bytes written or a negative value on error.

### dtpackx_pack_int64_length

Returns the number of bytes required to pack a 64-bit integer value.

Params:

> `void`  No parameters.  

Return: `int32_t` Size in bytes of the packed value.

### dtpackx_pack_string

Writes a NUL-terminated string including the terminator.

Params:

> `const char* input` String to pack or NULL.  
> `uint8_t* output` Destination buffer.  
> `int32_t offset` Start position in the buffer.  
> `int32_t buflen` Total buffer length.  

Return: `int32_t` Number of bytes written or a negative value on error.

### dtpackx_pack_string_length

Returns the number of bytes required to pack a string including its terminator.

Params:

> `const char* input` String to measure or NULL.  

Return: `int32_t` Size in bytes of the packed value.

### dtpackx_unpack_bool

Reads a boolean value from a single byte.

Params:

> `const uint8_t* input` Source buffer.  
> `int32_t offset` Start position in the buffer.  
> `int32_t buflen` Total buffer length.  
> `bool* value` Destination for the unpacked value.  

Return: `int32_t` Number of bytes read or a negative value on error.

### dtpackx_unpack_byte

Reads an unsigned byte value from the buffer.

Params:

> `const uint8_t* input` Source buffer.  
> `int32_t offset` Start position in the buffer.  
> `int32_t buflen` Total buffer length.  
> `uint8_t* value` Destination for the unpacked value.  

Return: `int32_t` Number of bytes read or a negative value on error.

### dtpackx_unpack_double

Reads a double value from an IEEE-754 little-endian bit pattern.

Params:

> `const uint8_t* input` Source buffer.  
> `int32_t offset` Start position in the buffer.  
> `int32_t buflen` Total buffer length.  
> `double* value` Destination for the unpacked value.  

Return: `int32_t` Number of bytes read or a negative value on error.

### dtpackx_unpack_float

Reads a float value from an IEEE-754 little-endian bit pattern.

Params:

> `const uint8_t* input` Source buffer.  
> `int32_t offset` Start position in the buffer.  
> `int32_t buflen` Total buffer length.  
> `float* value` Destination for the unpacked value.  

Return: `int32_t` Number of bytes read or a negative value on error.

### dtpackx_unpack_float_array

Reads a counted array of float values and allocates storage for the result.

Params:

> `const uint8_t* input` Source buffer.  
> `int32_t offset` Start position in the buffer.  
> `int32_t buflen` Total buffer length.  
> `float** value` Receives a heap-allocated array on success.  

Return: `int32_t` Number of bytes read or a negative value on error.

### dtpackx_unpack_int16

Reads a 16-bit integer value from little-endian storage.

Params:

> `const uint8_t* input` Source buffer.  
> `int32_t offset` Start position in the buffer.  
> `int32_t buflen` Total buffer length.  
> `int16_t* value` Destination for the unpacked value.  

Return: `int32_t` Number of bytes read or a negative value on error.

### dtpackx_unpack_int32

Reads a 32-bit integer value from little-endian storage.

Params:

> `const uint8_t* input` Source buffer.  
> `int32_t offset` Start position in the buffer.  
> `int32_t buflen` Total buffer length.  
> `int32_t* value` Destination for the unpacked value.  

Return: `int32_t` Number of bytes read or a negative value on error.

### dtpackx_unpack_int64

Reads a 64-bit integer value from little-endian storage.

Params:

> `const uint8_t* input` Source buffer.  
> `int32_t offset` Start position in the buffer.  
> `int32_t buflen` Total buffer length.  
> `int64_t* value` Destination for the unpacked value.  

Return: `int32_t` Number of bytes read or a negative value on error.

### dtpackx_unpack_string

Reads a NUL-terminated string and duplicates it to heap storage.

Params:

> `const uint8_t* input` Source buffer.  
> `int32_t offset` Start position in the buffer.  
> `int32_t buflen` Total buffer length.  
> `char** value` Receives a heap-allocated duplicate on success.  

Return: `int32_t` Number of bytes read or a negative value on error.

<!-- FG_IDC: 9f128597-9c29-4e5c-a8e8-bc13809ea240 | FG_UTC: 2026-01-18T09:23:34Z | FG_MAN=yes -->

// --8<-- [end:markdown-documentation]
// clang-format on
#endif
