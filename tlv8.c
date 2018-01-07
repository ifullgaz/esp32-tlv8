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
#include <stdarg.h>
#include "esp32-tlv8/tlv8.h"

#define min(a,b) ((a) < (b) ? (a) : (b))
#define TLV8_MAX_DATA_LEN       255

struct _tlv8 {
    uint8_t             type;
    uint32_t            len;
    struct {
        TLV8_DATA_TYPE  type;
        union {
            uint64_t    uint64;
            buffer_t    data;
            mbedtls_mpi *mpi;
        };
    } data;
};

struct _tlv8_encoder {
    uint8_t type;
    buffer_t data;
};

struct _tlv8_decoder {
    uint8_t type;
    buffer_t buffer;
    int len;
    int pos;
    const unsigned char *data;
};

/***********************************************************************************************************
 * TLV8
 ***********************************************************************************************************
 * Private interface
 ***********************************************************************************************************/
static int tlv8_integer_len(uint64_t integer) {
    int len = 0;
    while (integer) {
        len++;
        integer = integer >> 8;
    }
    len-= (integer);
    return len;
}

static tlv8_t tlv8_new(uint8_t type) {
    tlv8_t tlv = (tlv8_t)malloc(sizeof(struct _tlv8));
    if (tlv) {
        memset(tlv, 0, sizeof(struct _tlv8));
        tlv->type = type;
    }
    return tlv;
}

/***********************************************************************************************************
 * Public interface
 ***********************************************************************************************************/
tlv8_t tlv8_new_with_integer(uint8_t type, uint64_t integer) {
    tlv8_t tlv = tlv8_new(type);
    if (tlv) {
        tlv->data.type = TLV8_DATA_TYPE_INTEGER;
        tlv->data.uint64 = integer;
        tlv->len = tlv8_integer_len(integer);
    }
    return tlv;
}

tlv8_t tlv8_new_with_data(uint8_t type, void *data, int data_len) {
    if (!data || !data_len) {
        return NULL;
    }
    tlv8_t tlv = tlv8_new(type);
    tlv->data.type = TLV8_DATA_TYPE_BYTES;
    if (tlv && data_len) {
        tlv->data.data = buffer_new(data_len);
        if (!tlv->data.data) {
            tlv8_free(tlv);
            return NULL;
        }
        buffer_append(tlv->data.data, data, data_len);
        tlv->len = data_len;
    }
    return tlv;
}

tlv8_t tlv8_new_with_string(uint8_t type, const char *string) {
    if (!string) {
        return NULL;
    }
    int str_len = strlen(string);
    tlv8_t tlv = tlv8_new_with_data(type, (void *)string, str_len);
    if (tlv) {
        tlv->data.type = TLV8_DATA_TYPE_STRING;
    }
    return tlv;
}

tlv8_t tlv8_new_with_buffer(uint8_t type, buffer_t data) {
    if (!data) {
        return NULL;
    }
    return tlv8_new_with_data(type, (void *)buffer_get_data(data), buffer_get_length(data));    
}

// Create a new TLV8 structure from mpi (type TLV8_DATA_TYPE_MPI)
tlv8_t tlv8_new_with_mpi(uint8_t type, mbedtls_mpi *mpi) {
    mbedtls_mpi *cpy;
    if (!(cpy = (mbedtls_mpi *)malloc(sizeof(mbedtls_mpi)))) {
        return NULL;
    }
    mbedtls_mpi_init(cpy);
    if (mbedtls_mpi_copy(cpy, mpi)) {
        mbedtls_mpi_free(cpy);
        return NULL;
    }
    tlv8_t tlv = tlv8_new(type);
    if (tlv) {
        tlv->data.type = TLV8_DATA_TYPE_MPI;
        tlv->data.mpi = cpy;
        tlv->len = mbedtls_mpi_size(tlv->data.mpi);
    }
    return tlv;
}

void tlv8_free(void *t) {
    tlv8_t tlv = (tlv8_t)t;
    if (tlv) {
        if (tlv->data.type == TLV8_DATA_TYPE_MPI) {
            mbedtls_mpi_free(tlv->data.mpi);
        }
        else if (tlv->data.type != TLV8_DATA_TYPE_INTEGER) {
            buffer_free(tlv->data.data);
        }
        free(t);
    }
}

