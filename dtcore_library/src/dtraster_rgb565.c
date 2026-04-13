#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <dtcore/dtcore_constants.h>

#include <dtcore/dtbuffer.h>
#include <dtcore/dterr.h>
#include <dtcore/dtheaper.h>
#include <dtcore/dtlog.h>
#include <dtcore/dtobject.h>
#include <dtcore/dtpackable.h>
#include <dtcore/dtraster.h>
#include <dtcore/dtraster_rgb565.h>
#include <dtcore/dtrgb565.h>
#include <dtcore/dtrgba8888.h>

DTRASTER_INIT_VTABLE(dtraster_rgb565);
DTOBJECT_INIT_VTABLE(dtraster_rgb565);
DTPACKABLE_INIT_VTABLE(dtraster_rgb565);

#define TAG "dtraster_rgb565"

// the implementation
typedef struct dtraster_rgb565_t
{
    DTRASTER_COMMON_MEMBERS;
    dtbuffer_t* buffer;
    dtraster_rgb565_config_t config;
    bool is_malloced;
    bool is_buffer_owned;
} dtraster_rgb565_t;

// --------------------------------------------------------------------------------------------
dterr_t*
dtraster_rgb565_create(dtraster_rgb565_t** self)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);

    DTERR_C(dtheaper_alloc_and_zero(sizeof(dtraster_rgb565_t), "dtraster_rgb565_t", (void**)self));

    DTERR_C(dtraster_rgb565_init(*self));

    (*self)->is_malloced = true;

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
static bool vtables_are_registered = false;

