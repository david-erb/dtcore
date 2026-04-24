#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dtcore_constants.h>

#include <dtcore/dtbuffer.h>
#include <dtcore/dterr.h>
#include <dtcore/dtlog.h>
#include <dtcore/dtpackable.h>
#include <dtcore/dtpackx.h>
#include <dtcore/dtstr.h>

#include <dtcore/dtkvp.h>

#define TAG "dtkvp"

// ------------------------------------------------------------------------
static dtkvp_t*
dtkvp_list__find_key(dtkvp_list_t* self, const char* key)
{
    for (int32_t i = 0; i < self->count; i++)
    {
        dtkvp_t* kvp = &self->items[i];
        if (kvp->key != NULL && strcmp(kvp->key, key) == 0)
        {
            return kvp;
        }
    }
    return NULL;
}
// ------------------------------------------------------------------------
static dtkvp_t*
dtkvp_list__find_available(dtkvp_list_t* self)
{
    for (int32_t i = 0; i < self->count; i++)
    {
        dtkvp_t* kvp = &self->items[i];
        if (kvp->key == NULL)
        {
            return kvp;
        }
    }
    return NULL;
}

// ------------------------------------------------------------------------
// for url encoding/decoding
static bool
dtkvp_list__is_url_unreserved(char c);
static char
dtkvp_list__hex_digit(uint8_t v);
static int
dtkvp_list__from_hex_char(char c);
static dterr_t*
dtkvp_list__append_char(char** s, int32_t* length, int32_t* capacity, char c);
static dterr_t*
dtkvp_list__append_urlencoded(char** s, int32_t* length, int32_t* capacity, const char* input);
static dterr_t*
dtkvp_list__urldecode_component(const char* input, int32_t input_length, char** out);

// ------------------------------------------------------------------------
// initialize the kvp list, allocating internal storage
extern dterr_t*
dtkvp_list_init(dtkvp_list_t* self)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);

    memset(self, 0, sizeof(*self));

    self->count = 32;
    DTERR_C(dtbuffer_create(&self->storage, self->count * sizeof(dtkvp_t)));
    memset(self->storage->payload, 0, self->storage->length);
    self->items = (dtkvp_t*)self->storage->payload;

cleanup:
    return dterr;
}

// ------------------------------------------------------------------------
// set a key-value pair, overwrites any existing value, if value is NULL,
// removes the key
extern dterr_t*
dtkvp_list_set(dtkvp_list_t* self, const char* key, const char* value)
{
    dterr_t* dterr = NULL;

    DTERR_ASSERT_NOT_NULL(self);
    DTERR_ASSERT_NOT_NULL(self->items);
    DTERR_ASSERT_NOT_NULL(key);

    if (strcmp(key, DTCORE_CONSTANTS_KVP_KEY_WRITE_GUID) == 0)
    {
        dtlog_info(TAG, "setting key %s=%s", key, value ? value : "(null)");
    }

    dtkvp_t* kvp = dtkvp_list__find_key(self, key);

    // found existing key?
    if (kvp != NULL)
    {
        if (kvp->value)
            dtstr_dispose((char*)kvp->value);
        if (value == NULL)
        {
            // remove the key
            dtstr_dispose((char*)kvp->key);
            kvp->key = NULL;
            kvp->value = NULL;
        }
        else
        {
            // update the value
            kvp->value = dtstr_dup(value);
        }
    }
    // did not find existing key, but want to set a new value
    else if (value != NULL)
    {

        kvp = dtkvp_list__find_available(self);

        if (kvp == NULL)
        {
            // need to grow the storage
            int32_t old_count = self->count;
            self->count *= 2;
            dtbuffer_t* old_storage = self->storage;
            DTERR_C(dtbuffer_create(&self->storage, self->count * sizeof(dtkvp_t)));
            memset(self->storage->payload, 0, self->storage->length);
            memcpy(self->storage->payload, self->items, old_count * sizeof(dtkvp_t));
            dtbuffer_dispose(old_storage);
            self->items = (dtkvp_t*)self->storage->payload;
            // we can use the first new item
            kvp = &self->items[old_count];
        }

        kvp->key = dtstr_dup(key);
        kvp->value = dtstr_dup(value);
    }

cleanup:

    return dterr;
}