uint8_t tlv8_get_type(tlv8_t tlv) {
    return tlv->type;
}

uint64_t tlv8_get_integer_value(tlv8_t tlv) {
    return tlv->data.uint64;
}

const char *tlv8_get_string_value(tlv8_t tlv) {
    return (const char *)buffer_get_data(tlv->data.data);
}
buffer_t tlv8_get_data_value(tlv8_t tlv) {
    return (buffer_t)tlv->data.data;
}

mbedtls_mpi *tlv8_get_mpi_value(tlv8_t tlv) {
    return tlv->data.mpi;
}

/***********************************************************************************************************
 * TLV Encoder
 ***********************************************************************************************************
 * Private interface
 ***********************************************************************************************************/
static int tlv8_encoded_size(int len) {
    int num_fragments = len / TLV8_MAX_DATA_LEN + 1;
    int size = (num_fragments << 1) + len; // 2 bytes * num_fragments + tlv->len - Always 1 for type + 1 for size
    return size;
}

static void tlv8_encoder_write_buffer_integer(tlv8_encoder_t codec, tlv8_t tlv) {
    uint8_t len = tlv->len;
    uint64_t integer = tlv->data.uint64;
    buffer_append(codec->data, &(tlv->type), 1);
    buffer_append(codec->data, &len, 1);
    while (len > 0) {
        // Little endian
        unsigned char val = (unsigned char)integer;
        buffer_append(codec->data, &val, 1);
        integer = integer >> 8;
        len--;
    }
}

static void tlv8_encoder_write_buffer_data(tlv8_encoder_t codec, tlv8_t tlv) {
    int len = tlv->len;
    const unsigned char *data = buffer_get_data(tlv->data.data);
    int offset = 0;
    do {
        uint8_t size = min(len, TLV8_MAX_DATA_LEN);
        buffer_append(codec->data, &(tlv->type), 1);
        buffer_append(codec->data, &size, 1);
        buffer_append(codec->data, (data + offset), size);
        offset+= TLV8_MAX_DATA_LEN;
        len-= TLV8_MAX_DATA_LEN;
    } while (len > 0);
}

static void tlv8_encoder_write_buffer_mpi(tlv8_encoder_t codec, tlv8_t tlv) {
    int len = tlv->len;
    unsigned char bin[len];
    memset(bin, 0, len);
    mbedtls_mpi_write_binary(tlv->data.mpi, bin, len);
    int offset = 0;
    do {
        uint8_t size = min(len, TLV8_MAX_DATA_LEN);
        buffer_append(codec->data, &(tlv->type), 1);
        buffer_append(codec->data, &size, 1);
        buffer_append(codec->data, (bin + offset), size);
        offset+= TLV8_MAX_DATA_LEN;
        len-= TLV8_MAX_DATA_LEN;
    } while (len > 0);
}

static void tlv8_encoder_write_buffer(tlv8_encoder_t codec, tlv8_t tlv) {
    switch (tlv->data.type) {
        case TLV8_DATA_TYPE_INTEGER:
            tlv8_encoder_write_buffer_integer(codec, tlv); break;
        case TLV8_DATA_TYPE_STRING:
        case TLV8_DATA_TYPE_BYTES:
            tlv8_encoder_write_buffer_data(codec, tlv); break;
        case TLV8_DATA_TYPE_MPI:
            tlv8_encoder_write_buffer_mpi(codec, tlv); break;
        default:
            break;
    }
}

/***********************************************************************************************************
 * Public interface
 ***********************************************************************************************************/
tlv8_encoder_t tlv8_encoder_new(buffer_t buffer) {
    tlv8_encoder_t codec = (tlv8_encoder_t)malloc(sizeof(struct _tlv8_encoder));
    if (codec) {
        memset(codec, 0, sizeof(struct _tlv8_encoder));
        codec->data = buffer;
    }
    return codec;
}

int tlv8_encoder_encode(tlv8_encoder_t codec, tlv8_t tlv) {
    int size = tlv8_encoded_size(tlv->len);
    if (!codec->data) {
        codec->data = buffer_new(size);
        if (!codec->data) {
            return TLV8_ERR_ALLOC_FAILED;
        }
    }
    else if (tlv->type == codec->type) {
        // Should not encode 2 consecutive TLVs with the same type
        return TLV8_ERR_TYPE_FORBIDDEN;
    }
    else if (buffer_ensure_available(codec->data, size) != UTILS_ERR_OK) {
            return TLV8_ERR_ALLOC_FAILED;
        }
    tlv8_encoder_write_buffer(codec, tlv);
    codec->type = tlv->type;
    return TLV8_ERR_OK;
}

