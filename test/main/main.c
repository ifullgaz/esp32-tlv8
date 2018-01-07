/*
 * A TLV utility for esp32.
 *
 * Copyright (c) 2018 Emmanuel Merali
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
#include <stdio.h>

#include "esp32-tlv8/tlv8.h"

static const uint8_t data_types[] = {
    0,
    TLV8_DATA_TYPE_INTEGER,
    TLV8_DATA_TYPE_INTEGER,
    TLV8_DATA_TYPE_INTEGER,
    TLV8_DATA_TYPE_INTEGER,
    TLV8_DATA_TYPE_INTEGER,
    TLV8_DATA_TYPE_INTEGER,
    TLV8_DATA_TYPE_STRING,
    TLV8_DATA_TYPE_STRING,
    TLV8_DATA_TYPE_BYTES,
    TLV8_DATA_TYPE_STRING
};

static void dump_buffer(buffer_t buffer, const char *description) {
    dump_data(buffer_get_data(buffer), buffer_get_length(buffer), description);
}

static void dump_codec(tlv8_encoder_t codec, const char *description) {
    buffer_t buffer = tlv8_encoder_get_data(codec);
    dump_buffer(buffer, description);
}

void app_main() {
    tlv8_t tlv1, tlv2;
    tlv8_encoder_t codec, full_codec;
    tlv8_decoder_t decoder;

    full_codec = tlv8_encoder_new(NULL);
    tlv1 = tlv8_new_with_integer(1, 0x00000000);
    codec = tlv8_encoder_new(NULL);
    tlv8_encoder_encode(codec, tlv1);
    tlv8_encoder_encode(full_codec, tlv1);
    tlv8_free(tlv1);
    dump_codec(codec, "TLV 1");
    tlv8_encoder_free(codec);

    tlv1 = tlv8_new_with_integer(2, 0x00000001);
    codec = tlv8_encoder_new(NULL);
    tlv8_encoder_encode(codec, tlv1);
    tlv8_encoder_encode(full_codec, tlv1);
    tlv8_free(tlv1);
    dump_codec(codec, "TLV 2");
    tlv8_encoder_free(codec);

    tlv1 = tlv8_new_with_integer(3, 0x00FF0000);
    codec = tlv8_encoder_new(NULL);
    tlv8_encoder_encode(codec, tlv1);
    tlv8_encoder_encode(full_codec, tlv1);
    tlv8_free(tlv1);
    dump_codec(codec, "TLV 3");
    tlv8_encoder_free(codec);

    tlv1 = tlv8_new_with_integer(4, 0x0000FFFF);
    codec = tlv8_encoder_new(NULL);
    tlv8_encoder_encode(codec, tlv1);
    tlv8_encoder_encode(full_codec, tlv1);
    tlv8_free(tlv1);
    dump_codec(codec, "TLV 4");
    tlv8_encoder_free(codec);

    tlv1 = tlv8_new_with_integer(5, 0xFFFF0000);
    codec = tlv8_encoder_new(NULL);
    tlv8_encoder_encode(codec, tlv1);
    tlv8_encoder_encode(full_codec, tlv1);
    tlv8_free(tlv1);
    dump_codec(codec, "TLV 5");
    tlv8_encoder_free(codec);

    tlv1 = tlv8_new_with_integer(6, 0xFFFFFFFF);
    codec = tlv8_encoder_new(NULL);
    tlv8_encoder_encode(codec, tlv1);
    tlv8_encoder_encode(full_codec, tlv1);
    tlv8_free(tlv1);
    dump_codec(codec, "TLV 6");
    tlv8_encoder_free(codec);

    tlv1 = tlv8_new_with_string(7, "Hello");
    codec = tlv8_encoder_new(NULL);
    tlv8_encoder_encode(codec, tlv1);
    tlv8_encoder_encode(full_codec, tlv1);
    tlv8_free(tlv1);
    dump_codec(codec, "TLV 7");
    tlv8_encoder_free(codec);

    tlv1 = tlv8_new_with_string(8, "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum.");
    codec = tlv8_encoder_new(NULL);
    tlv8_encoder_encode(codec, tlv1);
    tlv8_encoder_encode(full_codec, tlv1);
    tlv8_free(tlv1);
    dump_codec(codec, "TLV 8");
    tlv8_encoder_free(codec);

    tlv2 = tlv8_new_with_string(10, "Hello");
    codec = tlv8_encoder_new(NULL);
    tlv8_encoder_encode(codec, tlv2);
    dump_codec(codec, "TLV 10");
    tlv1 = tlv8_new_with_buffer(9, tlv8_encoder_get_data(codec));
    tlv8_encoder_free(codec);
    tlv8_free(tlv2);
    codec = tlv8_encoder_new(NULL);
    tlv8_encoder_encode(codec, tlv1);
    tlv8_encoder_encode(full_codec, tlv1);
    dump_codec(codec, "TLV 9");
    tlv8_free(tlv1);

    codec = tlv8_encoder_new(NULL);
    tlv1 = tlv8_new_with_string(16, "Hello");
    tlv8_encoder_encode(codec, tlv1);
    tlv8_free(tlv1);
    tlv1 = tlv8_new_with_string(17, "Hello");
    tlv8_encoder_encode(codec, tlv1);
    tlv8_free(tlv1);
    dump_codec(codec, "TLV 16 + TLV 17");
    tlv8_encoder_free(codec);

    // Two TLVs can't have the same type. Second one is ignored
    codec = tlv8_encoder_new(NULL);
    tlv1 = tlv8_new_with_string(32, "Hello");
    tlv8_encoder_encode(codec, tlv1);
    tlv8_free(tlv1);
    tlv1 = tlv8_new_with_string(32, "Hello");
    tlv8_encoder_encode(codec, tlv1);
    tlv8_free(tlv1);
    dump_codec(codec, "TLV 32");
    tlv8_encoder_free(codec);

    dump_codec(full_codec, "-------------- Full Codec");

    decoder = tlv8_decoder_new(tlv8_encoder_get_data(full_codec));
    // At that point, full_codec contains all the TLVs created so far
    while (tlv8_decoder_has_next(decoder)) {
        uint8_t type = tlv8_decoder_peek_type(decoder);
        TLV8_DATA_TYPE data_type = data_types[type];
        tlv8_t tlv = tlv8_decoder_decode(decoder, data_type);
        if (tlv) {
            switch(data_type) {
                case TLV8_DATA_TYPE_INTEGER:
                    printf("Integer, Type: %d, data_type: %d, value: %4llx\n", type, data_type, tlv8_get_integer_value(tlv));
                    break;
                case TLV8_DATA_TYPE_STRING:
                    printf("String, Type: %d, data_type: %d, value: %s\n", type, data_type, tlv8_get_string_value(tlv));
                    break;
                case TLV8_DATA_TYPE_BYTES:
                    printf("Bytes, Type: %d, data_type: %d, value:\n", type, data_type);
                    dump_buffer(tlv8_get_data_value(tlv), NULL);
                    if (type == 9) {
                        tlv8_decoder_t decoder = tlv8_decoder_new(tlv8_get_data_value(tlv));
                        uint8_t inner_type = tlv8_decoder_peek_type(decoder);
                        TLV8_DATA_TYPE inner_data_type = data_types[inner_type];
                        tlv8_t inner_tlv = tlv8_decoder_decode(decoder, inner_data_type);
                        printf("Inner string, Type: %d, value: %s\n", inner_type, tlv8_get_string_value(inner_tlv));
                        tlv8_free(inner_tlv);
                        tlv8_encoder_free(decoder);
                    }
                    break;
                default:
                    break;
            }
        }
        tlv8_free(tlv);
    }
    tlv8_decoder_free(decoder);

    array_t array = tlv8_decode(tlv8_encoder_get_data(full_codec), data_types);
    for (int i = 0; i < array_count(array); i++) {
        tlv8_t tlv = array_at(array, i);
        uint8_t type = tlv8_get_type(tlv);        
        TLV8_DATA_TYPE data_type = data_types[type];
        if (tlv) {
            switch(data_type) {
                case TLV8_DATA_TYPE_INTEGER:
                    printf("Integer, Type: %d, data_type: %d, value: %4llx\n", type, data_type, tlv8_get_integer_value(tlv));
                    break;
                case TLV8_DATA_TYPE_STRING:
                    printf("String, Type: %d, data_type: %d, value: %s\n", type, data_type, tlv8_get_string_value(tlv));
                    break;
                case TLV8_DATA_TYPE_BYTES:
                    printf("Bytes, Type: %d, data_type: %d, value:\n", type, data_type);
                    dump_buffer(tlv8_get_data_value(tlv), NULL);
                    if (type == 9) {
                        tlv8_decoder_t decoder = tlv8_decoder_new(tlv8_get_data_value(tlv));
                        uint8_t inner_type = tlv8_decoder_peek_type(decoder);
                        TLV8_DATA_TYPE inner_data_type = data_types[inner_type];
                        tlv8_t inner_tlv = tlv8_decoder_decode(decoder, inner_data_type);
                        printf("Inner string, Type: %d, value: %s\n", inner_type, tlv8_get_string_value(inner_tlv));
                        tlv8_free(inner_tlv);
                        tlv8_encoder_free(decoder);
                    }
                    break;
                default:
                    break;
            }
        }        
    }
    array_free(array);
    tlv8_encoder_free(full_codec);
}
