#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <dtcore/dterr.h>
#include <dtcore/dtguid.h>
#include <dtcore/dtguidable.h>
#include <dtcore/dtguidable_pool.h>
#include <dtcore/dtheaper.h>
#include <dtcore/dtlog.h>
#include <dtcore/dtobject.h>
#include <dtcore/dtpackable.h>
#include <dtcore/dtpackx.h>
#include <dtcore/dtvtable.h>

#include <dtcore/dtunittest.h>

#define TAG "test_dtguidable_pool"

// ============================================================================
// Minimal in-test models implementing dtguidable (and dtpackable where needed)
// ============================================================================
//
// These are deliberately tiny “facade satisfiers” for dtguidable_pool tests.
// They are private to this test file and avoid depending on external dtdummy_*.
//

// pick model numbers that are very unlikely to collide with real library models
#define TEST_GUIDABLE_ONE_MODEL_NUMBER 70001
#define TEST_GUIDABLE_TWO_MODEL_NUMBER 70002
#define TEST_PACKABLE_MODEL_NUMBER 70003

// ------------------------------
// test_guidable_one (guidable-only)
// ------------------------------

typedef struct test_guidable_one_t
{
    int32_t model_number;
    dtguid_t guid;
} test_guidable_one_t;

DTOBJECT_DECLARE_API(test_guidable_one);
DTGUIDABLE_DECLARE_API(test_guidable_one);

DTOBJECT_INIT_VTABLE(test_guidable_one)
DTGUIDABLE_INIT_VTABLE(test_guidable_one)

static bool test_guidable_one_vtables_registered = false;

dterr_t*
test_guidable_one_register_vtables(void)
{
    dterr_t* dterr = NULL;

    if (!test_guidable_one_vtables_registered)
    {
        DTERR_C(dtobject_set_vtable(TEST_GUIDABLE_ONE_MODEL_NUMBER, &test_guidable_one_object_vt));
        DTERR_C(dtguidable_set_vtable(TEST_GUIDABLE_ONE_MODEL_NUMBER, &test_guidable_one_guidable_vt));
        test_guidable_one_vtables_registered = true;
    }

cleanup:
    return dterr;
}

dterr_t*
test_guidable_one_create(test_guidable_one_t** self_ptr)
{
    dterr_t* dterr = NULL;

    DTERR_C(dtheaper_alloc_and_zero(sizeof(test_guidable_one_t), "test_guidable_one_t", (void**)self_ptr));
    test_guidable_one_t* self = *self_ptr;
    self->model_number = TEST_GUIDABLE_ONE_MODEL_NUMBER;
    dtguid_generate_sequential(&self->guid);

    DTERR_C(test_guidable_one_register_vtables());

cleanup:
    return dterr;
}

// dtobject (minimal, not used by these tests but required by facade registration)
void
test_guidable_one_copy(test_guidable_one_t* self, test_guidable_one_t* that)
{
    (void)self;
    (void)that;
}
void
test_guidable_one_dispose(test_guidable_one_t* self)
{
    if (self)
    {
        dtheaper_free(self);
    }
}
bool
test_guidable_one_equals(test_guidable_one_t* self, test_guidable_one_t* that)
{
    return self == that;
}
const char*
test_guidable_one_get_class(test_guidable_one_t* self)
{
    (void)self;
    return "test_guidable_one";
}
bool
test_guidable_one_is_iface(test_guidable_one_t* self, const char* iface_name)
{
    (void)self;
    return iface_name && strcmp(iface_name, "test_guidable_one") == 0;
}
void
test_guidable_one_to_string(test_guidable_one_t* self, char* buffer, size_t buffer_size)
{
    if (!buffer || buffer_size == 0)
        return;
    (void)self;
    snprintf(buffer, buffer_size, "test_guidable_one");
}

// dtguidable
dterr_t*
test_guidable_one_get_guid(test_guidable_one_t* self, dtguid_t* guid)
{
    dterr_t* dterr = NULL;

    if (self == NULL || guid == NULL)
        return dterr_new(DTERR_BADARG, DTERR_LOC, NULL, "null arg");

    dtguid_copy(guid, &self->guid);
    return dterr;
}

// ------------------------------
// test_guidable_two (guidable-only)
// ------------------------------

typedef struct test_guidable_two_t
{
    int32_t model_number;
    dtguid_t guid;
} test_guidable_two_t;

static bool test_guidable_two_vtables_registered = false;

DTOBJECT_DECLARE_API(test_guidable_two);
DTGUIDABLE_DECLARE_API(test_guidable_two);

DTOBJECT_INIT_VTABLE(test_guidable_two)
DTGUIDABLE_INIT_VTABLE(test_guidable_two)

