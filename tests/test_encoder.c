/**
 * @file    test_encoder.c
 * @brief   编解码模块单元测试 / Unit tests for encoder/decoder module
 *
 * 覆盖 XOR 单键/多键加解密、Base64 编解码、边界条件等。
 * Covers XOR single/multi-key encryption, Base64 encoding,
 * and edge cases.
 */

#include "scloader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Windows 下强制 UTF-8 编码，防止中文乱码 */
#ifdef _WIN32
#include <windows.h>
#define INIT_CONSOLE() SetConsoleOutputCP(CP_UTF8)
#else
#define INIT_CONSOLE()
#endif

/* 测试结果计数器 / Test result counters */
static int tests_passed = 0;
static int tests_failed = 0;

/**
 * @brief 辅助函数：打印并通过/失败计数 / Helper: print pass/fail counts
 */
static void test_result(const char *name, int passed)
{
    if (passed) {
        printf("  [✓] %s\n", name);
        tests_passed++;
    } else {
        printf("  [✗] %s\n", name);
        tests_failed++;
    }
}

/**
 * @brief 测试 XOR 单键编码和解码的往返一致性
 *        Test XOR single-key encode/decode round-trip consistency
 */
static void test_xor_single_key_roundtrip(void)
{
    unsigned char plaintext[] = "Hello, Shellcode World!";
    unsigned char key[] = { 0xAB };
    unsigned char *encoded = NULL;
    unsigned char *decoded = NULL;
    size_t enc_size = 0, dec_size = 0;
    int ok = 1;

    /* 编码 / Encode */
    scl_status_t s = scl_encode(SCL_ENC_XOR_KEY, plaintext, sizeof(plaintext),
                                 key, sizeof(key), &encoded, &enc_size);
    if (s != SCL_OK || encoded == NULL || enc_size != sizeof(plaintext))
        ok = 0;

    /* 解码 / Decode */
    s = scl_decode(SCL_ENC_XOR_KEY, encoded, enc_size,
                    key, sizeof(key), &decoded, &dec_size);
    if (s != SCL_OK || decoded == NULL || dec_size != sizeof(plaintext))
        ok = 0;

    /* 验证内容一致 / Verify content matches */
    if (memcmp(plaintext, decoded, sizeof(plaintext)) != 0)
        ok = 0;

    free(encoded);
    free(decoded);
    test_result("XOR 单键往返 / XOR single-key round-trip", ok);
}

/**
 * @brief 测试 XOR 多键编码的往返一致性
 *        Test XOR multi-key encode/decode round-trip consistency
 */
static void test_xor_multi_key_roundtrip(void)
{
    unsigned char plaintext[] = "Multi-key XOR Test Data: 你好世界!";
    unsigned char key[] = { 0xAB, 0xCD, 0xEF, 0x12, 0x34 };
    unsigned char *encoded = NULL;
    unsigned char *decoded = NULL;
    size_t enc_size = 0, dec_size = 0;
    int ok = 1;

    scl_status_t s = scl_encode(SCL_ENC_XOR_KEY, plaintext, sizeof(plaintext),
                                 key, sizeof(key), &encoded, &enc_size);
    if (s != SCL_OK || enc_size != sizeof(plaintext)) ok = 0;

    s = scl_decode(SCL_ENC_XOR_KEY, encoded, enc_size,
                    key, sizeof(key), &decoded, &dec_size);
    if (s != SCL_OK || dec_size != sizeof(plaintext)) ok = 0;

    if (memcmp(plaintext, decoded, sizeof(plaintext)) != 0) ok = 0;

    free(encoded);
    free(decoded);
    test_result("XOR 多键往返 / XOR multi-key round-trip", ok);
}

/**
 * @brief 测试 XOR 默认密钥行为 / Test XOR default key fallback
 */
static void test_xor_default_key(void)
{
    unsigned char plaintext[] = { 0x41, 0x42, 0x43, 0x44 };
    unsigned char *encoded = NULL, *decoded = NULL;
    size_t enc_size = 0, dec_size = 0;
    int ok = 1;

    /* 传入 NULL key，应使用默认密钥 / NULL key should use default */
    scl_status_t s = scl_encode(SCL_ENC_XOR_KEY, plaintext, sizeof(plaintext),
                                 NULL, 0, &encoded, &enc_size);
    if (s != SCL_OK || enc_size != sizeof(plaintext)) ok = 0;

    s = scl_decode(SCL_ENC_XOR_KEY, encoded, enc_size,
                    NULL, 0, &decoded, &dec_size);
    if (s != SCL_OK || dec_size != sizeof(plaintext)) ok = 0;

    if (memcmp(plaintext, decoded, sizeof(plaintext)) != 0) ok = 0;

    free(encoded);
    free(decoded);
    test_result("XOR 默认密钥 / XOR default key", ok);
}