// Get data buffer
buffer_t tlv8_encoder_get_data(tlv8_encoder_t codec) {
    return codec->data;
}

// Detach data buffer
buffer_t tlv8_encoder_detach_data(tlv8_encoder_t codec) {
    buffer_t data = codec->data;
    codec->data = NULL;
    return data;
}

// Cleanup
void tlv8_encoder_free(void *c) {
    tlv8_encoder_t codec = (tlv8_encoder_t)c;
    if (codec) {
        buffer_free(codec->data);
        free(c);
    }
}
/***********************************************************************************************************
 * TLV Decoder
 ***********************************************************************************************************
 * Private interface
 ***********************************************************************************************************/
static uint8_t tlv8_decoder_get_type_and_advance(tlv8_decoder_t codec) {
    return codec->data[codec->pos++];
}

static uint8_t tlv8_decoder_get_size_and_advance(tlv8_decoder_t codec) {
    return codec->data[codec->pos++];
}

static int tlv8_decoder_data_size(tlv8_decoder_t codec) {
    int size = 0;
    int start_pos = codec->pos;
    do {
        uint8_t type = tlv8_decoder_get_type_and_advance(codec);
        if (type != codec->type) {
            goto cleanup;
        }
        int len = tlv8_decoder_get_size_and_advance(codec);
        size+= len;
        // Jump to next TLV if any
        codec->pos+= len;
    } while (tlv8_decoder_has_next(codec));
cleanup:
    // Restore initial pointer to type of TLV
    codec->pos = start_pos;
    return size;
}

static tlv8_t tlv8_decoder_next_tlv_integer(tlv8_decoder_t codec) {
    uint64_t integer = 0;
    tlv8_decoder_get_type_and_advance(codec);
    int size = tlv8_decoder_get_size_and_advance(codec);
    for (int i = 0; i < size; i++) {
        uint8_t byte = codec->data[codec->pos++];
        integer = integer | ((uint64_t)byte << (8 * i));
    }
    return tlv8_new_with_integer(codec->type, integer);
}

static tlv8_t tlv8_decoder_next_tlv_data(tlv8_decoder_t codec) {
    tlv8_t tlv = NULL;
    int size = tlv8_decoder_data_size(codec);
    int num_fragments = size / TLV8_MAX_DATA_LEN + 1;
    buffer_t data = buffer_new(size);
    for (int i = 0; i < num_fragments; i++) {
        tlv8_decoder_get_type_and_advance(codec);
        uint8_t len = tlv8_decoder_get_size_and_advance(codec);
        buffer_append(data, codec->data + codec->pos, len);
        codec->pos+= len;
    };
    tlv = tlv8_new(codec->type);
    tlv->data.type = TLV8_DATA_TYPE_BYTES;
    tlv->data.data = data;
    tlv->len = size;
    return tlv;
}

static tlv8_t tlv8_decoder_next_tlv_string(tlv8_decoder_t codec) {
    tlv8_t tlv = tlv8_decoder_next_tlv_data(codec);
    if (tlv) {
        tlv->data.type = TLV8_DATA_TYPE_STRING;
    }
    return tlv;
}

static tlv8_t tlv8_decoder_next_tlv_mpi(tlv8_decoder_t codec) {
    tlv8_t tlv = NULL;
    int size = tlv8_decoder_data_size(codec);
    int num_fragments = size / TLV8_MAX_DATA_LEN + 1;
    char data[size];
    int offset = 0;
    for (int i = 0; i < num_fragments; i++) {
        tlv8_decoder_get_type_and_advance(codec);
        uint8_t len = tlv8_decoder_get_size_and_advance(codec);
        memcpy(data + offset, codec->data + codec->pos, len);
        offset+= len;
        codec->pos+= len;
    };
    UTILS_DECLARE_MPI(mpi);
    if (!(mpi = utils_mpi_new())) {
        return NULL;
    }
    if (mbedtls_mpi_read_binary(mpi, (const unsigned char *)data, size)) {
        mbedtls_mpi_free(mpi);
        return NULL;
    }
    
    tlv = tlv8_new(codec->type);
    tlv->data.type = TLV8_DATA_TYPE_MPI;
    tlv->data.mpi = mpi;
    tlv->len = size;
    return tlv;
}