dterr_t*
dtraster_rgb565_register_vtables(void)
{
    dterr_t* dterr = NULL;

    if (!vtables_are_registered)
    {
        int32_t model_number = DTCORE_CONSTANTS_RASTER_MODEL_RGB565;

        DTERR_C(dtraster_set_vtable(model_number, &dtraster_rgb565_raster_vt));
        DTERR_C(dtobject_set_vtable(model_number, &dtraster_rgb565_object_vt));
        DTERR_C(dtpackable_set_vtable(model_number, &dtraster_rgb565_packable_vt));

        vtables_are_registered = true;
    }

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtraster_rgb565_init(dtraster_rgb565_t* self)
{
    dterr_t* dterr = NULL;

    memset(self, 0, sizeof(*self));
    self->model_number = DTCORE_CONSTANTS_RASTER_MODEL_RGB565;

    // ensure vtables are registered for this model number
    DTERR_C(dtraster_rgb565_register_vtables());

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtraster_rgb565_config(dtraster_rgb565_t* self, dtraster_rgb565_config_t* configuration)
{
    dterr_t* dterr = NULL;

    self->config = *configuration;

    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtraster_rgb565_blit(dtraster_rgb565_t* self DTRASTER_BLIT_ARGS)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);
    DTERR_ASSERT_NOT_NULL(that_handle);
    DTERR_ASSERT_NOT_NULL(self->buffer);

    dtraster_rgb565_t* that = (dtraster_rgb565_t*)that_handle;
    DTERR_ASSERT_NOT_NULL(that->buffer);

    int32_t dst_w = (int32_t)self->config.w;
    int32_t dst_h = (int32_t)self->config.h;
    int32_t src_w = (int32_t)that->config.w;
    int32_t src_h = (int32_t)that->config.h;

    int32_t src_x = 0;
    int32_t src_y = 0;
    int32_t dst_x = x;
    int32_t dst_y = y;

    /* Clip left/top */
    if (dst_x < 0)
    {
        src_x = -dst_x;
        dst_x = 0;
    }

    if (dst_y < 0)
    {
        src_y = -dst_y;
        dst_y = 0;
    }

    /* Compute copy size after clipping */
    int32_t copy_w = src_w - src_x;
    int32_t copy_h = src_h - src_y;

    /* Clip right/bottom */
    if (dst_x + copy_w > dst_w)
    {
        copy_w = dst_w - dst_x;
    }

    if (dst_y + copy_h > dst_h)
    {
        copy_h = dst_h - dst_y;
    }

    /* Nothing visible */
    if (copy_w <= 0 || copy_h <= 0)
    {
        goto cleanup;
    }

    uint8_t* dst_origin = (uint8_t*)self->buffer->payload;
    uint8_t* src_origin = (uint8_t*)that->buffer->payload;
    for (int32_t row = 0; row < copy_h; row++)
    {
        uint8_t* dst_row = dst_origin + (size_t)(dst_y + row) * self->config.stride_bytes + (size_t)dst_x * 2;

        uint8_t* src_row = src_origin + (size_t)(src_y + row) * that->config.stride_bytes + (size_t)src_x * 2;

        memcpy(dst_row, src_row, (size_t)copy_w * 2); /* 2 bytes per pixel */
    }

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
// create a raster of the same type as the backing raster
// this is used by clients who want to create a raster compatible with the display for blitting
dterr_t*
dtraster_rgb565_create_compatible_raster(dtraster_rgb565_t* self DTRASTER_CREATE_COMPATIBLE_RASTER_ARGS)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);
    DTERR_ASSERT_NOT_NULL(out_raster_handle);

    dtraster_rgb565_t* o;
    DTERR_C(dtraster_rgb565_create(&o));
    *out_raster_handle = (dtraster_handle)o;
    dtraster_rgb565_config_t c = {
        .w = w,
        .h = h,
        .stride_bytes = w * sizeof(dtrgb565_t),
    };

    DTERR_C(dtraster_rgb565_config(o, &c));

cleanup:

    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtraster_rgb565_use_buffer(dtraster_rgb565_t* self DTRASTER_USE_BUFFER_ARGS)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);
    DTERR_ASSERT_NOT_NULL(buffer);

    if (buffer->length < (size_t)self->config.h * self->config.stride_bytes)
    {
        dterr = dterr_new(DTERR_BADARG, DTERR_LOC, NULL, "buffer length is too small for the raster dimensions and stride");
        goto cleanup;
    }

    self->buffer = buffer;

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtraster_rgb565_new_buffer(dtraster_rgb565_t* self DTRASTER_NEW_BUFFER_ARGS)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);

    int32_t required_length = (int32_t)self->config.h * self->config.stride_bytes;
    DTERR_C(dtbuffer_create(&self->buffer, required_length));
    self->is_buffer_owned = true;

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtraster_rgb565_get_shape(dtraster_rgb565_t* self DTRASTER_GET_SHAPE_ARGS)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);
    DTERR_ASSERT_NOT_NULL(shape);

    shape->w = self->config.w;
    shape->h = self->config.h;
    shape->stride_bytes = self->config.stride_bytes;

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtraster_rgb565_fetch_pixel(dtraster_rgb565_t* self DTRASTER_FETCH_PIXEL_ARGS)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);
    DTERR_ASSERT_NOT_NULL(rgba8888);
    DTERR_ASSERT_NOT_NULL(self->buffer);

    if (x < 0 || x >= self->config.w || y < 0 || y >= self->config.h)
    {
        dterr = dterr_new(DTERR_BADARG, DTERR_LOC, NULL, "pixel coordinates [%" PRId32 ", %" PRId32 "] out of bounds", x, y);
        goto cleanup;
    }

    uint8_t* pixel_addr = (uint8_t*)self->buffer->payload + (size_t)y * self->config.stride_bytes + (size_t)x * 2;

    dtrgb565_t rgb565;
    rgb565.value = (uint16_t)(pixel_addr[0] | ((uint16_t)pixel_addr[1] << 8));

    *rgba8888 = dtrgb565_to_rgba8888(rgb565);

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtraster_rgb565_store_pixel(dtraster_rgb565_t* self DTRASTER_STORE_PIXEL_ARGS)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);
    DTERR_ASSERT_NOT_NULL(self->buffer);

    if (x < 0 || x >= self->config.w || y < 0 || y >= self->config.h)
    {
        dterr = dterr_new(DTERR_BADARG, DTERR_LOC, NULL, "pixel coordinates [%" PRId32 ", %" PRId32 "] out of bounds", x, y);
        goto cleanup;
    }

    dtrgb565_t rgb565 = dtrgb565_from_rgba8888(rgba8888);

    uint8_t* pixel_addr = (uint8_t*)self->buffer->payload + (size_t)y * self->config.stride_bytes + (size_t)x * 2;

    if (self->config.should_store_as_big_endian)
    {
        pixel_addr[0] = (uint8_t)((rgb565.value >> 8) & 0xFF);
        pixel_addr[1] = (uint8_t)(rgb565.value & 0xFF);
    }
    else
    {
        pixel_addr[0] = (uint8_t)(rgb565.value & 0xFF);
        pixel_addr[1] = (uint8_t)((rgb565.value >> 8) & 0xFF);
    }

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
// Returns packed length
dterr_t*
dtraster_rgb565_packx_length(dtraster_rgb565_t* self DTPACKABLE_PACKX_LENGTH_ARGS)
{
    (void)self;
    *length = 5 * dtpackx_pack_int32_length();
    return NULL;
}

