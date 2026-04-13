/*
 * dtrgb565 -- Packed RGB565 color type with accessors and named constants.
 *
 * Stores a color as a packed 16-bit value with inline constructors from
 * 8-bit RGB components or a raw value.  Component accessors expand 5- and
 * 6-bit fields back to 8-bit values.  Includes round-trip conversion to
 * and from dtrgba8888_t and a set of named color constants.
 *
 * cdox v1.0.2
 */
#pragma once

// --------------------------------------------------------------------------------------
// dtrgb565.h
//
// Packed RGB565 color structure and helpers
// --------------------------------------------------------------------------------------

#include <stdint.h>

#include <dtcore/dtrgba8888.h>

// --------------------------------------------------------------------------------------
// Color structure
// --------------------------------------------------------------------------------------

typedef struct dtrgb565_t
{
    uint16_t value;

} dtrgb565_t;

// --------------------------------------------------------------------------------------
// Internal helpers
// --------------------------------------------------------------------------------------

static inline uint8_t
_dtrgb565_expand5_to_8(uint8_t v)
{
    return (uint8_t)((v << 3) | (v >> 2));
}

// --------------------------------------------------------------------------------------
static inline uint8_t
_dtrgb565_expand6_to_8(uint8_t v)
{
    return (uint8_t)((v << 2) | (v >> 4));
}

// --------------------------------------------------------------------------------------
// Inline constructor from 8-bit RGB
// --------------------------------------------------------------------------------------

static inline dtrgb565_t
dtrgb565(uint8_t r, uint8_t g, uint8_t b)
{
    dtrgb565_t c;
    uint16_t r5 = (uint16_t)(r >> 3);
    uint16_t g6 = (uint16_t)(g >> 2);
    uint16_t b5 = (uint16_t)(b >> 3);

    c.value = (uint16_t)((r5 << 11) | (g6 << 5) | b5);
    return c;
}

// --------------------------------------------------------------------------------------
// Constructor from raw packed value
// --------------------------------------------------------------------------------------

static inline dtrgb565_t
dtrgb565_raw(uint16_t value)
{
    dtrgb565_t c;
    c.value = value;
    return c;
}

// --------------------------------------------------------------------------------------
// Macro constructor from 8-bit RGB
// --------------------------------------------------------------------------------------

#define DTRGB565(R, G, B) dtrgb565((R), (G), (B))

// --------------------------------------------------------------------------------------
// Component accessors
// --------------------------------------------------------------------------------------

static inline uint8_t
dtrgb565_r(dtrgb565_t c)
{
    return _dtrgb565_expand5_to_8((uint8_t)((c.value >> 11) & 0x1F));
}

// --------------------------------------------------------------------------------------
static inline uint8_t
dtrgb565_g(dtrgb565_t c)
{
    return _dtrgb565_expand6_to_8((uint8_t)((c.value >> 5) & 0x3F));
}

// --------------------------------------------------------------------------------------
static inline uint8_t
dtrgb565_b(dtrgb565_t c)
{
    return _dtrgb565_expand5_to_8((uint8_t)(c.value & 0x1F));
}

// --------------------------------------------------------------------------------------
// RGB565 has no alpha channel. Return fully opaque.
// --------------------------------------------------------------------------------------

static inline uint8_t
dtrgb565_a(dtrgb565_t c)
{
    (void)c;
    return 0xFF;
}

// --------------------------------------------------------------------------------------
// Conversion to rgba8888
// --------------------------------------------------------------------------------------

static inline dtrgba8888_t
dtrgb565_to_rgba8888(dtrgb565_t c)
{
    return dtrgba8888(dtrgb565_r(c), dtrgb565_g(c), dtrgb565_b(c), 0xFF);
}

// --------------------------------------------------------------------------------------
// Conversion from rgba8888
//
// Alpha is discarded because RGB565 cannot represent it.
// --------------------------------------------------------------------------------------

static inline dtrgb565_t
dtrgb565_from_rgba8888(dtrgba8888_t c)
{
    return dtrgb565(c.r, c.g, c.b);
}

// --------------------------------------------------------------------------------------
static inline int
dtrgb565_compare(dtrgb565_t a, dtrgb565_t b)
{
    if (a.value < b.value)
        return -1;
    if (a.value > b.value)
        return 1;
    return 0;
}

// --------------------------------------------------------------------------------------
// Named color constants
// --------------------------------------------------------------------------------------

// clang-format off
#define DTRGB565_BLACK         ((dtrgb565_t){ 0x0000 })
#define DTRGB565_WHITE         ((dtrgb565_t){ 0xFFFF })
#define DTRGB565_RED           ((dtrgb565_t){ 0xF800 })
#define DTRGB565_LIME          ((dtrgb565_t){ 0x07E0 })
#define DTRGB565_BLUE          ((dtrgb565_t){ 0x001F })
#define DTRGB565_YELLOW        ((dtrgb565_t){ 0xFFE0 })
#define DTRGB565_CYAN          ((dtrgb565_t){ 0x07FF })
#define DTRGB565_MAGENTA       ((dtrgb565_t){ 0xF81F })
#define DTRGB565_SILVER        ((dtrgb565_t){ 0xC618 })
#define DTRGB565_GRAY          ((dtrgb565_t){ 0x8410 })
#define DTRGB565_MAROON        ((dtrgb565_t){ 0x8000 })
#define DTRGB565_OLIVE         ((dtrgb565_t){ 0x8400 })
#define DTRGB565_GREEN         ((dtrgb565_t){ 0x07E0 })
#define DTRGB565_PURPLE        ((dtrgb565_t){ 0x8010 })
#define DTRGB565_TEAL          ((dtrgb565_t){ 0x0410 })
#define DTRGB565_NAVY          ((dtrgb565_t){ 0x0010 })
#define DTRGB565_ORANGE        ((dtrgb565_t){ 0xFD20 })
#define DTRGB565_PINK          ((dtrgb565_t){ 0xF81F })
#define DTRGB565_BROWN         ((dtrgb565_t){ 0xA145 })
#define DTRGB565_GOLD          ((dtrgb565_t){ 0xFEA0 })
#define DTRGB565_BEIGE         ((dtrgb565_t){ 0xF7BB })
#define DTRGB565_IVORY         ((dtrgb565_t){ 0xFFFE })
#define DTRGB565_TURQUOISE     ((dtrgb565_t){ 0x471A })
#define DTRGB565_VIOLET        ((dtrgb565_t){ 0xEC1D })
#define DTRGB565_SKYBLUE       ((dtrgb565_t){ 0x867D })
#define DTRGB565_DARKGREEN     ((dtrgb565_t){ 0x0320 })
#define DTRGB565_DARKRED       ((dtrgb565_t){ 0x8800 })
#define DTRGB565_DARKBLUE      ((dtrgb565_t){ 0x0011 })
#define DTRGB565_LIGHTGRAY     ((dtrgb565_t){ 0xD69A })
#define DTRGB565_DARKGRAY      ((dtrgb565_t){ 0x52AA })
//clang-format on
