#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dterr.h>
#include <dtcore/dtheaper.h>
#include <dtcore/dtkvp.h>
#include <dtcore/dtlog.h>
#include <dtcore/dtrpc.h>
#include <dtcore/dtstr.h>
#include <dtcore/dtunittest.h>
#include <dtcore/dtvtable.h>

#include <dtcore_tests.h>

#define TAG "test_dtcore_dtrpc"

typedef struct test_dtcore_dtrpc_myrpc_t
{
    int32_t model_number;
    const char* my_name;
    int32_t call_count;
} test_dtcore_dtrpc_myrpc_t;

DTRPC_DECLARE_API(test_dtcore_dtrpc_myrpc);
DTRPC_INIT_VTABLE(test_dtcore_dtrpc_myrpc);

// ------------------------------------------------------------------------
static dterr_t*
test_dtcore_dtrpc_myrpc_create(test_dtcore_dtrpc_myrpc_t** self_ptr, int32_t model_number, const char* name)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self_ptr);

    DTERR_C(dtrpc_set_vtable(model_number, &test_dtcore_dtrpc_myrpc_vt));

    DTERR_C(dtheaper_alloc_and_zero((int32_t)sizeof(test_dtcore_dtrpc_myrpc_t), "test_dtcore_dtrpc_myrpc_t", (void**)self_ptr));

    test_dtcore_dtrpc_myrpc_t* self = (test_dtcore_dtrpc_myrpc_t*)*self_ptr;

    self->model_number = model_number;
    self->my_name = name;

cleanup:
    return dterr;
}

// ------------------------------------------------------------------------
extern dterr_t*
test_dtcore_dtrpc_myrpc_call(test_dtcore_dtrpc_myrpc_t* self DTRPC_CALL_ARGS)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);
    DTERR_ASSERT_NOT_NULL(request_kvp_list);
    DTERR_ASSERT_NOT_NULL(was_refused);
    DTERR_ASSERT_NOT_NULL(response_kvp_list);

    *was_refused = false;

    self->call_count += 1;

    // check for expected request param, if not present or wrong then refuse the call
    const char* id_param = NULL;
    DTERR_C(dtkvp_list_get(request_kvp_list, "id_param", &id_param));
    if (id_param == NULL || strcmp(id_param, self->my_name) != 0)
    {
        *was_refused = true;
        goto cleanup;
    }

    DTERR_C(dtkvp_list_set(response_kvp_list, "response_param", self->my_name));

cleanup:
    return dterr;
}

// ------------------------------------------------------------------------
extern void
test_dtcore_dtrpc_myrpc_dispose(test_dtcore_dtrpc_myrpc_t* self)
{
    if (self != NULL)
    {
        dtheaper_free(self);
    }
}

// ------------------------------------------------------------------------
// Example: Small, readable "happy path" showing basic formatting & cleanup.
static dterr_t*
test_dtcore_dtrpc_example_basic(void)
{
    dterr_t* dterr = NULL;
    dtrpc_handle rpc_handle1 = NULL;
    dtkvp_list_t _request_kvp_list = { 0 }, *request_kvp_list = &_request_kvp_list;
    dtkvp_list_t _response_kvp_list = { 0 }, *response_kvp_list = &_response_kvp_list;
    bool was_refused = false;

    test_dtcore_dtrpc_myrpc_t* rpc_object1 = NULL;
    DTERR_C(test_dtcore_dtrpc_myrpc_create(&rpc_object1, 1, "rpc_object1"));
    rpc_handle1 = (dtrpc_handle)rpc_object1;

    DTERR_C(dtkvp_list_init(request_kvp_list));
    DTERR_C(dtkvp_list_init(response_kvp_list));

    // initially call with no request params, should be refused
    DTERR_C(dtrpc_call(rpc_handle1, request_kvp_list, &was_refused, response_kvp_list));
    DTUNITTEST_ASSERT_TRUE(was_refused);
    DTUNITTEST_ASSERT_INT(rpc_object1->call_count, ==, 1);

    // now set the expected request param, should succeed
    DTERR_C(dtkvp_list_set(request_kvp_list, "id_param", "rpc_object1"));
    DTERR_C(dtrpc_call(rpc_handle1, request_kvp_list, &was_refused, response_kvp_list));
    DTUNITTEST_ASSERT_TRUE(!was_refused);
    DTUNITTEST_ASSERT_INT(rpc_object1->call_count, ==, 2);

    // check for expected response param
    {
        const char* message = NULL;
        DTERR_C(dtkvp_list_get(response_kvp_list, "response_param", &message));
        DTUNITTEST_ASSERT_NOT_NULL(message);
        DTUNITTEST_ASSERT_EQUAL_STRING(message, "rpc_object1");
    }

cleanup:
    dtkvp_list_dispose(response_kvp_list);
    dtkvp_list_dispose(request_kvp_list);
    dtrpc_dispose(rpc_handle1);
    return dterr;
}

