/*
 * dtrgba8888 -- 32-bit RGBA color type with inline constructors and named constants.
 *
 * Stores a color as four separate 8-bit R, G, B, and A bytes with an inline
 * constructor and a macro initializer.  Includes a comparison function and
 * a full set of named color constants matching the RGB565 palette.
 *
 * cdox v1.0.2
 */
#pragma once

// --------------------------------------------------------------------------------------
// dtrgba8888.h
//
// Simple RGBA color structure for raster operations
// --------------------------------------------------------------------------------------

#include <stdint.h>

// --------------------------------------------------------------------------------------
// Color structure
// --------------------------------------------------------------------------------------

typedef struct dtrgba8888_t
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;

} dtrgba8888_t;

// --------------------------------------------------------------------------------------
// Inline constructor
// --------------------------------------------------------------------------------------

static inline dtrgba8888_t
dtrgba8888(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    dtrgba8888_t c;
    c.r = r;
    c.g = g;
    c.b = b;
    c.a = a;
    return c;
}

// --------------------------------------------------------------------------------------
// Macro constructor
// --------------------------------------------------------------------------------------

#define DTRGBA8888(R, G, B, A) ((dtrgba8888_t){ (R), (G), (B), (A) })

// --------------------------------------------------------------------------------------
static inline int
dtrgba8888_compare(dtrgba8888_t a, dtrgba8888_t b)
{
    if (a.r != b.r)
        return a.r - b.r;
    if (a.g != b.g)
        return a.g - b.g;
    if (a.b != b.b)
        return a.b - b.b;
    if (a.a != b.a)
        return a.a - b.a;
    return 0;
}

// --------------------------------------------------------------------------------------
// Named color constants
//
// These are exact inverses of the corresponding DTRGB565_* packed constants,
// using RGB565 5/6/5 -> 8-bit expansion.
// --------------------------------------------------------------------------------------

// clang-format off
#define DTRGBA8888_TRANSPARENT ((dtrgba8888_t){   0,   0,   0, 0x00 })
#define DTRGBA8888_BLACK       ((dtrgba8888_t){   0,   0,   0, 0xFF })
#define DTRGBA8888_WHITE       ((dtrgba8888_t){ 255, 255, 255, 0xFF })
#define DTRGBA8888_RED         ((dtrgba8888_t){ 255,   0,   0, 0xFF })
#define DTRGBA8888_LIME        ((dtrgba8888_t){   0, 255,   0, 0xFF })
#define DTRGBA8888_BLUE        ((dtrgba8888_t){   0,   0, 255, 0xFF })
#define DTRGBA8888_YELLOW      ((dtrgba8888_t){ 255, 255,   0, 0xFF })
#define DTRGBA8888_CYAN        ((dtrgba8888_t){   0, 255, 255, 0xFF })
#define DTRGBA8888_MAGENTA     ((dtrgba8888_t){ 255,   0, 255, 0xFF })
#define DTRGBA8888_SILVER      ((dtrgba8888_t){ 198, 195, 198, 0xFF })
#define DTRGBA8888_GRAY        ((dtrgba8888_t){ 132, 130, 132, 0xFF })
#define DTRGBA8888_MAROON      ((dtrgba8888_t){ 132,   0,   0, 0xFF })
#define DTRGBA8888_OLIVE       ((dtrgba8888_t){ 132, 130,   0, 0xFF })
#define DTRGBA8888_GREEN       ((dtrgba8888_t){   0, 255,   0, 0xFF })
#define DTRGBA8888_PURPLE      ((dtrgba8888_t){ 132,   0, 132, 0xFF })
#define DTRGBA8888_TEAL        ((dtrgba8888_t){   0, 130, 132, 0xFF })
#define DTRGBA8888_NAVY        ((dtrgba8888_t){   0,   0, 132, 0xFF })
#define DTRGBA8888_ORANGE      ((dtrgba8888_t){ 255, 166,   0, 0xFF })
#define DTRGBA8888_PINK        ((dtrgba8888_t){ 255,   0, 255, 0xFF })
#define DTRGBA8888_BROWN       ((dtrgba8888_t){ 165,  40,  41, 0xFF })
#define DTRGBA8888_GOLD        ((dtrgba8888_t){ 255, 215,   0, 0xFF })
#define DTRGBA8888_BEIGE       ((dtrgba8888_t){ 247, 247, 222, 0xFF })
#define DTRGBA8888_IVORY       ((dtrgba8888_t){ 255, 255, 247, 0xFF })
#define DTRGBA8888_TURQUOISE   ((dtrgba8888_t){  66, 227, 214, 0xFF })
#define DTRGBA8888_VIOLET      ((dtrgba8888_t){ 239, 130, 239, 0xFF })
#define DTRGBA8888_SKYBLUE     ((dtrgba8888_t){ 132, 207, 239, 0xFF })
#define DTRGBA8888_DARKGREEN   ((dtrgba8888_t){   0, 101,   0, 0xFF })
#define DTRGBA8888_DARKRED     ((dtrgba8888_t){ 140,   0,   0, 0xFF })
#define DTRGBA8888_DARKBLUE    ((dtrgba8888_t){   0,   0, 140, 0xFF })
#define DTRGBA8888_LIGHTGRAY   ((dtrgba8888_t){ 214, 211, 214, 0xFF })
#define DTRGBA8888_DARKGRAY    ((dtrgba8888_t){  82,  85,  82, 0xFF })
// clang-format on