// ------------------------------------------------------------------------
// get a value by key, returns value as NULL if key is not present
extern dterr_t*
dtkvp_list_get(dtkvp_list_t* self, const char* key, const char** value)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);
    DTERR_ASSERT_NOT_NULL(self->items);
    DTERR_ASSERT_NOT_NULL(key);
    DTERR_ASSERT_NOT_NULL(value);

    dtkvp_t* kvp = dtkvp_list__find_key(self, key);
    if (kvp != NULL)
        *value = kvp->value;
    else
        *value = NULL;

cleanup:
    return dterr;
}

// ------------------------------------------------------------------------
// write kvp list as application/x-www-form-urlencoded into a newly allocated
// output string. caller owns *out_string.
extern dterr_t*
dtkvp_list_urlencode(dtkvp_list_t* self, char** out_string)
{
    dterr_t* dterr = NULL;
    char* s = NULL;
    int32_t length = 0;
    int32_t capacity = 0;
    bool first = true;

    DTERR_ASSERT_NOT_NULL(self);
    DTERR_ASSERT_NOT_NULL(self->items);
    DTERR_ASSERT_NOT_NULL(out_string);

    for (int32_t i = 0; i < self->count; i++)
    {
        dtkvp_t* kvp = &self->items[i];
        if (kvp->key == NULL)
            continue;

        if (!first)
            DTERR_C(dtkvp_list__append_char(&s, &length, &capacity, '&'));

        DTERR_C(dtkvp_list__append_urlencoded(&s, &length, &capacity, kvp->key ? kvp->key : ""));
        DTERR_C(dtkvp_list__append_char(&s, &length, &capacity, '='));
        DTERR_C(dtkvp_list__append_urlencoded(&s, &length, &capacity, kvp->value ? kvp->value : ""));

        first = false;
    }

    if (s == NULL)
    {
        s = dtstr_dup("");
        if (s == NULL)
        {
            dterr = dterr_new(DTERR_STATE, DTERR_LOC, NULL, "failed to allocate empty urlencoded string");
            goto cleanup;
        }
    }

    *out_string = s;
    s = NULL;

cleanup:
    dtstr_dispose(s);
    return dterr;
}

// ------------------------------------------------------------------------
// parse application/x-www-form-urlencoded string and store entries into the
// kvp list using dtkvp_list_set(). existing matching keys are overwritten.
// unrelated existing keys are left untouched.
extern dterr_t*
dtkvp_list_urldecode(dtkvp_list_t* self, const char* in_string)
{
    dterr_t* dterr = NULL;
    const char* p = NULL;

    DTERR_ASSERT_NOT_NULL(self);
    DTERR_ASSERT_NOT_NULL(self->items);
    DTERR_ASSERT_NOT_NULL(in_string);

    p = in_string;

    while (*p != '\0')
    {
        const char* pair_start = p;
        const char* pair_end = strchr(pair_start, '&');
        const char* eq = NULL;
        char* key = NULL;
        char* value = NULL;
        int32_t pair_length = 0;
        int32_t key_length = 0;
        int32_t value_length = 0;

        if (pair_end == NULL)
            pair_end = pair_start + strlen(pair_start);

        pair_length = (int32_t)(pair_end - pair_start);
        eq = memchr(pair_start, '=', (size_t)pair_length);

        if (eq != NULL)
        {
            key_length = (int32_t)(eq - pair_start);
            value_length = (int32_t)(pair_end - eq - 1);
        }
        else
        {
            key_length = pair_length;
            value_length = 0;
        }

        DTERR_C(dtkvp_list__urldecode_component(pair_start, key_length, &key));

        if (eq != NULL)
        {
            DTERR_C(dtkvp_list__urldecode_component(eq + 1, value_length, &value));
        }
        else
        {
            value = dtstr_dup("");
            if (value == NULL)
            {
                dterr = dterr_new(DTERR_STATE, DTERR_LOC, NULL, "failed to allocate decoded value");
                goto cleanup;
            }
        }

        DTERR_C(dtkvp_list_set(self, key, value));

        dtstr_dispose(key);
        key = NULL;
        dtstr_dispose(value);
        value = NULL;

        if (*pair_end == '\0')
            break;

        p = pair_end + 1;
        continue;

    cleanup:
        dtstr_dispose(key);
        dtstr_dispose(value);
        return dterr;
    }

    return NULL;
}

