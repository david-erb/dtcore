#include <stdio.h>
#include <string.h>

#include <dtcore/dterr.h>
#include <dtcore/dtlog.h>
#include <dtcore/dtunittest.h>

#include <dtcore/dtbuffer.h>
#include <dtcore/dtchunker.h>
#include <dtcore_tests.h>

#define TAG "test_dtchunker"

/* Small helpers for readable test setup */
static void
fill_bytes(dtbuffer_t* b, const char* s, int32_t n)
{
    memcpy(b->payload, s, (size_t)n);
    b->length = n;
}
//------------------------------------------------------------------------
// Example 1: Basic roundtrip using header + multiple payload chunks
static dterr_t*
test_dtcore_dtchunker_example_roundtrip_chunks(void)
{
    dterr_t* dterr = NULL;

    dtchunker_t tx = { 0 }, rx = { 0 };
    dtbuffer_t *src = NULL, *chunk = NULL, *final = NULL;

    const char* msg = "The quick brown fox jumps over the lazy dog.";
    const int32_t msg_len = (int32_t)strlen(msg);

    DTERR_C(dtchunker_init(&tx, 8)); // small chunk size to force multiple chunks
    DTERR_C(dtchunker_init(&rx, 8));

    DTERR_C(dtbuffer_create(&src, msg_len));
    fill_bytes(src, msg, msg_len);

    do
    {
        DTERR_C(dtchunker_export(&tx, src, &chunk));
        if (chunk)
        {
            DTERR_C(dtchunker_import(&rx, chunk, &final));
        }
    } while (chunk != NULL);

    DTUNITTEST_ASSERT_NOT_NULL(final);
    DTUNITTEST_ASSERT_INT(final->length, ==, msg_len);
    DTUNITTEST_ASSERT_EQUAL_BYTES(final->payload, msg, (size_t)msg_len);

cleanup:
    dtbuffer_dispose(src);
    dtbuffer_dispose(final);
    dtchunker_dispose(&tx);
    dtchunker_dispose(&rx);
    return dterr;
}
//------------------------------------------------------------------------
// Example 2: Importer accepts "header + first payload" in the same first chunk
static dterr_t*
test_dtcore_dtchunker_example_header_plus_payload_first_chunk(void)
{
    dterr_t* dterr = NULL;

    dtchunker_t tx = { 0 }, rx = { 0 };
    dtbuffer_t *src = NULL, *chunk = NULL, *final = NULL;

    const char* msg = "ABCDEFGHIJK"; // 11 bytes
    const int32_t msg_len = (int32_t)strlen(msg);

    DTERR_C(dtchunker_init(&tx, 16));
    DTERR_C(dtchunker_init(&rx, 16));

    DTERR_C(dtbuffer_create(&src, msg_len));
    fill_bytes(src, msg, msg_len);

    /* Export header */
    DTERR_C(dtchunker_export(&tx, src, &chunk));
    DTUNITTEST_ASSERT_NOT_NULL(chunk);
    /* Feed header to importer */
    DTERR_C(dtchunker_import(&rx, chunk, &final));
    DTUNITTEST_ASSERT_PTR(final, ==, NULL);

    /* Continue payload flow (import supports header+payload, but header-only is fine too) */
    do
    {
        DTERR_C(dtchunker_export(&tx, src, &chunk));
        if (chunk)
        {
            DTERR_C(dtchunker_import(&rx, chunk, &final));
        }
    } while (chunk != NULL);

    DTUNITTEST_ASSERT_NOT_NULL(final);
    DTUNITTEST_ASSERT_INT(final->length, ==, msg_len);
    DTUNITTEST_ASSERT_EQUAL_BYTES(final->payload, msg, (size_t)msg_len);

cleanup:
    dtbuffer_dispose(src);
    dtbuffer_dispose(final);
    dtchunker_dispose(&tx);
    dtchunker_dispose(&rx);
    return dterr;
}
//------------------------------------------------------------------------
// Example 3: Zero-length buffer roundtrip (legal; yields header then completion)
static dterr_t*
test_dtcore_dtchunker_example_zero_length(void)
{
    dterr_t* dterr = NULL;

    dtchunker_t tx = { 0 }, rx = { 0 };
    dtbuffer_t *src = NULL, *chunk = NULL, *final = NULL;

    DTERR_C(dtchunker_init(&tx, 8));
    DTERR_C(dtchunker_init(&rx, 8));

    DTERR_C(dtbuffer_create(&src, 0));
    src->length = 0;

    /* Export header */
    DTERR_C(dtchunker_export(&tx, src, &chunk));
    DTUNITTEST_ASSERT_NOT_NULL(chunk);

    /* Import header */
    DTERR_C(dtchunker_import(&rx, chunk, &final));
    DTUNITTEST_ASSERT_PTR(final, ==, NULL);

    /* Next export should immediately end (no payload) */
    DTERR_C(dtchunker_export(&tx, src, &chunk));
    DTUNITTEST_ASSERT_PTR(chunk, ==, NULL);

    /* No more data will be imported; final should still be NULL. */
    DTUNITTEST_ASSERT_PTR(final, ==, NULL);

cleanup:
    dtbuffer_dispose(src);
    dtbuffer_dispose(final);
    dtchunker_dispose(&tx);
    dtchunker_dispose(&rx);
    return dterr;
}
//------------------------------------------------------------------------
static dterr_t*
test_dtcore_dtchunker_01_init_and_reset(void)
{
    dterr_t* dterr = NULL;

    dtchunker_t chunker = (dtchunker_t){ 0 };

    DTERR_C(dtchunker_init(&chunker, 16));
    DTUNITTEST_ASSERT_INT(chunker.max_chunk_size, ==, 16);
    DTUNITTEST_ASSERT_INT(chunker.mode, ==, DTCHUNKER_MODE_UNKNOWN);

    DTERR_C(dtchunker_reset(&chunker));
    DTUNITTEST_ASSERT_INT(chunker.mode, ==, DTCHUNKER_MODE_UNKNOWN);
    DTUNITTEST_ASSERT_INT(chunker.working_index, ==, 0);

cleanup:
    dtchunker_dispose(&chunker);
    return dterr;
}
//------------------------------------------------------------------------
static dterr_t*
test_dtcore_dtchunker_02_init_bad_chunk_size(void)
{
    dterr_t* dterr = NULL;

    dtchunker_t chunker = (dtchunker_t){ 0 };
    dterr = dtchunker_init(&chunker, 4); /* must be >= 8 */
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_BADARG);

    dtchunker_dispose(&chunker);

