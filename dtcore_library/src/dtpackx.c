#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dtpackx.h>
#include <dtcore/dtstr.h>

/* Internal helpers: explicit little-endian encode/decode.
   These are file-local and do not change the public API. */

static inline void
wr_le16(uint8_t* p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xFFu);
    p[1] = (uint8_t)((v >> 8) & 0xFFu);
}

static inline uint16_t
rd_le16(const uint8_t* p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static inline void
wr_le32(uint8_t* p, uint32_t v)
{
    p[0] = (uint8_t)(v & 0xFFu);
    p[1] = (uint8_t)((v >> 8) & 0xFFu);
    p[2] = (uint8_t)((v >> 16) & 0xFFu);
    p[3] = (uint8_t)((v >> 24) & 0xFFu);
}

static inline uint32_t
rd_le32(const uint8_t* p)
{
    return (uint32_t)((uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24));
}

static inline void
wr_le64(uint8_t* p, uint64_t v)
{
    p[0] = (uint8_t)(v & 0xFFu);
    p[1] = (uint8_t)((v >> 8) & 0xFFu);
    p[2] = (uint8_t)((v >> 16) & 0xFFu);
    p[3] = (uint8_t)((v >> 24) & 0xFFu);
    p[4] = (uint8_t)((v >> 32) & 0xFFu);
    p[5] = (uint8_t)((v >> 40) & 0xFFu);
    p[6] = (uint8_t)((v >> 48) & 0xFFu);
    p[7] = (uint8_t)((v >> 56) & 0xFFu);
}

static inline uint64_t
rd_le64(const uint8_t* p)
{
    return (uint64_t)((uint64_t)p[0] | ((uint64_t)p[1] << 8) | ((uint64_t)p[2] << 16) | ((uint64_t)p[3] << 24) |
                      ((uint64_t)p[4] << 32) | ((uint64_t)p[5] << 40) | ((uint64_t)p[6] << 48) | ((uint64_t)p[7] << 56));
}

/* Bit-cast helpers for floats/doubles (assumes IEEE-754). */
static inline uint32_t
f32_to_u32(float f)
{
    uint32_t u;
    memcpy(&u, &f, sizeof u);
    return u;
}
static inline float
u32_to_f32(uint32_t u)
{
    float f;
    memcpy(&f, &u, sizeof f);
    return f;
}
static inline uint64_t
f64_to_u64(double d)
{
    uint64_t u;
    memcpy(&u, &d, sizeof u);
    return u;
}
static inline double
u64_to_f64(uint64_t u)
{
    double d;
    memcpy(&d, &u, sizeof d);
    return d;
}

/* Returns -1 on error (including bounds/NULL), otherwise number of bytes written/read.
   Each function now accepts (offset, buflen) and verifies offset+size <= buflen. */

/* ------------------------------------------------------------------------ */
int32_t
dtpackx_pack_int32(int32_t input, uint8_t* output, int32_t offset, int32_t buflen)
{
    if (!output || offset < 0 || buflen < 0)
        return -1;
    if ((int64_t)offset + (int64_t)sizeof(int32_t) > (int64_t)buflen)
        return -1;
    wr_le32(output + offset, (uint32_t)input);
    return (int32_t)sizeof(int32_t);
}

/* ------------------------------------------------------------------------ */
int32_t
dtpackx_unpack_int32(const uint8_t* input, int32_t offset, int32_t buflen, int32_t* value)
{
    if (!input || !value || offset < 0 || buflen < 0)
        return -1;
    if ((int64_t)offset + (int64_t)sizeof(int32_t) > (int64_t)buflen)
        return -1;
    *value = (int32_t)rd_le32(input + offset);
    return (int32_t)sizeof(int32_t);
}

/* ------------------------------------------------------------------------ */
int32_t
dtpackx_pack_int16(int16_t input, uint8_t* output, int32_t offset, int32_t buflen)
{
    if (!output || offset < 0 || buflen < 0)
        return -1;
    if ((int64_t)offset + (int64_t)sizeof(int16_t) > (int64_t)buflen)
        return -1;
    wr_le16(output + offset, (uint16_t)input);
    return (int32_t)sizeof(int16_t);
}

/* ------------------------------------------------------------------------ */
int32_t
dtpackx_unpack_int16(const uint8_t* input, int32_t offset, int32_t buflen, int16_t* value)
{
    if (!input || !value || offset < 0 || buflen < 0)
        return -1;
    if ((int64_t)offset + (int64_t)sizeof(int16_t) > (int64_t)buflen)
        return -1;
    *value = (int16_t)rd_le16(input + offset);
    return (int32_t)sizeof(int16_t);
}

/* ------------------------------------------------------------------------ */
int32_t
dtpackx_pack_int64(int64_t input, uint8_t* output, int32_t offset, int32_t buflen)
{
    if (!output || offset < 0 || buflen < 0)
        return -1;
    if ((int64_t)offset + (int64_t)sizeof(int64_t) > (int64_t)buflen)
        return -1;
    wr_le64(output + offset, (uint64_t)input);
    return (int32_t)sizeof(int64_t);
}

/* ------------------------------------------------------------------------ */
int32_t
dtpackx_unpack_int64(const uint8_t* input, int32_t offset, int32_t buflen, int64_t* value)
{
    if (!input || !value || offset < 0 || buflen < 0)
        return -1;
    if ((int64_t)offset + (int64_t)sizeof(int64_t) > (int64_t)buflen)
        return -1;
    *value = (int64_t)rd_le64(input + offset);
    return (int32_t)sizeof(int64_t);
}

/* ------------------------------------------------------------------------ */
int32_t
dtpackx_pack_bool(bool input, uint8_t* output, int32_t offset, int32_t buflen)
{
    if (!output || offset < 0 || buflen < 0)
        return -1;
    if ((int64_t)offset + 1 > (int64_t)buflen)
        return -1;
    output[offset] = input ? 1u : 0u;
    return 1;
}

/* ------------------------------------------------------------------------ */
int32_t
dtpackx_unpack_bool(const uint8_t* input, int32_t offset, int32_t buflen, bool* value)
{
    if (!input || !value || offset < 0 || buflen < 0)
        return -1;
    if ((int64_t)offset + 1 > (int64_t)buflen)
        return -1;
    *value = (input[offset] != 0u);
    return 1;
}

/* ------------------------------------------------------------------------ */
int32_t
dtpackx_pack_double(double input, uint8_t* output, int32_t offset, int32_t buflen)
{
    if (!output || offset < 0 || buflen < 0)
        return -1;
    if ((int64_t)offset + (int64_t)sizeof(double) > (int64_t)buflen)
        return -1;
    uint64_t bits = f64_to_u64(input);
    wr_le64(output + offset, bits);
    return (int32_t)sizeof(double);
}

/* ------------------------------------------------------------------------ */
int32_t
dtpackx_unpack_double(const uint8_t* input, int32_t offset, int32_t buflen, double* value)
{
    if (!input || !value || offset < 0 || buflen < 0)
        return -1;
    if ((int64_t)offset + (int64_t)sizeof(double) > (int64_t)buflen)
        return -1;
    uint64_t bits = rd_le64(input + offset);
    *value = u64_to_f64(bits);
    return (int32_t)sizeof(double);
}

/* ------------------------------------------------------------------------ */
int32_t
dtpackx_pack_float(float input, uint8_t* output, int32_t offset, int32_t buflen)
{
    if (!output || offset < 0 || buflen < 0)
        return -1;
    if ((int64_t)offset + (int64_t)sizeof(float) > (int64_t)buflen)
        return -1;
    uint32_t bits = f32_to_u32(input);
    wr_le32(output + offset, bits);
    return (int32_t)sizeof(float);
}

/* ------------------------------------------------------------------------ */
int32_t
dtpackx_unpack_float(const uint8_t* input, int32_t offset, int32_t buflen, float* value)
{
    if (!input || !value || offset < 0 || buflen < 0)
        return -1;
    if ((int64_t)offset + (int64_t)sizeof(float) > (int64_t)buflen)
        return -1;
    uint32_t bits = rd_le32(input + offset);
    *value = u32_to_f32(bits);
    return (int32_t)sizeof(float);
}

/* ------------------------------------------------------------------------ */
/* Layout: [count:int32 LE][count x float32 LE] */
int32_t
dtpackx_pack_float_array(const float* input, int32_t count, uint8_t* output, int32_t offset, int32_t buflen)
{
    if (!input || !output || offset < 0 || buflen < 0 || count < 0)
        return -1;

    int64_t remaining = (int64_t)buflen - (int64_t)offset;
    if (remaining < 0)
        return -1;

    /* total bytes = 4 (count) + 4*count (data) */
    int64_t total_needed = 4 + 4 * (int64_t)count;
    if (total_needed < 0 || total_needed > remaining)
        return -1; /* overflow or not enough space */

    /* write count */
    wr_le32(output + offset, (uint32_t)count);

    /* write each float as LE32 */
    int32_t o = offset + 4;
    for (int32_t i = 0; i < count; ++i)
    {
        uint32_t bits = f32_to_u32(input[i]);
        wr_le32(output + o, bits);
        o += 4;
    }
    return (int32_t)total_needed;
}

/* ------------------------------------------------------------------------ */
int32_t
dtpackx_unpack_float_array(const uint8_t* input, int32_t offset, int32_t buflen, float** value)
{
    if (!input || !value || offset < 0 || buflen < 0)
        return -1;

    *value = NULL;

    int64_t remaining = (int64_t)buflen - (int64_t)offset;
    if (remaining < 4)
        return -1;

    /* read count */
    int32_t count = (int32_t)rd_le32(input + offset);
    if (count < 0)
        return -1;

    int64_t total_needed = 4 + 4 * (int64_t)count;
    if (total_needed < 0 || total_needed > remaining)
        return -1;

    float* arr = NULL;
    if (count > 0)
    {
        arr = (float*)malloc((size_t)count * sizeof(float));
        if (!arr)
            return -1;
    }

    int32_t o = offset + 4;
    for (int32_t i = 0; i < count; ++i)
    {
        uint32_t bits = rd_le32(input + o);
        arr[i] = u32_to_f32(bits);
        o += 4;
    }

    *value = arr;
    return (int32_t)total_needed;
}

/* ------------------------------------------------------------------------ */
int32_t
dtpackx_pack_byte(uint8_t input, uint8_t* output, int32_t offset, int32_t buflen)
{
    if (!output || offset < 0 || buflen < 0)
        return -1;
    if ((int64_t)offset + 1 > (int64_t)buflen)
        return -1;
    output[offset] = input;
    return 1;
}

/* ------------------------------------------------------------------------ */
int32_t
dtpackx_unpack_byte(const uint8_t* input, int32_t offset, int32_t buflen, uint8_t* value)
{
    if (!input || !value || offset < 0 || buflen < 0)
        return -1;
    if ((int64_t)offset + 1 > (int64_t)buflen)
        return -1;
    *value = input[offset];
    return 1;
}

/* ------------------------------------------------------------------------ */
/* Stores bytes up to and including the NUL. If input == NULL, writes a single 0. */
int32_t
dtpackx_pack_string(const char* input, uint8_t* output, int32_t offset, int32_t buflen)
{
    if (!output || offset < 0 || buflen < 0)
        return -1;

    if (input == NULL)
    {
        if (offset + 1 > buflen)
            return -1;
        output[offset] = 0;
        return 1;
    }

    int32_t len = (int32_t)(strlen(input) + 1); /* include NUL */
    if (offset + len > buflen)
        return -1;

    memcpy(output + offset, input, len);
    return len;
}

/* ------------------------------------------------------------------------ */
int32_t
dtpackx_unpack_string(const uint8_t* input, int32_t offset, int32_t buflen, char** value)
{
    if (!input || !value || offset < 0 || buflen < 0)
        return -1;

    int32_t remaining = buflen - offset;
    if (remaining <= 0)
        return -2;

    const char* start = (const char*)(input + offset);
    /* Search for NUL within remaining bytes */
    const void* nul = memchr(start, 0, (size_t)remaining);
    if (!nul)
        return -3; /* no terminator within buffer */

    int32_t len = (const uint8_t*)nul - (const uint8_t*)start + 1; /* include NUL */

    char* copy = dtstr_dup(start);
    if (!copy)
    {
        *value = NULL;
        return -4; /* allocation failure */
    }

    *value = copy;
    return len;
}

/* ------------------------------------------------------------------------ */
/* Length helpers (unchanged semantics; no buflen parameter) */
int32_t
dtpackx_pack_int32_length(void)
{
    return (int32_t)sizeof(int32_t);
}

/* ------------------------------------------------------------------------ */
int32_t
dtpackx_pack_int16_length(void)
{
    return (int32_t)sizeof(int16_t);
}

/* ------------------------------------------------------------------------ */
int32_t
dtpackx_pack_int64_length(void)
{
    return (int32_t)sizeof(int64_t);
}

/* ------------------------------------------------------------------------ */
int32_t
dtpackx_pack_bool_length(void)
{
    return 1;
}

/* ------------------------------------------------------------------------ */
int32_t
dtpackx_pack_double_length(void)
{
    return (int32_t)sizeof(double);
}

/* ------------------------------------------------------------------------ */
int32_t
dtpackx_pack_float_length(void)
{
    return (int32_t)sizeof(float);
}

/* ------------------------------------------------------------------------ */
int32_t
dtpackx_pack_byte_length(void)
{
    return (int32_t)sizeof(uint8_t);
}

/* ------------------------------------------------------------------------ */
int32_t
dtpackx_pack_string_length(const char* input)
{
    return (input == NULL) ? 1 : (int32_t)(strlen(input) + 1);
}
