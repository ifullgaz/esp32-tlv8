/*
 * A TLV8 utility for esp32.
 *
 * Copyright (c) 2017 Emmanuel Merali
 * https://github.com/ifullgaz/esp32-tlv8
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _TLV8_H
#define _TLV8_H

#include <stdint.h>
#include "mbedtls/bignum.h"
#include "esp32-utils/utils.h"

#define TLV8_VERSION_MAJ                 0
#define TLV8_VERSION_MIN                 3
#define TLV8_VERSION_REV                 2
#define TLV8_VERSION_STR                 "0.3.2"
#define TLV8_VERSION_CHK(maj, min)       ((maj==TLV8_VERSION_MAJ) && (min<=TLV8_VERSION_MIN))

#define TLV8_ERR_OK                     0
#define TLV8_ERR_INVALID_TLV            -0x0002
#define TLV8_ERR_TYPE_FORBIDDEN         -0x0004
#define TLV8_ERR_MALFORMED_TLV          -0x0005
#define TLV8_ERR_INVALID_TYPE           -0x0008
#define TLV8_ERR_ALLOC_FAILED           -0x000A
#define TLV8_ERR_OUT_OF_MEMORY          TLV8_ERR_ALLOC_FAILED

#define ESP32_TLV8_CHK(f) \
if (( ret = f ) != TLV8_ERR_OK) { \
    goto cleanup; \
}

typedef enum {
    TLV8_DATA_TYPE_SEPARATOR,
    TLV8_DATA_TYPE_INTEGER,
    TLV8_DATA_TYPE_STRING,
    TLV8_DATA_TYPE_BYTES,
    TLV8_DATA_TYPE_MPI
} TLV8_DATA_TYPE;

struct _tlv8;
typedef struct _tlv8 *tlv8_t;
typedef struct _tlv8_encoder *tlv8_encoder_t;
typedef struct _tlv8_decoder *tlv8_decoder_t;

// TLV8 methods
// Create a new TLV8 separator
tlv8_t tlv8_new_separator(uint8_t type);
// Create a new TLV8 structure from integer
tlv8_t tlv8_new_with_integer(uint8_t type, uint64_t integer);
// Create a new TLV8 structure from string
tlv8_t tlv8_new_with_string(uint8_t type, const char *string);
// Create a new TLV8 structure from data
tlv8_t tlv8_new_with_data(uint8_t type, void *data, int data_len);
// Create a new TLV8 structure from buffer (type TLV8_DATA_TYPE_BYTES)
tlv8_t tlv8_new_with_buffer(uint8_t type, buffer_t data);
// Create a new TLV8 structure from mpi (type TLV8_DATA_TYPE_MPI)
tlv8_t tlv8_new_with_mpi(uint8_t type, mbedtls_mpi *mpi);
// Getters
uint8_t tlv8_get_type(tlv8_t tlv);
uint64_t tlv8_get_integer_value(tlv8_t tlv);
const char *tlv8_get_string_value(tlv8_t tlv);
buffer_t tlv8_get_data_value(tlv8_t tlv);
mbedtls_mpi *tlv8_get_mpi_value(tlv8_t tlv);
// Cleanup
void tlv8_free(void *tlv);

// TLV8 codec methods
// Create a new TLV8 codec encoder.
tlv8_encoder_t tlv8_encoder_new(buffer_t buffer);
// Add and encode a tlv on this codec
int tlv8_encoder_encode(tlv8_encoder_t codec, tlv8_t tlv);
// Get data buffer
buffer_t tlv8_encoder_get_data(tlv8_encoder_t codec);
// Detach data buffer
buffer_t tlv8_encoder_detach_data(tlv8_encoder_t codec);
// Cleanup
void tlv8_encoder_free(void *codec);

// Create a new TLV8 codec decoder.
tlv8_decoder_t tlv8_decoder_new(buffer_t data);
// Detach data buffer (in case it's in use elsewhere) before free
buffer_t tlv8_decoder_detach_data(tlv8_decoder_t codec);
// Returns true if there are more tlvs to decode
int tlv8_decoder_has_next(tlv8_decoder_t codec);
// Returns the type of the next tlv
uint8_t tlv8_decoder_peek_type(tlv8_decoder_t codec);
// Returns a TLV of appropriate type form the next TLV data
tlv8_t tlv8_decoder_decode(tlv8_decoder_t codec, TLV8_DATA_TYPE type);
// Cleanup
void tlv8_decoder_free(void *codec);

// Convenience methods
// Deprecated, use tlv8_encode_array
buffer_t tlv8_encode(const array_t array);
// Encode tlvs in an array
buffer_t tlv8_encode_array(const array_t array);
// Encode tlvs as a list (will free tlvs after encoding)
buffer_t tlv8_encode_list(int count, ...);
array_t tlv8_decode(const buffer_t buffer, const TLV8_DATA_TYPE *mapping);
tlv8_t tlv8_tlv_of_type(array_t tlvs, uint8_t type);

#endif // _TLV8_H
#ifdef __cplusplus
}
#endif