// ------------------------------------------------------------------------
// get length needed to pack the kvp list
extern dterr_t*
dtkvp_list_packx_length(dtkvp_list_t* self DTPACKABLE_PACKX_LENGTH_ARGS)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);
    DTERR_ASSERT_NOT_NULL(self->items);
    DTERR_ASSERT_NOT_NULL(length);

    int32_t len = 0;

    // magic number
    len += dtpackx_pack_int32_length();
    // count of actual kvps
    len += dtpackx_pack_int32_length();
    for (int32_t i = 0; i < self->count; i++)
    {
        dtkvp_t* kvp = &self->items[i];
        if (kvp->key == NULL)
            continue;
        len += dtpackx_pack_string_length(kvp->key);
        len += dtpackx_pack_string_length(kvp->value);
    }

    *length = len;

cleanup:
    return dterr;
}

// ------------------------------------------------------------------------
// pack the kvp list into the output buffer, error if buffer too small
extern dterr_t*
dtkvp_list_packx(dtkvp_list_t* self DTPACKABLE_PACKX_ARGS)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);
    DTERR_ASSERT_NOT_NULL(self->items);
    DTERR_ASSERT_NOT_NULL(output);

    int32_t p = offset ? *offset : 0;

    // first thing packed is the magic number
    DTPACKX_PACK(dtpackx_pack_int32(DTCORE_CONSTANTS_KVP_LIST_MAGIC_NUMBER, output, p, length), p, length);

    // determine how many actual kvps we have
    int32_t actual_count = 0;
    for (int32_t i = 0; i < self->count; i++)
    {
        dtkvp_t* kvp = &self->items[i];
        if (kvp->key != NULL)
            actual_count++;
    }

    // next thing packed is the count
    DTPACKX_PACK(dtpackx_pack_int32(actual_count, output, p, length), p, length);
    for (int32_t i = 0; i < self->count; i++)
    {
        dtkvp_t* kvp = &self->items[i];
        if (kvp->key == NULL)
            continue;
        DTPACKX_PACK(dtpackx_pack_string(kvp->key, output, p, length), p, length);
        DTPACKX_PACK(dtpackx_pack_string(kvp->value, output, p, length), p, length);
    }

    if (offset)
        *offset = p;

cleanup:
    return dterr;
}

