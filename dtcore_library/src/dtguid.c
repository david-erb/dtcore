#include <stdatomic.h> // atomic_fetch_add
#include <stdint.h>
#include <stdio.h>  // snprintf
#include <string.h> // memcpy, memcmp, memset

#include <dtcore/dtguid.h>

// Global sequential counter (64 bits)
static uint32_t dtguid_counter = 1;

// ----------------------------------------------------------------------------
void
dtguid_init(dtguid_t* guid)
{
    dtguid_zero(guid);
}

// ----------------------------------------------------------------------------
void
dtguid_zero(dtguid_t* guid)
{
    memset(guid->bytes, 0, sizeof(guid->bytes));
}

// ----------------------------------------------------------------------------
bool
dtguid_is_zero(const dtguid_t* guid)
{
    static const uint8_t zero_bytes[16] = { 0 };
    return (memcmp(guid->bytes, zero_bytes, 16) == 0);
}

// ----------------------------------------------------------------------------
void
dtguid_copy(dtguid_t* dest, const dtguid_t* src)
{
    memcpy(dest->bytes, src->bytes, sizeof(dest->bytes));
}

// ----------------------------------------------------------------------------
int
dtguid_cmp(const dtguid_t* guid1, const dtguid_t* guid2)
{
    return memcmp(guid1->bytes, guid2->bytes, sizeof(guid1->bytes));
}

// ----------------------------------------------------------------------------
bool
dtguid_is_equal(const dtguid_t* guid1, const dtguid_t* guid2)
{
    return memcmp(guid1->bytes, guid2->bytes, 16) == 0;
}

// ----------------------------------------------------------------------------
void
dtguid_generate_sequential(dtguid_t* guid)
{
    uint32_t count = atomic_fetch_add(&dtguid_counter, 1);

    // Zero out the entire GUID
    memset(guid->bytes, 0, sizeof(guid->bytes));

    // Copy the 64-bit counter into the first 8 bytes
    memcpy(&guid->bytes[0], &count, sizeof(count));
}

// ----------------------------------------------------------------------------
static void
md5(const uint8_t* input, size_t length, uint8_t output[16]);

void
dtguid_generate_from_input(dtguid_t* guid, const uint8_t* input, size_t length)
{
    uint8_t hash[16];
    md5(input, length, hash);

    // RFC 4122 compliance
    hash[6] = (hash[6] & 0x0F) | 0x30; // Version 3
    hash[8] = (hash[8] & 0x3F) | 0x80; // Variant

    // Force 'DAE0' prefix (0xDAE = 110110101110b)
    hash[4] = 0xDA;
    hash[5] = 0xE0;

    // Copy the hash into the GUID
    memcpy(guid->bytes, hash, sizeof(guid->bytes));
}

// ----------------------------------------------------------------------------
void
dtguid_generate_from_string(dtguid_t* guid, const char* input)
{
    dtguid_generate_from_input(guid, (const uint8_t*)input, strlen(input));
}

// ----------------------------------------------------------------------------
void
dtguid_generate_from_int32(dtguid_t* guid, int32_t input)
{

    dtguid_generate_from_input(guid, (const uint8_t*)&input, sizeof(input));
}

// ----------------------------------------------------------------------------
void
dtguid_generate_from_int64(dtguid_t* guid, int64_t input)
{

    dtguid_generate_from_input(guid, (const uint8_t*)&input, sizeof(input));
}

// ----------------------------------------------------------------------------
void
dtguid_to_string(const dtguid_t* guid, char* buffer, size_t buffer_size)
{
    if (buffer_size < DTGUID_STRING_SIZE)
    {
        if (buffer_size > 0)
            buffer[0] = '\0'; // empty string if too small
        return;
    }

    snprintf(buffer,
      buffer_size,
      "%02x%02x%02x%02x-"
      "%02x%02x-"
      "%02x%02x-"
      "%02x%02x-"
      "%02x%02x%02x%02x%02x%02x",
      guid->bytes[0],
      guid->bytes[1],
      guid->bytes[2],
      guid->bytes[3],
      guid->bytes[4],
      guid->bytes[5],
      guid->bytes[6],
      guid->bytes[7],
      guid->bytes[8],
      guid->bytes[9],
      guid->bytes[10],
      guid->bytes[11],
      guid->bytes[12],
      guid->bytes[13],
      guid->bytes[14],
      guid->bytes[15]);
}

// ----------------------------------------------------------------------------
void
dtguid_to_string_short(const dtguid_t* guid, char* buffer, size_t buffer_size)
{
    if (buffer_size < DTGUID_STRING_SHORT_SIZE)
    {
        if (buffer_size > 0)
            buffer[0] = '\0'; // empty string if too small
        return;
    }

    snprintf(buffer, buffer_size, "%02x%02x-%02x%02x", guid->bytes[0], guid->bytes[1], guid->bytes[2], guid->bytes[3]);
}

// ----------------------------------------------------------------------------
void
dtguid_to_string_tiny(const dtguid_t* guid, char* buffer, size_t buffer_size)
{
    if (buffer_size < DTGUID_STRING_TINY_SIZE)
    {
        if (buffer_size > 0)
            buffer[0] = '\0'; // empty string if too small
        return;
    }

    snprintf(buffer, buffer_size, "%02x%02x", guid->bytes[0], guid->bytes[1]);
}