// ------------------------------------------------------------------------
// Non-example: wrong id_param should refuse the call and not set response_param.
static dterr_t*
test_dtcore_dtrpc_wrong_id_is_refused(void)
{
    dterr_t* dterr = NULL;
    dtrpc_handle rpc_handle = NULL;
    dtkvp_list_t _request_kvp_list = { 0 }, *request_kvp_list = &_request_kvp_list;
    dtkvp_list_t _response_kvp_list = { 0 }, *response_kvp_list = &_response_kvp_list;
    bool was_refused = false;
    const char* response_param = NULL;

    test_dtcore_dtrpc_myrpc_t* rpc_object = NULL;
    DTERR_C(test_dtcore_dtrpc_myrpc_create(&rpc_object, 10, "expected_name"));
    rpc_handle = (dtrpc_handle)rpc_object;

    DTERR_C(dtkvp_list_init(request_kvp_list));
    DTERR_C(dtkvp_list_init(response_kvp_list));

    DTERR_C(dtkvp_list_set(request_kvp_list, "id_param", "wrong_name"));
    DTERR_C(dtrpc_call(rpc_handle, request_kvp_list, &was_refused, response_kvp_list));

    DTUNITTEST_ASSERT_TRUE(was_refused);
    DTUNITTEST_ASSERT_INT(rpc_object->call_count, ==, 1);

    DTERR_C(dtkvp_list_get(response_kvp_list, "response_param", &response_param));
    DTUNITTEST_ASSERT_TRUE(response_param == NULL);

cleanup:
    dtkvp_list_dispose(response_kvp_list);
    dtkvp_list_dispose(request_kvp_list);
    dtrpc_dispose(rpc_handle);
    return dterr;
}

// ------------------------------------------------------------------------
// Non-example: refusal should not overwrite response data already present.
static dterr_t*
test_dtcore_dtrpc_refused_call_leaves_existing_response_unchanged(void)
{
    dterr_t* dterr = NULL;
    dtrpc_handle rpc_handle = NULL;
    dtkvp_list_t _request_kvp_list = { 0 }, *request_kvp_list = &_request_kvp_list;
    dtkvp_list_t _response_kvp_list = { 0 }, *response_kvp_list = &_response_kvp_list;
    bool was_refused = false;
    const char* response_param = NULL;

    test_dtcore_dtrpc_myrpc_t* rpc_object = NULL;
    DTERR_C(test_dtcore_dtrpc_myrpc_create(&rpc_object, 11, "real_name"));
    rpc_handle = (dtrpc_handle)rpc_object;

    DTERR_C(dtkvp_list_init(request_kvp_list));
    DTERR_C(dtkvp_list_init(response_kvp_list));

    DTERR_C(dtkvp_list_set(response_kvp_list, "response_param", "old_value"));
    DTERR_C(dtkvp_list_set(request_kvp_list, "id_param", "not_real_name"));

    DTERR_C(dtrpc_call(rpc_handle, request_kvp_list, &was_refused, response_kvp_list));

    DTUNITTEST_ASSERT_TRUE(was_refused);
    DTUNITTEST_ASSERT_INT(rpc_object->call_count, ==, 1);

    DTERR_C(dtkvp_list_get(response_kvp_list, "response_param", &response_param));
    DTUNITTEST_ASSERT_NOT_NULL(response_param);
    DTUNITTEST_ASSERT_EQUAL_STRING(response_param, "old_value");

cleanup:
    dtkvp_list_dispose(response_kvp_list);
    dtkvp_list_dispose(request_kvp_list);
    dtrpc_dispose(rpc_handle);
    return dterr;
}

// ------------------------------------------------------------------------
// Non-example: repeated successful calls should continue to work and keep count.
static dterr_t*
test_dtcore_dtrpc_repeated_successful_calls_increment_count(void)
{
    dterr_t* dterr = NULL;
    dtrpc_handle rpc_handle = NULL;
    dtkvp_list_t _request_kvp_list = { 0 }, *request_kvp_list = &_request_kvp_list;
    dtkvp_list_t _response_kvp_list = { 0 }, *response_kvp_list = &_response_kvp_list;
    bool was_refused = false;
    const char* response_param = NULL;

    test_dtcore_dtrpc_myrpc_t* rpc_object = NULL;
    DTERR_C(test_dtcore_dtrpc_myrpc_create(&rpc_object, 12, "repeat_me"));
    rpc_handle = (dtrpc_handle)rpc_object;

    DTERR_C(dtkvp_list_init(request_kvp_list));
    DTERR_C(dtkvp_list_init(response_kvp_list));

    DTERR_C(dtkvp_list_set(request_kvp_list, "id_param", "repeat_me"));

    DTERR_C(dtrpc_call(rpc_handle, request_kvp_list, &was_refused, response_kvp_list));
    DTUNITTEST_ASSERT_TRUE(!was_refused);

    DTERR_C(dtrpc_call(rpc_handle, request_kvp_list, &was_refused, response_kvp_list));
    DTUNITTEST_ASSERT_TRUE(!was_refused);

    DTERR_C(dtrpc_call(rpc_handle, request_kvp_list, &was_refused, response_kvp_list));
    DTUNITTEST_ASSERT_TRUE(!was_refused);

    DTUNITTEST_ASSERT_INT(rpc_object->call_count, ==, 3);

    DTERR_C(dtkvp_list_get(response_kvp_list, "response_param", &response_param));
    DTUNITTEST_ASSERT_NOT_NULL(response_param);
    DTUNITTEST_ASSERT_EQUAL_STRING(response_param, "repeat_me");

cleanup:
    dtkvp_list_dispose(response_kvp_list);
    dtkvp_list_dispose(request_kvp_list);
    dtrpc_dispose(rpc_handle);
    return dterr;
}