// ------------------------------------------------------------------------
// unpack the kvp list from the input buffer, adding to any existing contents
// and growing as needed
extern dterr_t*
dtkvp_list_unpackx(dtkvp_list_t* self DTPACKABLE_UNPACKX_ARGS)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);
    DTERR_ASSERT_NOT_NULL(self->items);
    DTERR_ASSERT_NOT_NULL(input);
    char* key = NULL;
    char* value = NULL;

    int32_t p = offset ? *offset : 0;

    // first unpack is the magic number
    int32_t magic_number = 0;
    DTPACKX_UNPACK(dtpackx_unpack_int32(input, p, length, &magic_number), p, length);
    if (magic_number != DTCORE_CONSTANTS_KVP_LIST_MAGIC_NUMBER)
    {
        dterr = dterr_new(DTERR_BADMAGIC,
          DTERR_LOC,
          NULL,
          "kvp list unpack failed, invalid magic number 0x%08" PRIX32 ", expected 0x%08" PRIX32,
          magic_number,
          DTCORE_CONSTANTS_KVP_LIST_MAGIC_NUMBER);
        goto cleanup;
    }

    // next unpack is the count
    int32_t kvp_count = 0;
    DTPACKX_UNPACK(dtpackx_unpack_int32(input, p, length, &kvp_count), p, length);

    if (kvp_count < 0 || kvp_count > 10000)
    {
        dterr = dterr_new(DTERR_BADARG,
                          DTERR_LOC,
                          NULL,
                          "kvp list unpack: count %" PRId32 " out of range [0, 10000]",
                          kvp_count);
        goto cleanup;
    }

    // then unpack each kvp and store sequentially
    for (int32_t i = 0; i < kvp_count; i++)
    {
        DTPACKX_UNPACK(dtpackx_unpack_string(input, p, length, &key), p, length);
        DTPACKX_UNPACK(dtpackx_unpack_string(input, p, length, &value), p, length);

        // add or replace the kvp in the list, and grow list as necessary
        DTERR_C(dtkvp_list_set(self, key, value));

        // release temporary strings
        dtstr_dispose(key);
        key = NULL;
        dtstr_dispose(value);
        value = NULL;
    }

    if (offset)
        *offset = p;

cleanup:
    dtstr_dispose(key);
    dtstr_dispose(value);
    return dterr;
}

// ------------------------------------------------------------------------
// write kvp as key=value\n strings extending the provided buffer
dterr_t*
dtkvp_list_compose_plain_text(dtkvp_list_t* self, char** s, const char* separator)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);
    DTERR_ASSERT_NOT_NULL(self->items);
    DTERR_ASSERT_NOT_NULL(s);

    for (int32_t i = 0; i < self->count; i++)
    {
        dtkvp_t* kvp = &self->items[i];
        if (kvp->key != NULL)
        {
            // kvp->value should really never be NULL here, but handle it gracefully
            // just in case
            *s = dtstr_concat_format(*s, separator, "%s=%s", kvp->key, kvp->value ? kvp->value : "(null)");
        }
    }
cleanup:
    return dterr;
}

// ------------------------------------------------------------------------
// dispose the kvp list and all its contents, tolerates a null pointer or
// partially initialized list
extern void
dtkvp_list_dispose(dtkvp_list_t* self)
{
    if (self == NULL)
        return;

    if (self->items)
        for (int32_t i = 0; i < self->count; i++)
        {
            dtkvp_t* kvp = &self->items[i];
            dtstr_dispose((char*)kvp->key);
            dtstr_dispose((char*)kvp->value);
        }

    dtbuffer_dispose(self->storage);

    memset(self, 0, sizeof(*self));
}

// ------------------------------------------------------------------------
static bool
dtkvp_list__is_url_unreserved(char c)
{
    return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' ||
            c == '~');
}

// ------------------------------------------------------------------------
static char
dtkvp_list__hex_digit(uint8_t v)
{
    return (v < 10) ? (char)('0' + v) : (char)('A' + (v - 10));
}

// ------------------------------------------------------------------------
static int
dtkvp_list__from_hex_char(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return 10 + (c - 'A');
    if (c >= 'a' && c <= 'f')
        return 10 + (c - 'a');
    return -1;
}

// ------------------------------------------------------------------------
static dterr_t*
dtkvp_list__append_char(char** s, int32_t* length, int32_t* capacity, char c)
{
    dterr_t* dterr = NULL;
    char* new_s = NULL;
    int32_t new_capacity = 0;

    DTERR_ASSERT_NOT_NULL(s);
    DTERR_ASSERT_NOT_NULL(length);
    DTERR_ASSERT_NOT_NULL(capacity);

    if (*capacity <= (*length + 1))
    {
        if (*capacity > 0 && *capacity > INT32_MAX / 2)
        {
            dterr = dterr_new(DTERR_OVERFLOW, DTERR_LOC, NULL, "string buffer capacity overflow");
            goto cleanup;
        }
        new_capacity = (*capacity == 0) ? 64 : (*capacity * 2);
        while (new_capacity <= (*length + 1))
        {
            if (new_capacity > INT32_MAX / 2)
            {
                dterr = dterr_new(DTERR_OVERFLOW, DTERR_LOC, NULL, "string buffer capacity overflow");
                goto cleanup;
            }
            new_capacity *= 2;
        }

        new_s = (char*)realloc(*s, (size_t)new_capacity);
        if (new_s == NULL)
        {
            dterr = dterr_new(DTERR_STATE, DTERR_LOC, NULL, "realloc failed while appending string");
            goto cleanup;
        }

        *s = new_s;
        *capacity = new_capacity;
    }

    (*s)[*length] = c;
    (*length)++;
    (*s)[*length] = '\0';

cleanup:
    return dterr;
}