/***********************************************************************************************************
 * Public interface
 ***********************************************************************************************************/
tlv8_decoder_t tlv8_decoder_new(buffer_t data) {
    if (!data) {
        return NULL;
    }
    tlv8_decoder_t codec = (tlv8_decoder_t)malloc(sizeof(struct _tlv8_decoder));
    if (codec) {
        memset(codec, 0, sizeof(struct _tlv8_decoder));
        codec->buffer = data;
        codec->data = (const unsigned char *)buffer_get_data(data);
        codec->len = buffer_get_length(data);
    }
    return codec;
}

// Detach data buffer
buffer_t tlv8_decoder_detach_data(tlv8_decoder_t codec) {
    buffer_t data = codec->buffer;
    codec->buffer = NULL;
    return data;
}

int tlv8_decoder_has_next(tlv8_decoder_t codec) {
    return codec->pos < codec->len;
}

uint8_t tlv8_decoder_peek_type(tlv8_decoder_t codec) {
    codec->type = codec->data[codec->pos];
    return codec->type;
}

tlv8_t tlv8_decoder_decode(tlv8_decoder_t codec, TLV8_DATA_TYPE type) {
    if (!tlv8_decoder_has_next(codec)) {
        return NULL;
    }
    switch (type) {
        case TLV8_DATA_TYPE_INTEGER:
            return tlv8_decoder_next_tlv_integer(codec);
        case TLV8_DATA_TYPE_STRING:
            return tlv8_decoder_next_tlv_string(codec);
        case TLV8_DATA_TYPE_BYTES:
            return tlv8_decoder_next_tlv_data(codec);
        case TLV8_DATA_TYPE_MPI:
            return tlv8_decoder_next_tlv_mpi(codec);
        default:
            // Nothing to return, we don't know that type
            return NULL;
    }
}

void tlv8_decoder_free(void *c) {
    tlv8_decoder_t codec = (tlv8_decoder_t)c;
    if (codec) {
        free(c);
    }
}

/***********************************************************************************************************
 * Convenience methods
 ***********************************************************************************************************/
buffer_t tlv8_encode(const array_t array) {
    return tlv8_encode_array(array);
}

buffer_t tlv8_encode_array(const array_t array) {
    tlv8_encoder_t codec = tlv8_encoder_new(NULL);
    for (int i = 0; i < array_count(array); i++) {
        tlv8_t tlv = (tlv8_t)array_at(array, i);
        tlv8_encoder_encode(codec, tlv);
    }
    return tlv8_encoder_detach_data(codec);
}

buffer_t tlv8_encode_list(int count, ...) {
    va_list list;
    va_start(list, count);
    tlv8_encoder_t codec = tlv8_encoder_new(NULL);
    for (int i = 0; i < count; i++) {
        tlv8_t tlv = va_arg(list, tlv8_t);
        tlv8_encoder_encode(codec, tlv);
        tlv8_free(tlv);
    }
    va_end(list);
    return tlv8_encoder_detach_data(codec);
}

array_t tlv8_decode(const buffer_t buffer, const uint8_t *mapping) {
    array_t array = array_new(tlv8_free);
    tlv8_decoder_t decoder = tlv8_decoder_new(buffer);
    while (tlv8_decoder_has_next(decoder)) {
        uint8_t type = tlv8_decoder_peek_type(decoder);
        TLV8_DATA_TYPE data_type = mapping[type];
        tlv8_t tlv = tlv8_decoder_decode(decoder, data_type);
        if (tlv) {
            array_push(array, tlv);
        }
    }
    tlv8_decoder_detach_data(decoder);
    tlv8_decoder_free(decoder);
    return array;
}

tlv8_t tlv8_tlv_of_type(array_t tlvs, uint8_t type) {
    for (int i = 0; i < array_count(tlvs); i++) {
        tlv8_t tlv = array_at(tlvs, i);
        if (tlv8_get_type(tlv) == type) {
            return tlv;
        }
    }
    return NULL;
}