cleanup:
    return dterr;
}
//------------------------------------------------------------------------
static dterr_t*
test_dtcore_dtchunker_03_export_small_buffer(void)
{
    dterr_t* dterr = NULL;

    dtchunker_t chunker = (dtchunker_t){ 0 };
    DTERR_C(dtchunker_init(&chunker, 8));

    dtbuffer_t* src = NULL;
    const char* msg = "hello";
    const int32_t n = (int32_t)strlen(msg);

    DTERR_C(dtbuffer_create(&src, n));
    fill_bytes(src, msg, n);

    dtbuffer_t* chunk = NULL;

    /* header */
    DTERR_C(dtchunker_export(&chunker, src, &chunk));
    DTUNITTEST_ASSERT_NOT_NULL(chunk);
    DTUNITTEST_ASSERT_INT(chunk->length, ==, sizeof(dtchunker_header_t));

    /* payload (fits in one) */
    DTERR_C(dtchunker_export(&chunker, src, &chunk));
    DTUNITTEST_ASSERT_NOT_NULL(chunk);
    DTUNITTEST_ASSERT_INT(chunk->length, ==, n);
    DTUNITTEST_ASSERT_EQUAL_BYTES(chunk->payload, msg, (size_t)n);

    /* end */
    DTERR_C(dtchunker_export(&chunker, src, &chunk));
    DTUNITTEST_ASSERT_PTR(chunk, ==, NULL);

cleanup:
    dtbuffer_dispose(src);
    dtchunker_dispose(&chunker);
    return dterr;
}
//------------------------------------------------------------------------
static dterr_t*
test_dtcore_dtchunker_04_export_large_buffer_chunks(void)
{
    dterr_t* dterr = NULL;

    dtchunker_t chunker = (dtchunker_t){ 0 };
    const int32_t max_chunk = 8;
    DTERR_C(dtchunker_init(&chunker, max_chunk));

    dtbuffer_t* src = NULL;
    const char* msg = "ABCDEFGHIJKLMNOPQR"; /* 18 bytes */
    const int32_t n = (int32_t)strlen(msg);

    DTERR_C(dtbuffer_create(&src, n));
    fill_bytes(src, msg, n);

    dtbuffer_t* chunk = NULL;

    /* header */
    DTERR_C(dtchunker_export(&chunker, src, &chunk));
    DTUNITTEST_ASSERT_NOT_NULL(chunk);
    DTUNITTEST_ASSERT_INT(chunk->length, ==, sizeof(dtchunker_header_t));

    /* A..H */
    DTERR_C(dtchunker_export(&chunker, src, &chunk));
    DTUNITTEST_ASSERT_INT(chunk->length, ==, max_chunk);
    DTUNITTEST_ASSERT_EQUAL_BYTES(chunk->payload, "ABCDEFGH", (size_t)max_chunk);

    /* I..P */
    DTERR_C(dtchunker_export(&chunker, src, &chunk));
    DTUNITTEST_ASSERT_INT(chunk->length, ==, max_chunk);
    DTUNITTEST_ASSERT_EQUAL_BYTES(chunk->payload, "IJKLMNOP", (size_t)max_chunk);

    /* Q..R */
    DTERR_C(dtchunker_export(&chunker, src, &chunk));
    DTUNITTEST_ASSERT_INT(chunk->length, ==, 2);
    DTUNITTEST_ASSERT_EQUAL_BYTES(chunk->payload, "QR", 2);

    /* end */
    DTERR_C(dtchunker_export(&chunker, src, &chunk));
    DTUNITTEST_ASSERT_PTR(chunk, ==, NULL);

cleanup:
    dtbuffer_dispose(src);
    dtchunker_dispose(&chunker);
    return dterr;
}
//------------------------------------------------------------------------
static dterr_t*
test_dtcore_dtchunker_05_import_roundtrip(void)
{
    dterr_t* dterr = NULL;

    dtchunker_t exporter = (dtchunker_t){ 0 };
    dtchunker_t importer = (dtchunker_t){ 0 };
    dtbuffer_t *src = NULL, *chunk = NULL, *final = NULL;

    DTERR_C(dtchunker_init(&exporter, 10));
    DTERR_C(dtchunker_init(&importer, 10));

    const char* msg = "123456789ABCDEF"; /* 15 bytes */
    const int32_t n = (int32_t)strlen(msg);
    DTERR_C(dtbuffer_create(&src, n));
    fill_bytes(src, msg, n);

    do
    {
        DTERR_C(dtchunker_export(&exporter, src, &chunk));
        if (chunk)
        {
            DTERR_C(dtchunker_import(&importer, chunk, &final));
        }
    } while (chunk != NULL);

    DTUNITTEST_ASSERT_NOT_NULL(final);
    DTUNITTEST_ASSERT_INT(final->length, ==, n);
    DTUNITTEST_ASSERT_EQUAL_BYTES(final->payload, msg, (size_t)n);

cleanup:
    dtbuffer_dispose(src);
    dtbuffer_dispose(final);
    dtchunker_dispose(&exporter);
    dtchunker_dispose(&importer);
    return dterr;
}
//------------------------------------------------------------------------
static dterr_t*
test_dtcore_dtchunker_06_import_with_bad_magic(void)
{
    dterr_t* dterr = NULL;

    dtchunker_t importer = (dtchunker_t){ 0 };
    dtbuffer_t* final = NULL;
    DTERR_C(dtchunker_init(&importer, 8));

    dtchunker_header_t hdr = { .magic = 0xBAD0C0DE, .source_buffer_length = 5 };
    dtbuffer_t bad = { 0 };
    bad.payload = &hdr;
    bad.length = sizeof(hdr);

    dterr = dtchunker_import(&importer, &bad, &final);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_FAIL);

