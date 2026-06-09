/**
 * @file    utils.c
 * @brief   Shellcode 工具函数实现 / Shellcode utility functions
 *
 * 提供 hex dump、hex 字符串转换等调试辅助功能
 * Provides hex dump, hex string conversion, and other debug helpers
 */

#include "utils/utils.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

void scl_hex_dump(const unsigned char *data, size_t size, const char *title)
{
    if (!data) return;

    if (title) {
        printf("═══════════════════════════════════════════════\n");
        printf("  %s\n", title);
        printf("═══════════════════════════════════════════════\n");
    }

    /* 打印列标题 / Print column headers */
    printf("  Offset    00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F  ASCII\n");
    printf("  ─────────────────────────────────────────────────────────────────\n");

    /* 每行显示 16 字节 / Display 16 bytes per line */
    for (size_t i = 0; i < size; i += 16) {
        printf("  %06zX  ", i);

        char ascii[17] = {0};
        for (size_t j = 0; j < 16; j++) {
            if (i + j < size) {
                printf("%02X ", data[i + j]);
                /* 可打印字符显示原字符，否则显示 '.' */
                ascii[j] = (data[i + j] >= 32 && data[i + j] <= 126)
                           ? (char)data[i + j] : '.';
            } else {
                printf("   ");
                ascii[j] = ' ';
            }
            if (j == 7) printf(" ");
        }
        printf(" %s\n", ascii);
    }
    printf("\n");
}

scl_status_t scl_hex_to_bytes(const char *hex, unsigned char *out,
                               size_t out_size, size_t *written)
{
    if (!hex || !out || !written) {
        return SCL_ERR_INVALID_ARG;
    }

    size_t hex_len = strlen(hex);
    /* 去除空格 / Remove whitespace */
    size_t clean_len = 0;
    for (size_t i = 0; i < hex_len; i++) {
        if (!isspace((unsigned char)hex[i])) clean_len++;
    }

    /* 十六进制字符串长度必须为偶数 / Hex length must be even */
    if (clean_len % 2 != 0) {
        return SCL_ERR_INVALID_ARG;
    }

    size_t byte_len = clean_len / 2;
    if (byte_len > out_size) {
        return SCL_ERR_BUFFER_TOO_SMALL;
    }

    size_t out_idx = 0;
    size_t hex_idx = 0;
    while (hex_idx < hex_len) {
        /* 跳过空格 / Skip whitespace */
        if (isspace((unsigned char)hex[hex_idx])) {
            hex_idx++;
            continue;
        }

        char high = (char)toupper((unsigned char)hex[hex_idx]);
        char low  = (char)toupper((unsigned char)hex[hex_idx + 1]);

        unsigned char h, l;
        if (high >= '0' && high <= '9')
            h = (unsigned char)(high - '0');
        else if (high >= 'A' && high <= 'F')
            h = (unsigned char)(high - 'A' + 10);
        else
            return SCL_ERR_INVALID_ARG;

        if (low >= '0' && low <= '9')
            l = (unsigned char)(low - '0');
        else if (low >= 'A' && low <= 'F')
            l = (unsigned char)(low - 'A' + 10);
        else
            return SCL_ERR_INVALID_ARG;

        out[out_idx++] = (unsigned char)((h << 4) | l);
        hex_idx += 2;
    }

    *written = out_idx;
    return SCL_OK;
}

scl_status_t scl_bytes_to_hex(const unsigned char *data, size_t data_size,
                               char *out, size_t out_size)
{
    if (!data || !out) {
        return SCL_ERR_INVALID_ARG;
    }

    /* 确保缓冲区足够大 / Ensure buffer is large enough */
    if (out_size < data_size * 2 + 1) {
        return SCL_ERR_BUFFER_TOO_SMALL;
    }

    static const char hex_chars[] = "0123456789ABCDEF";
    for (size_t i = 0; i < data_size; i++) {
        out[i * 2]     = hex_chars[(data[i] >> 4) & 0x0F];
        out[i * 2 + 1] = hex_chars[data[i] & 0x0F];
    }
    out[data_size * 2] = '\0';

    return SCL_OK;
}