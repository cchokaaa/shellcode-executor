/**
 * @file    encoder.c
 * @brief   Shellcode 编码器/解码器分发 / Encoder/decoder dispatch
 *
 * 根据传入的编码类型，分发给具体的编码器实现
 * Dispatches to specific encoder implementations based on encoding type
 */

#include "encoder/encoder.h"
#include <stdlib.h>
#include <string.h>

scl_status_t scl_encode(scl_enc_type_t type,
                        const unsigned char *input, size_t input_size,
                        const unsigned char *key, size_t key_size,
                        unsigned char **output, size_t *output_size)
{
    if (!input || !output || !output_size || input_size == 0) {
        return SCL_ERR_INVALID_ARG;
    }

    switch (type) {
        case SCL_ENC_NONE:
            /* 不编码，直接拷贝 / No encoding, direct copy */
            *output = (unsigned char *)malloc(input_size);
            if (!*output) return SCL_ERR_ALLOC_FAILED;
            memcpy(*output, input, input_size);
            *output_size = input_size;
            return SCL_OK;

        case SCL_ENC_XOR:
        case SCL_ENC_XOR_KEY:
            return scl_xor_encode(input, input_size, key, key_size,
                                  output, output_size);

        case SCL_ENC_BASE64:
            return scl_b64_encode(input, input_size, output, output_size);

        case SCL_ENC_HEX:
            return SCL_ERR_ENCODE_FAILED; /* 由外部处理 / Handled separately */

        default:
            return SCL_ERR_INVALID_ARG;
    }
}

scl_status_t scl_decode(scl_enc_type_t type,
                        const unsigned char *input, size_t input_size,
                        const unsigned char *key, size_t key_size,
                        unsigned char **output, size_t *output_size)
{
    if (!input || !output || !output_size || input_size == 0) {
        return SCL_ERR_INVALID_ARG;
    }

    switch (type) {
        case SCL_ENC_NONE:
            *output = (unsigned char *)malloc(input_size);
            if (!*output) return SCL_ERR_ALLOC_FAILED;
            memcpy(*output, input, input_size);
            *output_size = input_size;
            return SCL_OK;

        case SCL_ENC_XOR:
        case SCL_ENC_XOR_KEY:
            return scl_xor_decode(input, input_size, key, key_size,
                                  output, output_size);

        case SCL_ENC_BASE64:
            return scl_b64_decode(input, input_size, output, output_size);

        case SCL_ENC_HEX:
            return SCL_ERR_DECODE_FAILED;

        default:
            return SCL_ERR_INVALID_ARG;
    }
}