cleanup:
    dtchunker_dispose(&importer);
    return NULL;
}
//------------------------------------------------------------------------
static dterr_t*
test_dtcore_dtchunker_07_import_header_too_small(void)
{
    dterr_t* dterr = NULL;

    dtchunker_t importer = (dtchunker_t){ 0 };
    DTERR_C(dtchunker_init(&importer, 8));

    dtbuffer_t tiny = { 0 };
    dtchunker_header_t hdr = { .magic = DTCHUNKER_MAGIC, .source_buffer_length = 1 };
    tiny.payload = &hdr;
    tiny.length = sizeof(int32_t); /* only magic, no length */

    dtbuffer_t* final = NULL;
    dterr = dtchunker_import(&importer, &tiny, &final);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_FAIL);

cleanup:
    dtchunker_dispose(&importer);
    return NULL;
}
//------------------------------------------------------------------------
static dterr_t*
test_dtcore_dtchunker_08_mode_mismatch_errors(void)
{
    dterr_t* dterr = NULL;

    dtchunker_t a = (dtchunker_t){ 0 };
    dtchunker_t b = (dtchunker_t){ 0 };
    DTERR_C(dtchunker_init(&a, 8));
    DTERR_C(dtchunker_init(&b, 8));

    /* Put 'a' into export mode */
    dtbuffer_t* src = NULL;
    DTERR_C(dtbuffer_create(&src, 1));
    ((char*)src->payload)[0] = 'X';
    src->length = 1;

    dtbuffer_t* out = NULL;
    DTERR_C(dtchunker_export(&a, src, &out)); /* header exported; a is in export mode now */

    /* Now call import on 'a' (wrong mode) */
    dterr = dtchunker_import(&a, out, &src);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_STATE);

    /* Put 'b' into import mode by feeding a valid header */
    dtchunker_header_t hdr = { .magic = DTCHUNKER_MAGIC, .source_buffer_length = 1 };
    dtbuffer_t head = { 0 };
    head.payload = &hdr;
    head.length = sizeof(hdr);

    dtbuffer_t* final = NULL;
    DTERR_C(dtchunker_import(&b, &head, &final));
    DTUNITTEST_ASSERT_PTR(final, ==, NULL);

    /* Now call export on 'b' (wrong mode) */
    dterr = dtchunker_export(&b, src, &out);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_STATE);