// ------------------------------------------------------------------------
static dterr_t*
dtkvp_list__append_urlencoded(char** s, int32_t* length, int32_t* capacity, const char* input)
{
    dterr_t* dterr = NULL;
    const unsigned char* p = (const unsigned char*)input;

    DTERR_ASSERT_NOT_NULL(s);
    DTERR_ASSERT_NOT_NULL(length);
    DTERR_ASSERT_NOT_NULL(capacity);
    DTERR_ASSERT_NOT_NULL(input);

    while (*p != '\0')
    {
        if (dtkvp_list__is_url_unreserved((char)*p))
        {
            DTERR_C(dtkvp_list__append_char(s, length, capacity, (char)*p));
        }
        else if (*p == ' ')
        {
            DTERR_C(dtkvp_list__append_char(s, length, capacity, '+'));
        }
        else
        {
            DTERR_C(dtkvp_list__append_char(s, length, capacity, '%'));
            DTERR_C(dtkvp_list__append_char(s, length, capacity, dtkvp_list__hex_digit((*p >> 4) & 0x0F)));
            DTERR_C(dtkvp_list__append_char(s, length, capacity, dtkvp_list__hex_digit(*p & 0x0F)));
        }
        p++;
    }

cleanup:
    return dterr;
}
// ------------------------------------------------------------------------
static dterr_t*
dtkvp_list__urldecode_component(const char* input, int32_t input_length, char** out)
{
    dterr_t* dterr = NULL;
    char* temp = NULL;
    char* final = NULL;
    int32_t w = 0;

    DTERR_ASSERT_NOT_NULL(input);
    DTERR_ASSERT_NOT_NULL(out);

    temp = (char*)calloc((size_t)input_length + 1, 1);
    if (temp == NULL)
    {
        dterr = dterr_new(DTERR_STATE, DTERR_LOC, NULL, "failed to allocate temp decode buffer");
        goto cleanup;
    }

    for (int32_t i = 0; i < input_length; i++)
    {
        char c = input[i];

        if (c == '+')
        {
            temp[w++] = ' ';
        }
        else if (c == '%')
        {
            int hi;
            int lo;

            if ((i + 2) >= input_length)
            {
                dterr = dterr_new(DTERR_STATE, DTERR_LOC, NULL, "malformed url encoding: truncated percent escape");
                goto cleanup;
            }

            hi = dtkvp_list__from_hex_char(input[i + 1]);
            lo = dtkvp_list__from_hex_char(input[i + 2]);
            if (hi < 0 || lo < 0)
            {
                dterr = dterr_new(DTERR_STATE, DTERR_LOC, NULL, "malformed url encoding: invalid percent escape");
                goto cleanup;
            }

            temp[w++] = (char)((hi << 4) | lo);
            i += 2;
        }
        else
        {
            temp[w++] = c;
        }
    }

    temp[w] = '\0';

    final = dtstr_dup(temp);
    if (final == NULL)
    {
        dterr = dterr_new(DTERR_STATE, DTERR_LOC, NULL, "failed to allocate decoded dtstr");
        goto cleanup;
    }

    *out = final;
    final = NULL;

cleanup:
    free(temp);
    dtstr_dispose(final);
    return dterr;
}

// 2026-04-10 crev v1.0.2