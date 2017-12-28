/*
 * A TLV utility for esp32.
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "esp32-tlv8/tlv8.h"

#define min(a,b) ((a) < (b) ? (a) : (b))
#define TLV8_MAX_DATA_LEN       255

/***********************************************************************************************************
 * TLV8
 ***********************************************************************************************************
 * Private interface
 ***********************************************************************************************************/
static int tlv8_integer_len(uint32_t integer) {
    int len = 0;
    while (integer) {
        len++;
        integer = integer >> 8;
    }
    return len;
}

static int tlv8_encoded_size(tlv8 *tlv) {
    int num_fragments = tlv->len / TLV8_MAX_DATA_LEN + 1;
    int size = (num_fragments << 1) + tlv->len; // num_fragments * (1 + 1) + tlv->len - Always 1 for type + 1 for size
    return size;
}

static void tlv8_write_buffer_integer(int len, uint8_t type, int32_t integer, unsigned char *buffer) {
    buffer[0] = type;
    buffer[1] = len;
    while (len > 0) {
        buffer[2] = (unsigned char)integer;
        integer = integer >> 8;
        buffer++; len--;
    }
}

static void tlv8_write_buffer_data(int len, uint8_t type, const void *bytes, unsigned char *buffer) {
    int offset = 0;
    do {
        uint8_t size = min(len, TLV8_MAX_DATA_LEN);
        buffer[0] = type;
        buffer[1] = size;
        memcpy(buffer + 2, bytes + offset, size);
        buffer+= TLV8_MAX_DATA_LEN + 2;
        offset+= TLV8_MAX_DATA_LEN;
        len-= TLV8_MAX_DATA_LEN;
    } while (len > 0);
}

static void tlv8_write_buffer(tlv8 *tlv, unsigned char *buffer) {
    int len = tlv->len;
    switch (tlv->data.type) {
        case TLV8_DATA_TYPE_INTEGER:
            tlv8_write_buffer_integer(len, tlv->type, tlv->data.int32, buffer); break;
        case TLV8_DATA_TYPE_STRING:
        case TLV8_DATA_TYPE_BYTES:
            tlv8_write_buffer_data(len, tlv->type, tlv->data.bytes, buffer); break;
        default:
            break;
    }
}

static tlv8 *tlv8_new(uint8_t type) {
    tlv8 *tlv = (tlv8 *)malloc(sizeof(tlv8));
    if (tlv) {
        memset(tlv, 0, sizeof(tlv8));
        tlv->type = type;
    }
    return tlv;
}

/***********************************************************************************************************
 * Public interface
 ***********************************************************************************************************/
tlv8 *tlv8_new_with_integer(uint8_t type, int32_t integer) {
    tlv8 *tlv = tlv8_new(type);
    if (tlv) {
        tlv->data.type = TLV8_DATA_TYPE_INTEGER;
        tlv->data.int32 = integer;
        tlv->len = tlv8_integer_len((uint32_t)integer);
    }
    return tlv;
}

tlv8 *tlv8_new_with_string(uint8_t type, const char *string) {
    if (!string) {
        return NULL;
    }
    int str_len = strlen(string);
    tlv8 *tlv = tlv8_new(type);
    if (tlv && str_len) {
        tlv->data.type = TLV8_DATA_TYPE_STRING;
        tlv->data.string = strdup(string);
        if (!tlv->data.string) {
            tlv8_free(tlv);
            return NULL;
        }
        tlv->len = str_len;
    }
    return tlv;
}

tlv8 *tlv8_new_with_data(uint8_t type, void *data, int data_len) {
    if (!data || !data_len) {
        return NULL;
    }
    tlv8 *tlv = tlv8_new(type);
    if (tlv && data_len) {
        tlv->data.type = TLV8_DATA_TYPE_BYTES;
        tlv->data.bytes = malloc(data_len);
        if (!tlv->data.bytes) {
            tlv8_free(tlv);
            return NULL;
        }
        memcpy(tlv->data.bytes, data, data_len);
        tlv->len = data_len;
    }
    return tlv;
}

void tlv8_free(tlv8 *tlv) {
    if (tlv) {
        if (tlv->data.type > TLV8_DATA_TYPE_INTEGER && tlv->len) {
            free (tlv->data.bytes);
        }
        free(tlv);
    }
}

/***********************************************************************************************************
 * TLV Codec
 ***********************************************************************************************************
 * Private interface
 ***********************************************************************************************************/
static tlv8_codec *tlv8_codec_new(const uint8_t *data, int len) {
    tlv8_codec *codec = (tlv8_codec *)malloc(sizeof(tlv8_codec));
    if (codec) {
        memset(codec, 0, sizeof(tlv8_codec));
        if (len) {
            codec->data = malloc(len);
            if (!codec->data) {
                tlv8_codec_free(codec);
                return NULL;
            }
            memcpy(codec->data, data, len);
            codec->len = len;
        }
    }
    return codec;
}

static uint8_t tlv8_codec_decode_get_type_and_advance(tlv8_codec *codec) {
    return codec->data[codec->pos++];
}

static int tlv8_codec_decode_get_size_and_advance(tlv8_codec *codec) {
    return codec->data[codec->pos++];
}

