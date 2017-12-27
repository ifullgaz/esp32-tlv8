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

void dump_data(const void* data, size_t size, const char *description) {
	char ascii[17];
	size_t i, j;
	ascii[16] = '\0';
	if (description) {
		printf("%s\n", description);
	}
	if (!data || !size) {
		printf("NULL\n");
		return;
	}
	for (i = 0; i < size; ++i) {
		printf("%02X ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char*)data)[i];
		} else {
			ascii[i % 16] = '.';
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			printf(" ");
			if ((i+1) % 16 == 0) {
				printf("|  %s \n", ascii);
			} else if (i+1 == size) {
				ascii[(i+1) % 16] = '\0';
				if ((i+1) % 16 <= 8) {
					printf(" ");
				}
				for (j = (i+1) % 16; j < 16; ++j) {
					printf("   ");
				}
				printf("|  %s \n", ascii);
			}
		}
	}
	if (description) {
		printf("End\n");
	}
}

static const uint8_t data_types[] = {
    TLV8_DATA_TYPE_INTEGER, 0, TLV8_DATA_TYPE_INTEGER, 0, TLV8_DATA_TYPE_INTEGER, TLV8_DATA_TYPE_INTEGER, TLV8_DATA_TYPE_INTEGER, TLV8_DATA_TYPE_STRING,
    TLV8_DATA_TYPE_STRING, TLV8_DATA_TYPE_BYTES, TLV8_DATA_TYPE_STRING
};