// ----------------------------------------------------------------------------
// Packing/Unpacking functions for serialization
int32_t
dtguid_pack_length(void)
{
    // A GUID is 16 bytes
    return 16;
}

int32_t
dtguid_pack(const dtguid_t* guid, uint8_t* output, int32_t offset, int32_t length)
{
    if (offset + 16 > length)
        return 0; // Not enough space

    memcpy(output + offset, guid->bytes, 16);
    return 16;
}

int32_t
dtguid_unpack(dtguid_t* guid, const uint8_t* input, int32_t offset, int32_t length)
{
    if (offset + 16 > length)
        return 0; // Not enough data

    memcpy(guid->bytes, input + offset, 16);
    return 16;
}

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Begin public domain MD5 implementation (RFC 1321) ---
/*
 * MD5 implementation based on the RSA Data Security, Inc. MD5 Message-Digest Algorithm
 * as described in RFC 1321.
 * This code is in the public domain.
 */

typedef struct
{
    uint32_t h[4];
    uint32_t data[16];
    uint64_t bitlen;
} md5_ctx;

static void
md5_transform(md5_ctx* ctx, const uint8_t data[])
{
    uint32_t a = ctx->h[0], b = ctx->h[1], c = ctx->h[2], d = ctx->h[3], f, g, temp;
    // clang-format off
    static const uint32_t k[] = { 
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
        0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
        0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
        0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
        0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
        0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
        0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
        0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
     };
    static const uint32_t r[] = { 
        7,12,17,22,7,12,17,22,7,12,17,22,7,12,17,22,
        5,9,14,20,5,9,14,20,5,9,14,20,5,9,14,20,
        4,11,16,23,4,11,16,23,4,11,16,23,4,11,16,23,
        6,10,15,21,6,10,15,21,6,10,15,21,6,10,15,21
     };
    // clang-format on

    uint32_t w[16];
    for (int i = 0; i < 16; i++)
    {
        w[i] = ((uint32_t)data[i * 4]) | (((uint32_t)data[i * 4 + 1]) << 8) | (((uint32_t)data[i * 4 + 2]) << 16) |
               (((uint32_t)data[i * 4 + 3]) << 24);
    }

    for (int i = 0; i < 64; i++)
    {
        if (i < 16)
        {
            f = (b & c) | (~b & d);
            g = i;
        }
        else if (i < 32)
        {
            f = (d & b) | (~d & c);
            g = (5 * i + 1) % 16;
        }
        else if (i < 48)
        {
            f = b ^ c ^ d;
            g = (3 * i + 5) % 16;
        }
        else
        {
            f = c ^ (b | ~d);
            g = (7 * i) % 16;
        }

        temp = d;
        d = c;
        c = b;
        b = b + ((a + f + k[i] + w[g]) << r[i] | (a + f + k[i] + w[g]) >> (32 - r[i]));
        a = temp;
    }

    ctx->h[0] += a;
    ctx->h[1] += b;
    ctx->h[2] += c;
    ctx->h[3] += d;
}

static void
md5_init(md5_ctx* ctx)
{
    ctx->bitlen = 0;
    ctx->h[0] = 0x67452301;
    ctx->h[1] = 0xefcdab89;
    ctx->h[2] = 0x98badcfe;
    ctx->h[3] = 0x10325476;
}

static void
md5_update(md5_ctx* ctx, const uint8_t* data, size_t len)
{
    size_t i;

    for (i = 0; i < len; i++)
    {
        ((uint8_t*)ctx->data)[ctx->bitlen / 8 % 64] = data[i];
        ctx->bitlen += 8;

        if ((ctx->bitlen / 8) % 64 == 0)
        {
            md5_transform(ctx, (uint8_t*)ctx->data);
        }
    }
}

static void
md5_final(md5_ctx* ctx, uint8_t hash[16])
{
    size_t index = (ctx->bitlen / 8) % 64;
    ((uint8_t*)ctx->data)[index++] = 0x80;

    while (index != 56)
    {
        if (index == 64)
        {
            md5_transform(ctx, (uint8_t*)ctx->data);
            index = 0;
        }
        ((uint8_t*)ctx->data)[index++] = 0x00;
    }

    uint64_t bits = ctx->bitlen;
    for (int i = 0; i < 8; i++)
    {
        ((uint8_t*)ctx->data)[56 + i] = bits >> (8 * i);
    }

    md5_transform(ctx, (uint8_t*)ctx->data);

    for (int i = 0; i < 4; i++)
    {
        hash[i * 4 + 0] = ctx->h[i] & 0xFF;
        hash[i * 4 + 1] = (ctx->h[i] >> 8) & 0xFF;
        hash[i * 4 + 2] = (ctx->h[i] >> 16) & 0xFF;
        hash[i * 4 + 3] = (ctx->h[i] >> 24) & 0xFF;
    }
}

static void
md5(const uint8_t* data, size_t len, uint8_t hash[16])
{
    md5_ctx ctx;
    md5_init(&ctx);
    md5_update(&ctx, data, len);
    md5_final(&ctx, hash);
}
// --- End public domain MD5 ---