static int tlv8_codec_decode_data_size(tlv8_codec *codec) {
    int size = 0;
    int start_pos = codec->pos;
    while (tlv8_codec_decode_has_next(codec)) {
        uint8_t type = tlv8_codec_decode_get_type_and_advance(codec);
        if (type != codec->type) {
            goto cleanup;
        }
        int len = tlv8_codec_decode_get_size_and_advance(codec);
        size+= len;
        // Jump to next TLV if any
        codec->pos+= len;
    }
cleanup:
    // Restore initial pointer to type of TLV
    codec->pos = start_pos;
    return size;
}

static tlv8 *tlv8_codec_decode_next_tlv_integer(tlv8_codec *codec) {
    tlv8 *tlv = NULL;
    if (tlv8_codec_decode_has_next(codec)) {
        uint32_t integer = 0;
        do {
            uint8_t type = tlv8_codec_decode_get_type_and_advance(codec);
            if (type != codec->type) {
                codec->pos--;
                break;
            }
            int size = tlv8_codec_decode_get_size_and_advance(codec);
            for (int i = 0; i < size; i++) {
                integer = integer | ((uint8_t)codec->data[codec->pos++] << (8 * i));
            }
        } while (0);
        tlv = tlv8_new_with_integer(codec->type, integer);
    }
    return tlv;
}

static tlv8 *tlv8_codec_decode_next_tlv_string(tlv8_codec *codec) {
    tlv8 *tlv = NULL;
    if (tlv8_codec_decode_has_next(codec)) {
        int size = tlv8_codec_decode_data_size(codec);
        char *str = (char *)malloc(sizeof(char) * size + 1);
        int offset = 0;
        do {
            uint8_t type = tlv8_codec_decode_get_type_and_advance(codec);
            if (type != codec->type) {
                codec->pos--;
                break;
            }
            int len = tlv8_codec_decode_get_size_and_advance(codec);
            memcpy(str + offset, codec->data + codec->pos, len);
            offset+= len;
            codec->pos+= len;
        } while (tlv8_codec_decode_has_next(codec));
        str[size] = '\0';
        tlv = tlv8_new_with_string(codec->type, str);
        free(str);
    }
    return tlv;
}

static tlv8 *tlv8_codec_decode_next_tlv_data(tlv8_codec *codec) {
    tlv8 *tlv = NULL;
    if (tlv8_codec_decode_has_next(codec)) {
        int size = tlv8_codec_decode_data_size(codec);
        char *data = (char *)malloc(sizeof(char) * size);
        int offset = 0;
        do {
            uint8_t type = tlv8_codec_decode_get_type_and_advance(codec);
            if (type != codec->type) {
                codec->pos--;
                break;
            }
            int len = tlv8_codec_decode_get_size_and_advance(codec);
            memcpy(data, codec->data + codec->pos, len);
            offset+= len;
            codec->pos+= len;
        } while (tlv8_codec_decode_has_next(codec));
        tlv = tlv8_new_with_data(codec->type, data, size);
        free(data);
    }
    return tlv;
}

/***********************************************************************************************************
 * Public interface
 ***********************************************************************************************************
 * Encoder
 ***********************************************************************************************************/
tlv8_codec *tlv8_codec_encoder_new() {
    return tlv8_codec_new(NULL, 0);
}

int tlv8_codec_encode(tlv8_codec *codec, tlv8 *tlv) {
    int size = tlv8_encoded_size(tlv);
    if (!codec->data) {
        codec->data = (uint8_t *)malloc(size);
    }
    else if (tlv->type == codec->type) {
        // Should not encode 2 consecutive TLVs with the same type
        return TLV8_ERR_TYPE_FORBIDDEN;
    }
    else {
        codec->data = (uint8_t *)realloc((void *)codec->data, codec->len + size);
    }
    if (!codec->data) {
        return TLV8_ERR_ALLOC_FAILED;
    }
    tlv8_write_buffer(tlv, (unsigned char*)codec->data + codec->len);
    codec->len+= size;
    codec->type = tlv->type;
    return TLV8_ERR_OK;
}

/***********************************************************************************************************
 * Decoder
 ***********************************************************************************************************/
tlv8_codec *tlv8_codec_decoder_new(const uint8_t *data, int len) {
    if (!data || !len) {
        return NULL;
    }
    return tlv8_codec_new(data, len);
}

int tlv8_codec_decode_has_next(tlv8_codec *codec) {
    return codec->pos < codec->len;
}

uint8_t tlv8_codec_decode_peek_type(tlv8_codec *codec) {
    codec->type = codec->data[codec->pos];
    return codec->type;
}

tlv8 *tlv8_codec_decode_next_tlv(tlv8_codec *codec, TLV8_DATA_TYPE type) {
    if (!tlv8_codec_decode_has_next(codec)) {
        printf("Oh oh...\n");
        return NULL;
    }
    switch (type) {
        case TLV8_DATA_TYPE_INTEGER:
            return tlv8_codec_decode_next_tlv_integer(codec);
        case TLV8_DATA_TYPE_STRING:
            return tlv8_codec_decode_next_tlv_string(codec);
        case TLV8_DATA_TYPE_BYTES:
            return tlv8_codec_decode_next_tlv_data(codec);
        default:
            // Nothing to return, we don't know that type
            return NULL;
    }
}

void tlv8_codec_free(tlv8_codec *codec) {
    if (codec) {
        if (codec->data) {
            free (codec->data);
        }
        free(codec);
    }
}
