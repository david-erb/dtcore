#include <inttypes.h>
#include <stdint.h>

#include <dtcore/dterr.h>
#include <dtcore/dtvtable.h>

/* ------------------------------------------------------------------------ */
dterr_t*
dtvtable_set(dtvtable_registry_t* reg, int32_t model_number, void* vtable)
{
    dterr_t* dterr = NULL;

    /* Basic argument validation */
    DTERR_ASSERT_NOT_NULL(reg);
    DTERR_ASSERT_NOT_NULL(reg->model_numbers);
    DTERR_ASSERT_NOT_NULL(reg->vtables);

    if (reg->max_vtables <= 0)
    {
        dterr = dterr_new(
          DTERR_BADARG, DTERR_LOC, NULL, "vtable registry set called with non-positive capacity (%d)", reg->max_vtables);
        goto cleanup;
    }
    if (model_number == 0)
    {
        dterr = dterr_new(DTERR_BADARG, DTERR_LOC, NULL, "vtable registry cannot set model number zero");
        goto cleanup;
    }
    if (vtable == NULL)
    {
        dterr = dterr_new(DTERR_BADARG, DTERR_LOC, NULL, "vtable registry cannot set a null vtable pointer");
        goto cleanup;
    }

    /* Probe for existing mapping */
    void* existing = NULL;
    dterr = dtvtable_get(reg, model_number, &existing);
    if (dterr == NULL)
    {
        /* Already registered:
           - If same pointer: idempotent success.
           - If different pointer: fail explicitly to avoid silent remap. */
        if (existing == vtable)
            return NULL;

        dterr = dterr_new(DTERR_EXISTS,
          DTERR_LOC,
          NULL,
          "model number %" PRId32 " already registered to a different vtable "
          "(existing=%p, new=%p)",
          model_number,
          existing,
          vtable);
        goto cleanup;
    }
    else if (dterr->error_code == DTERR_NOTFOUND)
    {
        /* Expected: not present yet. Clear error and continue to insert. */
        dterr_dispose(dterr);
        dterr = NULL;
    }
    else
    {
        /* Unexpected probe error: propagate */
        goto cleanup;
    }

    /* Insert into first empty slot; scan entire table */
    for (int i = 0; i < reg->max_vtables; i++)
    {
        if (reg->model_numbers[i] == 0)
        {
            reg->model_numbers[i] = model_number;
            reg->vtables[i] = vtable;
            return NULL; /* success */
        }
    }

    dterr = dterr_new(DTERR_FAIL,
      DTERR_LOC,
      NULL,
      "vtable registry too full (%d) to handle model number %" PRId32,
      reg->max_vtables,
      model_number);

cleanup:
    return dterr;
}

/* ------------------------------------------------------------------------ */
dterr_t*
dtvtable_get(dtvtable_registry_t* reg, int32_t model_number, void** vtable)
{
    dterr_t* dterr = NULL;

    /* Basic argument validation */
    DTERR_ASSERT_NOT_NULL(reg);
    DTERR_ASSERT_NOT_NULL(reg->model_numbers);
    DTERR_ASSERT_NOT_NULL(reg->vtables);

    if (reg->max_vtables <= 0)
    {
        dterr = dterr_new(
          DTERR_BADARG, DTERR_LOC, NULL, "vtable registry get called with non-positive capacity (%d)", reg->max_vtables);
        goto cleanup;
    }
    if (model_number == 0)
    {
        dterr = dterr_new(DTERR_BADARG, DTERR_LOC, NULL, "vtable registry cannot get vtable for model number zero");
        goto cleanup;
    }

    /* Scan full table; do not rely on compact layout or sentinel holes */
    for (int i = 0; i < reg->max_vtables; i++)
    {
        if (reg->model_numbers[i] == model_number)
        {
            if (vtable != NULL)
                *vtable = reg->vtables[i];
            return NULL; /* success */
        }
    }

    dterr = dterr_new(DTERR_NOTFOUND, DTERR_LOC, NULL, "vtable registry does not contain model number %" PRId32, model_number);

cleanup:
    return dterr;
}