static dterr_t*
test_guidable_two_register_vtables(void)
{
    dterr_t* dterr = NULL;

    if (!test_guidable_two_vtables_registered)
    {
        DTERR_C(dtobject_set_vtable(TEST_GUIDABLE_TWO_MODEL_NUMBER, &test_guidable_two_object_vt));
        DTERR_C(dtguidable_set_vtable(TEST_GUIDABLE_TWO_MODEL_NUMBER, &test_guidable_two_guidable_vt));
        test_guidable_two_vtables_registered = true;
    }

cleanup:
    return dterr;
}

dterr_t*
test_guidable_two_create(test_guidable_two_t** self_ptr)
{
    dterr_t* dterr = NULL;

    DTERR_C(dtheaper_alloc_and_zero(sizeof(test_guidable_two_t), "test_guidable_two_t", (void**)self_ptr));
    test_guidable_two_t* self = *self_ptr;
    self->model_number = TEST_GUIDABLE_TWO_MODEL_NUMBER;
    dtguid_generate_sequential(&self->guid);

    DTERR_C(test_guidable_two_register_vtables());
cleanup:
    return dterr;
}

// dtobject (minimal)
void
test_guidable_two_copy(test_guidable_two_t* self, test_guidable_two_t* that)
{
    (void)self;
    (void)that;
}
void
test_guidable_two_dispose(test_guidable_two_t* self)
{
    if (self)
    {
        dtheaper_free(self);
    }
}
bool
test_guidable_two_equals(test_guidable_two_t* self, test_guidable_two_t* that)
{
    return self == that;
}
const char*
test_guidable_two_get_class(test_guidable_two_t* self)
{
    (void)self;
    return "test_guidable_two";
}
bool
test_guidable_two_is_iface(test_guidable_two_t* self, const char* iface_name)
{
    (void)self;
    return iface_name && strcmp(iface_name, "test_guidable_two") == 0;
}
void
test_guidable_two_to_string(test_guidable_two_t* self, char* buffer, size_t buffer_size)
{
    if (!buffer || buffer_size == 0)
        return;
    (void)self;
    snprintf(buffer, buffer_size, "test_guidable_two");
}

// dtguidable
dterr_t*
test_guidable_two_get_guid(test_guidable_two_t* self, dtguid_t* guid)
{
    dterr_t* dterr = NULL;

    if (self == NULL || guid == NULL)
        return dterr_new(DTERR_BADARG, DTERR_LOC, NULL, "null arg");

    dtguid_copy(guid, &self->guid);
    return dterr;
}

// ------------------------------
// test_packable (guidable + packable)
// ------------------------------

typedef struct test_packable_t
{
    int32_t model_number;
    dtguid_t guid;
} test_packable_t;

static bool test_packable_vtables_registered = false;

DTOBJECT_DECLARE_API(test_packable);
DTGUIDABLE_DECLARE_API(test_packable);
DTPACKABLE_DECLARE_API(test_packable);

DTOBJECT_INIT_VTABLE(test_packable)
DTGUIDABLE_INIT_VTABLE(test_packable)
DTPACKABLE_INIT_VTABLE(test_packable)

static dterr_t*
test_packable_register_vtables(void)
{
    dterr_t* dterr = NULL;

    if (!test_packable_vtables_registered)
    {
        DTERR_C(dtobject_set_vtable(TEST_PACKABLE_MODEL_NUMBER, &test_packable_object_vt));
        DTERR_C(dtguidable_set_vtable(TEST_PACKABLE_MODEL_NUMBER, &test_packable_guidable_vt));
        DTERR_C(dtpackable_set_vtable(TEST_PACKABLE_MODEL_NUMBER, &test_packable_packable_vt));
        test_packable_vtables_registered = true;
    }

cleanup:
    return dterr;
}

dterr_t*
test_packable_create(test_packable_t** self_ptr)
{
    dterr_t* dterr = NULL;

    DTERR_C(dtheaper_alloc_and_zero(sizeof(test_packable_t), "test_packable_t", (void**)self_ptr));
    test_packable_t* self = *self_ptr;
    self->model_number = TEST_PACKABLE_MODEL_NUMBER;
    dtguid_generate_sequential(&self->guid);

    DTERR_C(test_packable_register_vtables());
cleanup:
    return dterr;
}

