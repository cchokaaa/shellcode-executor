/**
 * @file    xor.c
 * @brief   XOR Shellcode 编码器/解码器 / XOR Shellcode Encoder/Decoder
 *
 * 支持两种模式 / Supports two modes:
 *   - 单字节 XOR 密钥 / Single-byte XOR key (SCL_ENC_XOR)
 *   - 多字节 XOR 密钥 / Multi-byte XOR key (SCL_ENC_XOR_KEY)
 *
 * XOR 是对称加密：encode == decode
 * XOR is symmetric: encode and decode are identical operations
 */

#include "encoder/encoder.h"
#include <stdlib.h>
#include <string.h>

scl_status_t scl_xor_encode(const unsigned char *input, size_t input_size,
                            const unsigned char *key, size_t key_size,
                            unsigned char **output, size_t *output_size)
{
    if (!input || !output || !output_size || input_size == 0) {
        return SCL_ERR_INVALID_ARG;
    }

    /* 如果未提供密钥，默认使用单字节 0xFF */
    /* Default to single-byte key 0xFF if no key provided */
    unsigned char default_key[] = { 0xFF };
    if (!key || key_size == 0) {
        key = default_key;
        key_size = 1;
    }

    unsigned char *result = (unsigned char *)malloc(input_size);
    if (!result) {
        return SCL_ERR_ALLOC_FAILED;
    }

    /* 逐字节 XOR，密钥循环使用 / XOR each byte with cycling key */
    for (size_t i = 0; i < input_size; i++) {
        result[i] = input[i] ^ key[i % key_size];
    }

    *output = result;
    *output_size = input_size;
    return SCL_OK;
}

scl_status_t scl_xor_decode(const unsigned char *input, size_t input_size,
                            const unsigned char *key, size_t key_size,
                            unsigned char **output, size_t *output_size)
{
    /* XOR 是对称操作，加解密相同 */
    /* XOR is symmetric: encrypt and decrypt are identical */
    return scl_xor_encode(input, input_size, key, key_size, output, output_size);
}