#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dtledger.h>

void
dtledger_to_string(dtledger_t* ledger, char* buffer, int32_t buffer_size)
{
    if (ledger == NULL || buffer == NULL || buffer_size == 0)
    {
        return;
    }

    snprintf(buffer,
      buffer_size,
      "ledger %s: total=%" PRId32 " balance=%" PRId32 " high_water_mark=%" PRId32 " low_water_mark=%" PRId32
      " bytes_total=%" PRId32 " bytes_balance=%" PRId32 " bytes_high_water_mark=%" PRId32 " bytes_low_water_mark=%" PRId32,
      (ledger->class_name != NULL) ? ledger->class_name : "(null)",
      ledger->count_total,
      ledger->count_balance,
      ledger->count_high_water_mark,
      ledger->count_low_water_mark,
      ledger->bytes_total,
      ledger->bytes_balance,
      ledger->bytes_high_water_mark,
      ledger->bytes_low_water_mark);
}