// dtobject (minimal)
void
test_packable_copy(test_packable_t* self, test_packable_t* that)
{
    (void)self;
    (void)that;
}
void
test_packable_dispose(test_packable_t* self)
{
    if (self)
    {
        dtheaper_free(self);
    }
}
bool
test_packable_equals(test_packable_t* self, test_packable_t* that)
{
    return self == that;
}
const char*
test_packable_get_class(test_packable_t* self)
{
    (void)self;
    return "test_packable";
}
bool
test_packable_is_iface(test_packable_t* self, const char* iface_name)
{
    (void)self;
    return iface_name && strcmp(iface_name, "test_packable") == 0;
}
void
test_packable_to_string(test_packable_t* self, char* buffer, size_t buffer_size)
{
    if (!buffer || buffer_size == 0)
        return;
    (void)self;
    snprintf(buffer, buffer_size, "test_packable");
}

// dtguidable
dterr_t*
test_packable_get_guid(test_packable_t* self, dtguid_t* guid)
{
    dterr_t* dterr = NULL;

    if (self == NULL || guid == NULL)
        return dterr_new(DTERR_BADARG, DTERR_LOC, NULL, "null arg");

    dtguid_copy(guid, &self->guid);
    return dterr;
}

// dtpackable (minimal but real: packs model_number + guid)
dterr_t*
test_packable_packx_length(test_packable_t* self, int32_t* length)
{
    (void)self;
    if (length == NULL)
        return dterr_new(DTERR_BADARG, DTERR_LOC, NULL, "null arg");

    *length = dtpackx_pack_int32_length() + dtguid_pack_length();
    return NULL;
}

dterr_t*
test_packable_packx(test_packable_t* self, uint8_t* output, int32_t* offset, int32_t length)
{
    dterr_t* dterr = NULL;
    int32_t p = offset ? *offset : 0;

    if (self == NULL || output == NULL || offset == NULL)
        return dterr_new(DTERR_BADARG, DTERR_LOC, NULL, "null arg");

    int32_t n = dtpackx_pack_int32(self->model_number, output, p, length);
    if (n < 0)
        return dterr_new(DTERR_FAIL, DTERR_LOC, NULL, "pack int32 failed");
    p += n;

    n = dtguid_pack(&self->guid, output, p, length);
    if (n < 0)
        return dterr_new(DTERR_FAIL, DTERR_LOC, NULL, "pack guid failed");
    p += n;

    *offset = p;
    return dterr;
}

// not needed by these tests, but provide a stub to satisfy the vtable
dterr_t*
test_packable_unpackx(test_packable_t* self, const uint8_t* input, int32_t* offset, int32_t length)
{
    dterr_t* dterr = NULL;
    int32_t p = offset ? *offset : 0;

    if (self == NULL || input == NULL || offset == NULL)
        return dterr_new(DTERR_BADARG, DTERR_LOC, NULL, "null arg");

    int32_t model = 0;

    // Mirror packx(): model_number then guid.
    int32_t n = dtpackx_unpack_int32(input, p, length, &model);
    if (n < 0)
        return dterr_new(DTERR_FAIL, DTERR_LOC, NULL, "unpack int32 failed");
    p += n;

    dtguid_t guid = { 0 };
    n = dtguid_unpack(&guid, input, p, length);
    if (n < 0)
        return dterr_new(DTERR_FAIL, DTERR_LOC, NULL, "unpack guid failed");
    p += n;

    self->model_number = model;
    dtguid_copy(&self->guid, &guid);

    *offset = p;
    return dterr;
}

// not needed by these tests, but provide a stub to satisfy the vtable
dterr_t*
test_packable_validate_unpacked(test_packable_t* self)
{
    (void)self;
    return dterr_new(DTERR_NOTIMPL, DTERR_LOC, NULL, "validated_unpacked not implemented in test model");
}

// ============================================================================
// Tests
// ============================================================================

static dterr_t*
test_dtguidable_pool_insert_and_search(void)
{
    dterr_t* dterr = NULL;
    dtguidable_pool_t pool = { 0 };
    test_guidable_one_t* a = NULL;
    test_guidable_two_t* b = NULL;

    DTERR_C(dtguidable_pool_init(&pool, 4));

    DTERR_C(test_guidable_one_create(&a));
    DTERR_C(test_guidable_two_create(&b));

    DTERR_C(dtguidable_pool_insert(&pool, (dtguidable_handle)a));
    DTERR_C(dtguidable_pool_insert(&pool, (dtguidable_handle)b));

    dtguidable_handle found = NULL;
    DTERR_C(dtguidable_pool_search(&pool, &a->guid, &found));
    DTUNITTEST_ASSERT_TRUE(found == (dtguidable_handle)a);

    found = NULL;
    DTERR_C(dtguidable_pool_search(&pool, &b->guid, &found));
    DTUNITTEST_ASSERT_TRUE(found == (dtguidable_handle)b);

cleanup:
    dtguidable_pool_dispose(&pool);
    test_guidable_one_dispose(a);
    test_guidable_two_dispose(b);

    if (dterr != NULL)
        dterr = dterr_new(DTERR_FAIL, __LINE__, __FILE__, __func__, dterr, "test failed: %s", dterr->message);
    return dterr;
}

