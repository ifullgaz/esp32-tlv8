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

#ifndef TLV8_H
#define TLV8_H

#include <stdint.h>

#define TLV8_ERR_OK                     0
#define TLV8_ERR_UNKNOWN                -0x2001
#define TLV8_ERR_ALLOC_FAILED           -0x0010
#define TLV8_ERR_OUT_OF_MEMORY          TLV8_ERR_ALLOC_FAILED
#define TLV8_ERR_INVALID_TLV            -0x2002
#define TLV8_ERR_TYPE_FORBIDDEN         -0x2002
#define TLV8_ERR_MALFORMED_TLV          -0x2004
#define TLV8_ERR_INVALID_TYPE           -0x2006

typedef enum {
    TLV8_DATA_TYPE_INTEGER,
    TLV8_DATA_TYPE_STRING,
    TLV8_DATA_TYPE_BYTES
} TLV8_DATA_TYPE;

struct _tlv8;
typedef struct _tlv8 tlv8;
typedef struct _tlv8 {
    uint8_t             type;
    uint32_t            len;
    struct {
        TLV8_DATA_TYPE  type;
        union {
            int32_t     int32;
            void        *bytes;
            char        *string;
        };
    } data;
} tlv8;

typedef struct _tlv8_codec {
    uint8_t type;
    uint8_t *data;
    int len;
    int pos;
} tlv8_codec;

// TLV8 methods
// Create a new TLV8 structure from integer
tlv8 *tlv8_new_with_integer(uint8_t type, int32_t integer);
// Create a new TLV8 structure from string
tlv8 *tlv8_new_with_string(uint8_t type, const char *string);
// Create a new TLV8 structure from data
tlv8 *tlv8_new_with_data(uint8_t type, void *data, int data_len);
// Cleanup
void tlv8_free(tlv8 *tlv);

// TLV8 codec methods
// Create a new TLV8 codec encoder.
tlv8_codec *tlv8_codec_encoder_new();
// Encode a tlv on this codec
int tlv8_codec_encode(tlv8_codec *codec, tlv8 *tlv);
// Create a new TLV8 codec decoder.
tlv8_codec *tlv8_codec_decoder_new(const uint8_t *data, int len);
// Returns true if there are more tlvs to decode
int tlv8_codec_decode_has_next(tlv8_codec *codec);
// Returns the type of the next tlv
uint8_t tlv8_codec_decode_peek_type(tlv8_codec *codec);
// Returns a TLV of appropriate type form the next TLV data
tlv8 *tlv8_codec_decode_next_tlv(tlv8_codec *codec, TLV8_DATA_TYPE type);
// Cleanup
void tlv8_codec_free(tlv8_codec *codec);

#endif /* Include Guard */
#ifdef __cplusplus
}
#endif