void app_main() {
    tlv8 *tlv1, *tlv2;
    tlv8_codec *codec, *full_codec;

    full_codec = tlv8_codec_encoder_new();
    tlv1 = tlv8_new_with_integer(1, 0x00000000);
    codec = tlv8_codec_encoder_new();
    tlv8_codec_encode(codec, tlv1);
    tlv8_codec_encode(full_codec, tlv1);
    tlv8_free(tlv1);
    dump_data(codec->data, codec->len, "codec");
    tlv8_codec_free(codec);

    tlv1 = tlv8_new_with_integer(2, 0x00000001);
    codec = tlv8_codec_encoder_new();
    tlv8_codec_encode(codec, tlv1);
    tlv8_codec_encode(full_codec, tlv1);
    tlv8_free(tlv1);
    dump_data(codec->data, codec->len, "codec");
    tlv8_codec_free(codec);

    tlv1 = tlv8_new_with_integer(3, 0x00FF0000);
    codec = tlv8_codec_encoder_new();
    tlv8_codec_encode(codec, tlv1);
    tlv8_codec_encode(full_codec, tlv1);
    tlv8_free(tlv1);
    dump_data(codec->data, codec->len, "codec");
    tlv8_codec_free(codec);

    tlv1 = tlv8_new_with_integer(4, 0x0000FFFF);
    codec = tlv8_codec_encoder_new();
    tlv8_codec_encode(codec, tlv1);
    tlv8_codec_encode(full_codec, tlv1);
    tlv8_free(tlv1);
    dump_data(codec->data, codec->len, "codec");
    tlv8_codec_free(codec);

    tlv1 = tlv8_new_with_integer(5, 0xFFFF0000);
    codec = tlv8_codec_encoder_new();
    tlv8_codec_encode(codec, tlv1);
    tlv8_codec_encode(full_codec, tlv1);
    tlv8_free(tlv1);
    dump_data(codec->data, codec->len, "codec");
    tlv8_codec_free(codec);

    tlv1 = tlv8_new_with_integer(6, 0xFFFFFFFF);
    codec = tlv8_codec_encoder_new();
    tlv8_codec_encode(codec, tlv1);
    tlv8_codec_encode(full_codec, tlv1);
    tlv8_free(tlv1);
    dump_data(codec->data, codec->len, "codec");
    tlv8_codec_free(codec);

    tlv1 = tlv8_new_with_string(7, "Hello");
    codec = tlv8_codec_encoder_new();
    tlv8_codec_encode(codec, tlv1);
    tlv8_codec_encode(full_codec, tlv1);
    tlv8_free(tlv1);
    dump_data(codec->data, codec->len, "codec");
    tlv8_codec_free(codec);

    tlv1 = tlv8_new_with_string(8, "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum.");
    codec = tlv8_codec_encoder_new();
    tlv8_codec_encode(codec, tlv1);
    tlv8_codec_encode(full_codec, tlv1);
    tlv8_free(tlv1);
    dump_data(codec->data, codec->len, "codec");
    tlv8_codec_free(codec);

    tlv2 = tlv8_new_with_string(10, "Hello");
    codec = tlv8_codec_encoder_new();
    tlv8_codec_encode(codec, tlv2);
    dump_data(codec->data, codec->len, "codec");
    tlv1 = tlv8_new_with_data(9, codec->data, codec->len);
    tlv8_codec_free(codec);
    tlv8_free(tlv2);
    codec = tlv8_codec_encoder_new();
    tlv8_codec_encode(codec, tlv1);
    tlv8_codec_encode(full_codec, tlv1);
    dump_data(codec->data, codec->len, "codec");
    tlv8_free(tlv1);

    codec = tlv8_codec_encoder_new();
    tlv1 = tlv8_new_with_string(16, "Hello");
    tlv8_codec_encode(codec, tlv1);
    tlv8_free(tlv1);
    tlv1 = tlv8_new_with_string(17, "Hello");
    tlv8_codec_encode(codec, tlv1);
    tlv8_free(tlv1);
    dump_data(codec->data, codec->len, "codec");
    tlv8_codec_free(codec);

    codec = tlv8_codec_encoder_new();
    tlv1 = tlv8_new_with_string(32, "Hello");
    tlv8_codec_encode(codec, tlv1);
    tlv8_free(tlv1);
    tlv1 = tlv8_new_with_string(32, "Hello");
    tlv8_codec_encode(codec, tlv1);
    tlv8_free(tlv1);
    dump_data(codec->data, codec->len, "codec");
    tlv8_codec_free(codec);

    dump_data(full_codec->data, full_codec->len, "---------------- Full codec");
    printf("Type: %0x, Len: %d, Pos: %d\n", full_codec->type, full_codec->len, full_codec->pos);

    // At that point, full_codec contains all the TLVs created so far
    while (tlv8_codec_decode_has_next(full_codec)) {
        uint8_t type = tlv8_codec_decode_peek_type(full_codec);
        TLV8_DATA_TYPE data_type = data_types[type];
        tlv8 *tlv = tlv8_codec_decode_next_tlv(full_codec, data_type);
        if (tlv) {
            switch(data_type) {
                case TLV8_DATA_TYPE_INTEGER:
                    printf("Integer, Type: %d, data_type: %d, value: %4x\n", type, data_type, tlv->data.int32);
                    break;
                case TLV8_DATA_TYPE_STRING:
                    printf("String, Type: %d, data_type: %d, value: %s\n", type, data_type, tlv->data.string);
                    break;
                case TLV8_DATA_TYPE_BYTES:
                    printf("Bytes, Type: %d, data_type: %d, value:\n", type, data_type);
                    dump_data(tlv->data.bytes, tlv->len, NULL);
                    if (type == 9) {
                        tlv8_codec *decoder = tlv8_codec_decoder_new(tlv->data.bytes, tlv->len);
                        uint8_t inner_type = tlv8_codec_decode_peek_type(decoder);
                        TLV8_DATA_TYPE inner_data_type = data_types[inner_type];
                        tlv8 *inner_tlv = tlv8_codec_decode_next_tlv(decoder, inner_data_type);
                        printf("Inner string, Type: %d, value: %s\n", inner_type, inner_tlv->data.string);
                        tlv8_free(inner_tlv);
                        tlv8_codec_free(decoder);
                    }
                    break;
                default:
                    break;
            }
        }
        tlv8_free(tlv);
    }
    tlv8_codec_free(full_codec);
}
