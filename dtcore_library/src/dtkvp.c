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
  for (int32_t i = 0; i < self->count; i++) {
    dtkvp_t* kvp = &self->items[i];
    if (kvp->key != NULL && strcmp(kvp->key, key) == 0) {
      return kvp;
    }
  }
  return NULL;
}
// ------------------------------------------------------------------------
static dtkvp_t*
dtkvp_list__find_available(dtkvp_list_t* self)
{
  for (int32_t i = 0; i < self->count; i++) {
    dtkvp_t* kvp = &self->items[i];
    if (kvp->key == NULL) {
      return kvp;
    }
  }
  return NULL;
}

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

  if (strcmp(key, DTCORE_CONSTANTS_KVP_KEY_WRITE_GUID) == 0) {
    dtlog_info(TAG, "setting key %s=%s", key, value ? value : "(null)");
  }

  dtkvp_t* kvp = dtkvp_list__find_key(self, key);

  // found existing key?
  if (kvp != NULL) {
    if (kvp->value)
      dtstr_dispose((char*)kvp->value);
    if (value == NULL) {
      // remove the key
      dtstr_dispose((char*)kvp->key);
      kvp->key = NULL;
      kvp->value = NULL;
    } else {
      // update the value
      kvp->value = dtstr_dup(value);
    }
  }
  // did not find existing key, but want to set a new value
  else if (value != NULL) {

    kvp = dtkvp_list__find_available(self);

    if (kvp == NULL) {
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
  for (int32_t i = 0; i < self->count; i++) {
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
  DTPACKX_PACK(dtpackx_pack_int32(
                 DTCORE_CONSTANTS_KVP_LIST_MAGIC_NUMBER, output, p, length),
               p,
               length);

  // determine how many actual kvps we have
  int32_t actual_count = 0;
  for (int32_t i = 0; i < self->count; i++) {
    dtkvp_t* kvp = &self->items[i];
    if (kvp->key != NULL)
      actual_count++;
  }

  // next thing packed is the count
  DTPACKX_PACK(dtpackx_pack_int32(actual_count, output, p, length), p, length);
  for (int32_t i = 0; i < self->count; i++) {
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
  DTPACKX_UNPACK(
    dtpackx_unpack_int32(input, p, length, &magic_number), p, length);
  if (magic_number != DTCORE_CONSTANTS_KVP_LIST_MAGIC_NUMBER) {
    dterr =
      dterr_new(DTERR_BADMAGIC,
                DTERR_LOC,
                NULL,
                "kvp list unpack failed, invalid magic number 0x%08" PRIX32
                ", expected 0x%08" PRIX32,
                magic_number,
                DTCORE_CONSTANTS_KVP_LIST_MAGIC_NUMBER);
    goto cleanup;
  }

  // next unpack is the count
  int32_t kvp_count = 0;
  DTPACKX_UNPACK(dtpackx_unpack_int32(input, p, length, &kvp_count), p, length);

  // then unpack each kvp and store sequentially
  for (int32_t i = 0; i < kvp_count; i++) {
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
dtkvp_list_compose_plain_text(dtkvp_list_t* self,
                              char** s,
                              const char* separator)
{
  dterr_t* dterr = NULL;
  DTERR_ASSERT_NOT_NULL(self);
  DTERR_ASSERT_NOT_NULL(self->items);

  for (int32_t i = 0; i < self->count; i++) {
    dtkvp_t* kvp = &self->items[i];
    if (kvp->key != NULL) {
      // kvp->value should really never be NULL here, but handle it gracefully
      // just in case
      *s = dtstr_concat_format(
        *s, separator, "%s=%s", kvp->key, kvp->value ? kvp->value : "(null)");
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
    for (int32_t i = 0; i < self->count; i++) {
      dtkvp_t* kvp = &self->items[i];
      dtstr_dispose((char*)kvp->key);
      dtstr_dispose((char*)kvp->value);
    }

  dtbuffer_dispose(self->storage);

  memset(self, 0, sizeof(*self));
}