/**
 * @brief 测试 Base64 编解码往返 / Test Base64 encode/decode round-trip
 */
static void test_base64_roundtrip(void)
{
    unsigned char data[] = "Base64 encoding test with binary data: \x01\x02\xFF\xFE";
    unsigned char *encoded = NULL, *decoded = NULL;
    size_t enc_size = 0, dec_size = 0;
    int ok = 1;

    scl_status_t s = scl_encode(SCL_ENC_BASE64, data, sizeof(data),
                                 NULL, 0, &encoded, &enc_size);
    if (s != SCL_OK || encoded == NULL || enc_size == 0) ok = 0;

    s = scl_decode(SCL_ENC_BASE64, encoded, enc_size,
                    NULL, 0, &decoded, &dec_size);
    if (s != SCL_OK || decoded == NULL || dec_size != sizeof(data)) ok = 0;

    if (memcmp(data, decoded, sizeof(data)) != 0) ok = 0;

    free(encoded);
    free(decoded);
    test_result("Base64 往返 / Base64 round-trip", ok);
}

/**
 * @brief 测试空输入边界条件 / Test empty input edge case
 */
static void test_empty_input(void)
{
    unsigned char *encoded = NULL, *decoded = NULL;
    size_t enc_size = 0, dec_size = 0;
    int ok = 1;

    /* 空数据编码 / Encode empty data */
    scl_status_t s = scl_encode(SCL_ENC_XOR_KEY, NULL, 0,
                                 NULL, 0, &encoded, &enc_size);
    if (s != SCL_ERR_INVALID_ARG) ok = 0;

    /* 空数据解码 / Decode empty data */
    s = scl_decode(SCL_ENC_XOR_KEY, NULL, 0,
                    NULL, 0, &decoded, &dec_size);
    if (s != SCL_ERR_INVALID_ARG) ok = 0;

    test_result("空输入处理 / Empty input handling", ok);
}

/**
 * @brief 测试 NULL 输出指针 / Test NULL output pointer
 */
static void test_null_output(void)
{
    unsigned char data[] = { 0xCC, 0xC3 };
    int ok = 1;

    scl_status_t s = scl_encode(SCL_ENC_XOR_KEY, data, sizeof(data),
                                 NULL, 0, NULL, NULL);
    if (s != SCL_ERR_INVALID_ARG) ok = 0;

    s = scl_decode(SCL_ENC_XOR_KEY, data, sizeof(data),
                    NULL, 0, NULL, NULL);
    if (s != SCL_ERR_INVALID_ARG) ok = 0;

    test_result("NULL 输出指针 / NULL output pointer", ok);
}

/**
 * @brief 测试无效的编码类型 / Test invalid encoder type
 */
static void test_invalid_type(void)
{
    unsigned char data[] = { 0xCC };
    unsigned char *out = NULL;
    size_t out_size = 0;
    int ok = 1;

    scl_status_t s = scl_encode(999, data, sizeof(data),
                                 NULL, 0, &out, &out_size);
    if (s != SCL_ERR_INVALID_ARG) ok = 0;

    s = scl_decode(999, data, sizeof(data),
                    NULL, 0, &out, &out_size);
    if (s != SCL_ERR_INVALID_ARG) ok = 0;

    test_result("无效编码类型 / Invalid encoder type", ok);
}

int main(void)
{
    INIT_CONSOLE();
    printf("╔══════════════════════════════════════════╗\n");
    printf("║   编解码单元测试 / Encoder Unit Tests     ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");

    test_xor_single_key_roundtrip();
    test_xor_multi_key_roundtrip();
    test_xor_default_key();
    test_base64_roundtrip();
    test_empty_input();
    test_null_output();
    test_invalid_type();

    printf("\n═══════════════════════════════════════════\n");
    printf("  合计 / Total: %d  通过 / Passed: %d  失败 / Failed: %d\n",
           tests_passed + tests_failed, tests_passed, tests_failed);
    printf("═══════════════════════════════════════════\n");

    return tests_failed > 0 ? 1 : 0;
}