// ------------------------------------------------------------------------
// Non-example: two RPC objects should dispatch independently and maintain separate state.
static dterr_t*
test_dtcore_dtrpc_multiple_objects_keep_independent_state(void)
{
    dterr_t* dterr = NULL;
    dtrpc_handle rpc_handle1 = NULL;
    dtrpc_handle rpc_handle2 = NULL;

    dtkvp_list_t _request1 = { 0 }, *request1 = &_request1;
    dtkvp_list_t _request2 = { 0 }, *request2 = &_request2;
    dtkvp_list_t _response1 = { 0 }, *response1 = &_response1;
    dtkvp_list_t _response2 = { 0 }, *response2 = &_response2;

    bool was_refused1 = false;
    bool was_refused2 = false;

    const char* response_param1 = NULL;
    const char* response_param2 = NULL;

    test_dtcore_dtrpc_myrpc_t* rpc_object1 = NULL;
    test_dtcore_dtrpc_myrpc_t* rpc_object2 = NULL;

    DTERR_C(test_dtcore_dtrpc_myrpc_create(&rpc_object1, 21, "alpha"));
    DTERR_C(test_dtcore_dtrpc_myrpc_create(&rpc_object2, 22, "beta"));

    rpc_handle1 = (dtrpc_handle)rpc_object1;
    rpc_handle2 = (dtrpc_handle)rpc_object2;

    DTERR_C(dtkvp_list_init(request1));
    DTERR_C(dtkvp_list_init(request2));
    DTERR_C(dtkvp_list_init(response1));
    DTERR_C(dtkvp_list_init(response2));

    DTERR_C(dtkvp_list_set(request1, "id_param", "alpha"));
    DTERR_C(dtkvp_list_set(request2, "id_param", "beta"));

    DTERR_C(dtrpc_call(rpc_handle1, request1, &was_refused1, response1));
    DTERR_C(dtrpc_call(rpc_handle2, request2, &was_refused2, response2));
    DTERR_C(dtrpc_call(rpc_handle1, request1, &was_refused1, response1));

    DTUNITTEST_ASSERT_TRUE(!was_refused1);
    DTUNITTEST_ASSERT_TRUE(!was_refused2);

    DTUNITTEST_ASSERT_INT(rpc_object1->call_count, ==, 2);
    DTUNITTEST_ASSERT_INT(rpc_object2->call_count, ==, 1);

    DTERR_C(dtkvp_list_get(response1, "response_param", &response_param1));
    DTERR_C(dtkvp_list_get(response2, "response_param", &response_param2));

    DTUNITTEST_ASSERT_NOT_NULL(response_param1);
    DTUNITTEST_ASSERT_NOT_NULL(response_param2);
    DTUNITTEST_ASSERT_EQUAL_STRING(response_param1, "alpha");
    DTUNITTEST_ASSERT_EQUAL_STRING(response_param2, "beta");

cleanup:
    dtkvp_list_dispose(response2);
    dtkvp_list_dispose(response1);
    dtkvp_list_dispose(request2);
    dtkvp_list_dispose(request1);
    dtrpc_dispose(rpc_handle2);
    dtrpc_dispose(rpc_handle1);
    return dterr;
}

// ------------------------------------------------------------------------
void
test_dtcore_dtrpc(DTUNITTEST_SUITE_ARGS)
{
    // Examples first (teachable, self-contained).
    DTUNITTEST_RUN_TEST(test_dtcore_dtrpc_example_basic);

    // Additional coverage.
    DTUNITTEST_RUN_TEST(test_dtcore_dtrpc_wrong_id_is_refused);
    DTUNITTEST_RUN_TEST(test_dtcore_dtrpc_refused_call_leaves_existing_response_unchanged);
    DTUNITTEST_RUN_TEST(test_dtcore_dtrpc_repeated_successful_calls_increment_count);
    DTUNITTEST_RUN_TEST(test_dtcore_dtrpc_multiple_objects_keep_independent_state);
}