cleanup:
    dtbuffer_dispose(src);
    dtchunker_dispose(&a);
    dtchunker_dispose(&b);
    return NULL;
}
//------------------------------------------------------------------------
static dterr_t*
test_dtcore_dtchunker_09_export_alias_lifetime(void)
{
    dterr_t* dterr = NULL;

    dtchunker_t tx = (dtchunker_t){ 0 };
    dtbuffer_t *src = NULL, *chunk1 = NULL, *chunk2 = NULL;

    const char* msg = "0123456789";
    const int32_t n = (int32_t)strlen(msg);

    DTERR_C(dtchunker_init(&tx, 8)); /* minimum chunk size per API */
    DTERR_C(dtbuffer_create(&src, n));
    fill_bytes(src, msg, n);

    /* header (alias 1) */
    DTERR_C(dtchunker_export(&tx, src, &chunk1));
    DTUNITTEST_ASSERT_NOT_NULL(chunk1);

    /* first payload (alias 2 - may reuse internal buffer) */
    DTERR_C(dtchunker_export(&tx, src, &chunk2));
    DTUNITTEST_ASSERT_NOT_NULL(chunk2);

    DTUNITTEST_ASSERT_INT(chunk1->length, ==, sizeof(dtchunker_header_t));
    DTUNITTEST_ASSERT_INT(chunk2->length, ==, 8);

cleanup:
    dtbuffer_dispose(src);
    dtchunker_dispose(&tx);
    return dterr;
}
//------------------------------------------------------------------------
static dterr_t*
test_dtcore_dtchunker_10_import_overflow_guard(void)
{
    dterr_t* dterr = NULL;

    dtchunker_t rx = (dtchunker_t){ 0 };
    DTERR_C(dtchunker_init(&rx, 8));

    /* header says total length = 3; then feed more than 3 bytes total */
    dtchunker_header_t hdr = { .magic = DTCHUNKER_MAGIC, .source_buffer_length = 3 };
    dtbuffer_t head = { 0 };
    head.payload = &hdr;
    head.length = sizeof(hdr);

    dtbuffer_t* final = NULL;
    DTERR_C(dtchunker_import(&rx, &head, &final));
    DTUNITTEST_ASSERT_PTR(final, ==, NULL);

    /* Now pass 4 bytes (exceeds) */
    char data[4] = { 'a', 'b', 'c', 'd' };
    dtbuffer_t over = { 0 };
    over.payload = data;
    over.length = 4;

    dterr = dtchunker_import(&rx, &over, &final);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_FAIL);

cleanup:
    dtchunker_dispose(&rx);
    return NULL;
}
//------------------------------------------------------------------------
void
test_dtcore_dtchunker(DTUNITTEST_SUITE_ARGS)
{
    /* Examples first */
    DTUNITTEST_RUN_TEST(test_dtcore_dtchunker_example_roundtrip_chunks);
    DTUNITTEST_RUN_TEST(test_dtcore_dtchunker_example_header_plus_payload_first_chunk);
    DTUNITTEST_RUN_TEST(test_dtcore_dtchunker_example_zero_length);

    /* Core tests */
    DTUNITTEST_RUN_TEST(test_dtcore_dtchunker_01_init_and_reset);
    DTUNITTEST_RUN_TEST(test_dtcore_dtchunker_02_init_bad_chunk_size);
    DTUNITTEST_RUN_TEST(test_dtcore_dtchunker_03_export_small_buffer);
    DTUNITTEST_RUN_TEST(test_dtcore_dtchunker_04_export_large_buffer_chunks);
    DTUNITTEST_RUN_TEST(test_dtcore_dtchunker_05_import_roundtrip);
    DTUNITTEST_RUN_TEST(test_dtcore_dtchunker_06_import_with_bad_magic);
    DTUNITTEST_RUN_TEST(test_dtcore_dtchunker_07_import_header_too_small);
    DTUNITTEST_RUN_TEST(test_dtcore_dtchunker_08_mode_mismatch_errors);
    DTUNITTEST_RUN_TEST(test_dtcore_dtchunker_09_export_alias_lifetime);
    DTUNITTEST_RUN_TEST(test_dtcore_dtchunker_10_import_overflow_guard);
}
