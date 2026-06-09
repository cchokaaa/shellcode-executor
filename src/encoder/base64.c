/**
 * @file    base64.c
 * @brief   Base64 Shellcode 编码器/解码器 / Base64 Shellcode Encoder/Decoder
 *
 * 实现 RFC 4648 Base64 编码，用于通过文本协议安全传输 shellcode
 * Implements RFC 4648 Base64 for safe shellcode transport via text protocols
 *
 * 适用于 HTTP header、JSON 等文本通道
 * Suitable for text channels like HTTP headers, JSON, etc.
 */

#include "encoder/encoder.h"
#include <stdlib.h>
#include <string.h>

/* Base64 字母表 / Base64 alphabet (RFC 4648) */
static const char b64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* Base64 反向查找表 / Base64 reverse lookup table */
static signed char b64_rev[256] = { -1 };

/**
 * 初始化反向查找表（只执行一次）
 * Initialize the reverse lookup table (one-time)
 */
static void b64_init_rev(void)
{
    static bool initialized = false;
    if (initialized) return;

    for (int i = 0; i < 256; i++) {
        b64_rev[i] = -1;
    }
    for (int i = 0; i < 64; i++) {
        b64_rev[(unsigned char)b64_table[i]] = (signed char)i;
    }
    b64_rev[(unsigned char)'='] = 0;
    initialized = true;
}

scl_status_t scl_b64_encode(const unsigned char *input, size_t input_size,
                            unsigned char **output, size_t *output_size)
{
    if (!input || !output || !output_size || input_size == 0) {
        return SCL_ERR_INVALID_ARG;
    }

    /* 计算带填充的输出大小 / Calculate output size with padding */
    size_t out_len = 4 * ((input_size + 2) / 3);
    char *result = (char *)malloc(out_len + 1);
    if (!result) {
        return SCL_ERR_ALLOC_FAILED;
    }

    size_t i, j;
    for (i = 0, j = 0; i < input_size;) {
        /* 每次处理 3 字节 / Process 3 bytes at a time */
        uint32_t octet_a = (i < input_size) ? input[i++] : 0;
        uint32_t octet_b = (i < input_size) ? input[i++] : 0;
        uint32_t octet_c = (i < input_size) ? input[i++] : 0;

        uint32_t triple = (octet_a << 16) | (octet_b << 8) | octet_c;

        /* 编码为 4 个 Base64 字符 / Encode to 4 Base64 chars */
        result[j++] = b64_table[(triple >> 18) & 0x3F];
        result[j++] = b64_table[(triple >> 12) & 0x3F];
        result[j++] = b64_table[(triple >> 6) & 0x3F];
        result[j++] = b64_table[triple & 0x3F];
    }

    /* 添加 '=' 填充 / Apply '=' padding */
    size_t padding = (3 - (input_size % 3)) % 3;
    for (size_t p = 0; p < padding; p++) {
        result[out_len - 1 - p] = '=';
    }
    result[out_len] = '\0';

    *output = (unsigned char *)result;
    *output_size = out_len;
    return SCL_OK;
}

scl_status_t scl_b64_decode(const unsigned char *input, size_t input_size,
                            unsigned char **output, size_t *output_size)
{
    if (!input || !output || !output_size || input_size == 0) {
        return SCL_ERR_INVALID_ARG;
    }

    /* Base64 输入长度必须是 4 的倍数 */
    /* Input must be valid base64 length (multiple of 4) */
    if (input_size % 4 != 0) {
        return SCL_ERR_INVALID_ARG;
    }

    b64_init_rev();

    /* 计算输出大小（去掉填充） / Calculate output size (remove padding) */
    size_t padding = 0;
    if (input_size >= 2 && input[input_size - 1] == '=') padding++;
    if (input_size >= 2 && input[input_size - 2] == '=') padding++;

    size_t out_len = (input_size / 4) * 3 - padding;
    unsigned char *result = (unsigned char *)malloc(out_len);
    if (!result) {
        return SCL_ERR_ALLOC_FAILED;
    }

    size_t j = 0;
    for (size_t i = 0; i < input_size; i += 4) {
        /* 将 4 个 Base64 字符解码为 3 字节 */
        /* Decode 4 Base64 chars back to 3 bytes */
        signed char a = b64_rev[(unsigned char)input[i]];
        signed char b = b64_rev[(unsigned char)input[i + 1]];
        signed char c = b64_rev[(unsigned char)input[i + 2]];
        signed char d = b64_rev[(unsigned char)input[i + 3]];

        if (a == -1 || b == -1 || c == -1 || d == -1) {
            free(result);
            return SCL_ERR_DECODE_FAILED;
        }

        uint32_t triple = ((uint32_t)a << 18) |
                          ((uint32_t)b << 12) |
                          ((uint32_t)c << 6)  |
                          (uint32_t)d;

        if (j < out_len) result[j++] = (unsigned char)(triple >> 16);
        if (j < out_len) result[j++] = (unsigned char)(triple >> 8);
        if (j < out_len) result[j++] = (unsigned char)triple;
    }

    *output = result;
    *output_size = out_len;
    return SCL_OK;
}