#pragma once

// Composes input bytes into hex string grouped in 4-byte blocks
// Example: {0xF0,0x03,0x00,0x00,0x19,0x05,0x00,0x00}
// → "F0030000 19050000"
extern void
dtbytes_compose_hex(const void* source,
                    int32_t source_length,
                    char* output,
                    int32_t output_length);

#if MARKDOWN_DOCUMENTATION
// clang-format off
// --8<-- [start:markdown-documentation]

# dtbytes

Hexadecimal formatting of byte sequences for inspection.

This group of functions provides byte-to-text hexadecimal formatting for diagnostic output.  
Usefor inspection and logging of raw byte buffers in C programs.  

## Mini-guide

- Formats a contiguous byte buffer into uppercase hexadecimal text.  
- Groups output into fixed 4-byte blocks by inserting a single space after every fourth byte.  
- Requires the caller to provide sufficient output space because truncation occurs silently when the buffer is too small.  

## Example

```c
uint8_t data[8] = { 0xF0, 0x03, 0x00, 0x00, 0x19, 0x05, 0x00, 0x00 };
char text[32];

dtbytes_compose_hex(data, 8, text, sizeof(text));
```


## Functions

### dtbytes_compose_hex

Formats a byte buffer as a grouped hexadecimal string.

Params:

> `const void* source` Pointer to the input byte buffer.  
> `int32_t source_length` Number of bytes to read from the source buffer.  
> `char* output` Pointer to the output character buffer.  
> `int32_t output_length` Size of the output buffer in bytes.  

Return: `void`  No return value.

<!-- FG_IDC: 5f41b023-580b-4376-8a7f-27bbef2302a4 | FG_UTC: 2026-01-17T09:40:46Z | FG_MAN=yes -->

// --8<-- [end:markdown-documentation]
// clang-format on
#endif
