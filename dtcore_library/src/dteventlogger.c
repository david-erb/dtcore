#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dtbuffer.h>
#include <dtcore/dterr.h>
#include <dtcore/dtlog.h>
#include <dtcore/dtstr.h>
#include <dtcore/dtwallclock.h>

#include <dtcore/dteventlogger.h>

#define TAG "dteventlogger"

// --------------------------------------------------------------------------------------------
dterr_t*
dteventlogger_init(dteventlogger_t* self, int32_t item_count, int32_t item_length)
{
    dterr_t* dterr = NULL;
    memset(self, 0, sizeof(*self));
    self->item_count = item_count;
    self->item_length = item_length;

    self->buffer_size = item_count * item_length;
    DTERR_C(dtbuffer_create(&self->buffer, self->buffer_size));
    memset(self->buffer->payload, 0, self->buffer_size);

    self->write_index = 0;

cleanup:
    if (dterr != NULL)
        dteventlogger_dispose(self);

    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dteventlogger_clone(dteventlogger_t* self, dteventlogger_t* clone)
{
    dterr_t* dterr = NULL;
    DTERR_C(dteventlogger_init(clone, self->item_count, self->item_length));

    clone->write_index = self->write_index;
    memcpy(clone->buffer->payload, self->buffer->payload, self->buffer_size);

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dteventlogger_append(dteventlogger_t* self, const void* item)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);
    DTERR_ASSERT_NOT_NULL(item);
    DTERR_ASSERT_NOT_NULL(self->buffer);

    if (self->item_count <= 0 || self->item_length <= 0)
    {
        dterr = dterr_new(DTERR_BADARG,
          DTERR_LOC,
          NULL,
          "dteventlogger_append: invalid internal state of the eventlogger (self->item_count=%" PRId32
          ", self->item_length=%" PRId32 ")",
          self->item_count,
          self->item_length);
        goto cleanup;
    }

    uint32_t item_index = self->write_index % self->item_count;
    char* write_pointer = (char*)self->buffer->payload + (item_index * self->item_length);
    memcpy((void*)write_pointer, item, self->item_length);
    self->write_index++;
cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dteventlogger_get_item(dteventlogger_t* self, int32_t index, void** item)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);
    DTERR_ASSERT_NOT_NULL(item);

    // asking for an item which is not in the buffer?
    if (index >= self->write_index || index >= self->item_count - 1)
    {
        *item = NULL;
        goto cleanup;
    }

    int32_t item_index;
    if (self->write_index < self->item_count)
        item_index = index;
    else
        item_index = (self->write_index + index + 1) % self->item_count;

    *item = (void*)((uint8_t*)self->buffer->payload + (item_index * self->item_length));

cleanup:
    return dterr;
}

// -------------------------------------------------------------------------------
dterr_t*
dteventlogger_log_item1(const char* tag,
  dteventlogger_t* eventlogger,
  const char* logger_label,
  const char* value1_label,
  const char* value2_label)
{
    dterr_t* dterr = NULL;
    char* str = NULL;

    DTERR_C(dteventlogger_printf_item1(eventlogger, logger_label, value1_label, value2_label, &str));

    dtlog_info(tag, "\n%s", str);

cleanup:
    dtstr_dispose(str);
    return dterr;
}

// -------------------------------------------------------------------------------
dterr_t*
dteventlogger_printf_item1( //
  dteventlogger_t* eventlogger,
  const char* logger_label,
  const char* value1_label,
  const char* value2_label,
  char** out_string)
{
    dterr_t* dterr = NULL;
    dteventlogger_t _clone = { 0 }, *clone = &_clone;

    char* str = *out_string;
    char* sep = "\n";

    if (eventlogger == NULL || logger_label == NULL || value1_label == NULL)
    {
        dterr = dterr_new(DTERR_BADARG,
          DTERR_LOC,
          NULL,
          "dteventlogger_log_item1: invalid parameters (eventlogger=%p, logger_label=%p, value1_label=%p)",
          eventlogger,
          logger_label,
          value1_label);
        goto cleanup;
    }

    DTERR_C(dteventlogger_clone(eventlogger, clone));

    const char indent1[] = "    ";
    const char indent2[] = "        ";

    str = dtstr_concat_format(str, sep, "%s%s:", indent1, logger_label);

    int32_t index = 0;
    dteventlogger_item1_t* item;
    dteventlogger_item1_t* last = NULL;
    while (true)
    {
        DTERR_C(dteventlogger_get_item(clone, index, (void**)&item));

        if (item == NULL)
            break;

        if (index == 0)
        {
            // compose header
            str = dtstr_concat_format(str,
              sep,
              "%s%3s  %-16s %-16s %-8s %-8s",
              indent2,
              "   ",
              "wall clock",
              "since last",
              value1_label,
              value2_label ? value2_label : "");
        }

        char wallclock_buffer[64];
        DTERR_C(dtwallclock_format_milliseconds_as_hhmmss_lll(item->timestamp, wallclock_buffer, sizeof(wallclock_buffer)));

        char sincelast_buffer[64];
        if (last == NULL)
        {
            sincelast_buffer[0] = '\0'; // no gap for the first item
        }
        else
        {
            uint64_t gap = item->timestamp - last->timestamp;
            DTERR_C(dtwallclock_format_milliseconds_as_hhmmss_lll(gap, sincelast_buffer, sizeof(sincelast_buffer)));
        }

        const char* format1 = "%s%3" PRId32 ". %-16s %-16s %8" PRId32;
        const char* format2 = "%s%3" PRId32 ". %-16s %-16s %8" PRId32 " %8" PRId32;

        const char* format = value2_label ? format2 : format1;
        str =
          dtstr_concat_format(str, sep, format, indent2, index, wallclock_buffer, sincelast_buffer, item->value1, item->value2);

        index++;
        last = item;
    }
    if (index == 0) // no items found
    {
        str = dtstr_concat_format(str, sep, "%sno items found in %s eventlogger", indent2, logger_label);
    }

cleanup:
    dteventlogger_dispose(clone);

    if (dterr == NULL)
    {
        // caller takes ownership of the string
        *out_string = str;
    }
    return dterr;
}

// --------------------------------------------------------------------------------------------
void
dteventlogger_dispose(dteventlogger_t* self)
{
    if (self == NULL)
        return;

    if (self->buffer != NULL)
    {
        dtbuffer_dispose(self->buffer);
    }

    memset(self, 0, sizeof(*self));
}
