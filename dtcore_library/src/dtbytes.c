#include <stdint.h>
#include <stdio.h>

#include <dtcore/dtbytes.h>

// Composes input bytes into hex string grouped in 4-byte blocks
// Example: {0xF0,0x03,0x00,0x00,0x19,0x05,0x00,0x00}
// → "F0030000 19050000"
void
dtbytes_compose_hex(const void* source, int32_t source_length, char* output, int32_t output_length)
{
    if (!source || !output || source_length <= 0 || output_length <= 0)
        return;

    int written = 0;
    for (int i = 0; i < source_length; i++)
    {
        // Start a new group after every 4 bytes (except the first group)
        if (i > 0 && (i % 4 == 0))
        {
            if (written + 1 < output_length)
            {
                output[written++] = ' ';
            }
            else
            {
                break; // not enough space
            }
        }

        if (written + 2 < output_length)
        {
            unsigned char b = (unsigned char)((uint8_t*)source)[i];
            written += snprintf(output + written, output_length - written, "%02X", b);
        }
        else
        {
            break; // not enough space
        }
    }

    // Null-terminate
    if (written < output_length)
        output[written] = '\0';
    else
        output[output_length - 1] = '\0';
}
