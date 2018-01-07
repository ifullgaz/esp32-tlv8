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

#include "mbedtls/bignum.h"
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
    TLV8_DATA_TYPE_STRING,
    TLV8_DATA_TYPE_MPI
};

static void dump_buffer(buffer_t buffer, const char *description) {
    dump_data(buffer_get_data(buffer), buffer_get_length(buffer), description);
}

static void dump_codec(tlv8_encoder_t codec, const char *description) {
    buffer_t buffer = tlv8_encoder_get_data(codec);
    dump_buffer(buffer, description);
}

static const char *big_number_string = 
"FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E08"
"8A67CC74020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B"
"302B0A6DF25F14374FE1356D6D51C245E485B576625E7EC6F44C42E9"
"A637ED6B0BFF5CB6F406B7EDEE386BFB5A899FA5AE9F24117C4B1FE6"
"49286651ECE45B3DC2007CB8A163BF0598DA48361C55D39A69163FA8"
"FD24CF5F83655D23DCA3AD961C62F356208552BB9ED529077096966D"
"670C354E4ABC9804F1746C08CA18217C32905E462E36CE3BE39E772C"
"180E86039B2783A2EC07A28FB5C55DF06F4C52C9DE2BCBF695581718"
"3995497CEA956AE515D2261898FA051015728E5A8AAAC42DAD33170D"
"04507A33A85521ABDF1CBA64ECFB850458DBEF0A8AEA71575D060C7D"
"B3970F85A6E1E4C7ABF5AE8CDB0933D71E8C94E04A25619DCEE3D226"
"1AD2EE6BF12FFA06D98A0864D87602733EC86A64521F2B18177B200C"
"BBE117577A615D6C770988C0BAD946E208E24FA074E5AB3143DB5BFC"
"E0FD108E4B82D120A92108011A723C12A787E6D788719A10BDBA5B26"
"99C327186AF4E23C1A946834B6150BDA2583E9CA2AD44CE8DBBBC2DB"
"04DE8EF92E8EFC141FBECAA6287C59474E6BC05D99B2964FA090C3A2"
"233BA186515BE7ED1F612970CEE2D7AFB81BDD762170481CD0069127"
"D5B05AA993B4EA988D8FDDC186FFB7DC90A6C08F4DF435C934063199"
"FFFFFFFFFFFFFFFF";

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
    tlv8_free(tlv1);
    dump_codec(codec, "TLV 9");
    tlv8_encoder_free(codec);

    UTILS_DECLARE_MPI(bg);
    bg = utils_mpi_new();
    mbedtls_mpi_read_string(bg, 16, big_number_string);
    tlv1 = tlv8_new_with_mpi(11, bg);
    codec = tlv8_encoder_new(NULL);
    tlv8_encoder_encode(codec, tlv1);
    tlv8_encoder_encode(full_codec, tlv1);
    tlv8_free(tlv1);
    dump_codec(codec, "TLV 33");
    tlv8_encoder_free(codec);
    utils_mpi_free(bg);

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
                        tlv8_decoder_t inner_decoder = tlv8_decoder_new(tlv8_get_data_value(tlv));
                        uint8_t inner_type = tlv8_decoder_peek_type(inner_decoder);
                        TLV8_DATA_TYPE inner_data_type = data_types[inner_type];
                        tlv8_t inner_tlv = tlv8_decoder_decode(inner_decoder, inner_data_type);
                        printf("Inner string, Type: %d, value: %s\n", inner_type, tlv8_get_string_value(inner_tlv));
                        tlv8_free(inner_tlv);
                        tlv8_decoder_free(inner_decoder);
                    }
                    break;
                case TLV8_DATA_TYPE_MPI:
                    printf("MPI, Type: %d, data_type: %d, value:\n", type, data_type);
                    dump_big_number(tlv8_get_mpi_value(tlv), NULL);
                    break;
                default:
                    break;
            }
        }
        tlv8_free(tlv);
    }
    tlv8_decoder_free(decoder);

    array_t array = tlv8_decode(tlv8_encoder_get_data(full_codec), data_types);
    tlv8_encoder_free(full_codec);
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
                        array_t inner_array = tlv8_decode(tlv8_get_data_value(tlv), data_types);
                        tlv8_t inner_tlv = array_at(inner_array, 0);
                        uint8_t inner_type = tlv8_get_type(inner_tlv);
                        printf("Inner string, Type: %d, value: %s\n", inner_type, tlv8_get_string_value(inner_tlv));
                        array_free(inner_array);
                    }
                    break;
                case TLV8_DATA_TYPE_MPI:
                    printf("MPI, Type: %d, data_type: %d, value:\n", type, data_type);
                    dump_big_number(tlv8_get_mpi_value(tlv), NULL);
                    break;
                default:
                    break;
            }
        }        
    }
    array_free(array);
}