static dterr_t*
test_dtguidable_pool_duplicate_insert(void)
{
    dterr_t* dterr = NULL;
    dtguidable_pool_t pool = { 0 };
    test_guidable_one_t* a = NULL;

    DTERR_C(dtguidable_pool_init(&pool, 2));

    DTERR_C(test_guidable_one_create(&a));
    DTERR_C(dtguidable_pool_insert(&pool, (dtguidable_handle)a));

    dterr = dtguidable_pool_insert(&pool, (dtguidable_handle)a);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_EXISTS);

cleanup:
    dtguidable_pool_dispose(&pool);
    test_guidable_one_dispose(a);

    if (dterr != NULL)
        dterr = dterr_new(DTERR_FAIL, __LINE__, __FILE__, __func__, dterr, "test failed: %s", dterr->message);
    return dterr;
}

static dterr_t*
test_dtguidable_pool_remove(void)
{
    dterr_t* dterr = NULL;
    dtguidable_pool_t pool = { 0 };
    test_guidable_one_t* a = NULL;

    DTERR_C(dtguidable_pool_init(&pool, 2));

    DTERR_C(test_guidable_one_create(&a));

    DTERR_C(dtguidable_pool_insert(&pool, (dtguidable_handle)a));
    DTERR_C(dtguidable_pool_remove(&pool, (dtguidable_handle)a));

    dtguidable_handle found = (dtguidable_handle)1; // anything non-NULL
    DTERR_C(dtguidable_pool_search(&pool, &a->guid, &found));
    DTUNITTEST_ASSERT_TRUE(found == NULL);

cleanup:
    dtguidable_pool_dispose(&pool);
    test_guidable_one_dispose(a);

    if (dterr != NULL)
        dterr = dterr_new(DTERR_FAIL, __LINE__, __FILE__, __func__, dterr, "test failed: %s", dterr->message);
    return dterr;
}

static dterr_t*
test_dtguidable_pool_search_notfound(void)
{
    dterr_t* dterr = NULL;
    dtguidable_pool_t pool = { 0 };
    test_guidable_one_t* a = NULL;

    DTERR_C(dtguidable_pool_init(&pool, 2));

    DTERR_C(test_guidable_one_create(&a));

    dtguidable_handle found = (dtguidable_handle)1;
    DTERR_C(dtguidable_pool_search(&pool, &a->guid, &found));
    DTUNITTEST_ASSERT_TRUE(found == NULL);

cleanup:
    dtguidable_pool_dispose(&pool);
    test_guidable_one_dispose(a);

    if (dterr != NULL)
        dterr = dterr_new(DTERR_FAIL, __LINE__, __FILE__, __func__, dterr, "test failed: %s", dterr->message);
    return dterr;
}

static dterr_t*
test_dtguidable_pool_packx(void)
{
    dterr_t* dterr = NULL;
    dtguidable_pool_t pool = { 0 };
    test_packable_t* a = NULL;
    test_packable_t* b = NULL;

    DTERR_C(dtguidable_pool_init(&pool, 2));

    DTERR_C(test_packable_create(&a));
    DTERR_C(test_packable_create(&b));
    DTERR_C(dtguidable_pool_insert(&pool, (dtguidable_handle)a));
    DTERR_C(dtguidable_pool_insert(&pool, (dtguidable_handle)b));

    uint8_t output[256];
    int32_t offset = 0;

    DTERR_C(dtguidable_pool_packx(&pool, output, &offset, sizeof(output)));

    int32_t expected_length = 0;
    DTERR_C(dtpackable_packx_length((dtpackable_handle)a, &expected_length));
    expected_length *= 2;
    DTUNITTEST_ASSERT_INT(offset, ==, expected_length);

cleanup:
    dtguidable_pool_dispose(&pool);
    test_packable_dispose(a);
    test_packable_dispose(b);

    if (dterr != NULL)
        dterr = dterr_new(DTERR_FAIL, __LINE__, __FILE__, __func__, dterr, "test failed: %s", dterr->message);
    return dterr;
}

void
test_dtguidable_pool(DTUNITTEST_SUITE_ARGS)
{
    DTUNITTEST_RUN_TEST(test_dtguidable_pool_insert_and_search);
    DTUNITTEST_RUN_TEST(test_dtguidable_pool_duplicate_insert);
    DTUNITTEST_RUN_TEST(test_dtguidable_pool_remove);
    DTUNITTEST_RUN_TEST(test_dtguidable_pool_search_notfound);
    DTUNITTEST_RUN_TEST(test_dtguidable_pool_packx);
}