// --------------------------------------------------------------------------------------------
// Packs the rgb565 raster values into the output buffer
dterr_t*
dtraster_rgb565_packx(dtraster_rgb565_t* self DTPACKABLE_PACKX_ARGS)
{
    dterr_t* dterr = NULL;
    int32_t p = offset ? *offset : 0;
    int32_t n = 0;

    n = dtpackx_pack_int32(self->model_number, output, p, length);
    if (n < 0)
        goto cleanup;
    p += n;

    n = dtpackx_pack_int16(self->config.w, output, p, length);
    if (n < 0)
        goto cleanup;
    p += n;

    n = dtpackx_pack_int16(self->config.h, output, p, length);
    if (n < 0)
        goto cleanup;
    p += n;

    n = dtpackx_pack_int16(self->config.stride_bytes, output, p, length);
    if (n < 0)
        goto cleanup;
    p += n;

    if (offset)
        *offset = p;

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
// Unpacks the rgb565 raster values from the input buffer
dterr_t*
dtraster_rgb565_unpackx(dtraster_rgb565_t* self DTPACKABLE_UNPACKX_ARGS)
{
    dterr_t* dterr = NULL;
    int32_t p = offset ? *offset : 0;
    int32_t n = 0;

    n = dtpackx_unpack_int32(input, p, length, &self->model_number);
    if (n < 0)
        goto cleanup;
    p += n;

    n = dtpackx_unpack_int16(input, p, length, (int16_t*)&self->config.w);
    if (n < 0)
        goto cleanup;
    p += n;

    n = dtpackx_unpack_int16(input, p, length, (int16_t*)&self->config.h);
    if (n < 0)
        goto cleanup;
    p += n;

    n = dtpackx_unpack_int16(input, p, length, (int16_t*)&self->config.stride_bytes);
    if (n < 0)
        goto cleanup;
    p += n;

    if (offset)
        *offset = p;

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
// Validates the device configuration
dterr_t*
dtraster_rgb565_validate_unpacked(dtraster_rgb565_t* self)
{
    dterr_t* dterr = NULL;

    if (self->model_number != DTCORE_CONSTANTS_RASTER_MODEL_RGB565)
    {
        dterr = dterr_new(DTERR_BADCONFIG,
          DTERR_LOC,
          NULL,
          "model_number was %d but must be "
          "DTCORE_CONSTANTS_RASTER_MODEL_RGB565 (%d)",
          self->model_number,
          DTCORE_CONSTANTS_RASTER_MODEL_RGB565);
        goto cleanup;
    }

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
// dtobject implementation
// --------------------------------------------------------------------------------------------
void
dtraster_rgb565_copy(dtraster_rgb565_t* self, dtraster_rgb565_t* that_handle)
{
    self->config = that_handle->config;
    self->model_number = that_handle->model_number;
}

// --------------------------------------------------------------------------------------------
void
dtraster_rgb565_dispose(dtraster_rgb565_t* self)
{
    if (self == NULL)
        return;

    if (self->is_buffer_owned && self->buffer != NULL)
        dtbuffer_dispose(self->buffer);

    bool is_malloced = self->is_malloced;

    memset(self, 0, sizeof(*self));
    if (is_malloced)
        dtheaper_free(self);
}

// --------------------------------------------------------------------------------------------
// Equality check
bool
dtraster_rgb565_equals(dtraster_rgb565_t* a, dtraster_rgb565_t* b)
{
    if (a == NULL || b == NULL)
    {
        return false;
    }
    bool equals = a->model_number == b->model_number &&             //
                  a->config.w == b->config.w &&                     //
                  a->config.h == b->config.h &&                     //
                  a->config.stride_bytes == b->config.stride_bytes; //

    if (equals)
    {
        if (a->buffer == NULL || b->buffer == NULL)
        {
            equals = a->buffer == b->buffer; // both must be null to be equal
        }
    }

    if (equals && a->buffer != NULL && b->buffer != NULL)
    {
        for (int y = 0; y < a->config.h; y++)
        {
            for (int x = 0; x < a->config.w; x++)
            {
                dtrgba8888_t pixel_a;
                dtrgba8888_t pixel_b;
                dtraster_rgb565_fetch_pixel(a, x, y, &pixel_a);
                dtraster_rgb565_fetch_pixel(b, x, y, &pixel_b);
                equals = dtrgba8888_compare(pixel_a, pixel_b) == 0;
                if (!equals)
                    break;
            }
            if (!equals)
                break;
        }
    }

    return equals;
}

// --------------------------------------------------------------------------------------------
const char*
dtraster_rgb565_get_class(dtraster_rgb565_t* self)
{
    (void)self;
    return "dtraster_rgb565_t";
}

// --------------------------------------------------------------------------------------------
bool
dtraster_rgb565_is_iface(dtraster_rgb565_t* self, const char* iface_name)
{
    (void)self;
    return strcmp(iface_name, "dtraster_iface") == 0 ||   //
           strcmp(iface_name, "dtpackable_iface") == 0 || //
           strcmp(iface_name, "dtobject_iface") == 0;
}

// --------------------------------------------------------------------------------------------
// Converts the raster values to a string (for debugging)
void
dtraster_rgb565_to_string(dtraster_rgb565_t* self, char* buffer, size_t buffer_size)
{
    snprintf(buffer,
      buffer_size,
      "%s, [%" PRId32 "x%" PRId32 "], stride_bytes=%" PRId32 ", big_endian=%s",
      dtraster_rgb565_get_class(self),
      self->config.w,
      self->config.h,
      self->config.stride_bytes,
      self->config.should_store_as_big_endian ? "true" : "false");
}