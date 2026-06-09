/**
 * @file    test_utils.c
 * @brief   工具函数模块单元测试 / Unit tests for utility functions
 *
 * 覆盖 Hex 编解码、边界条件等。
 * Covers hex encoding/decoding and edge cases.
 */

#include "scloader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#define INIT_CONSOLE() SetConsoleOutputCP(CP_UTF8)
#else
#define INIT_CONSOLE()
#endif
#include <ctype.h>

static int tests_passed = 0;
static int tests_failed = 0;

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
 * @brief 测试十六进制字符串转字节数组 / Test hex string to bytes
 */
static void test_hex_to_bytes_standard(void)
{
    unsigned char buf[16];
    size_t written = 0;
    int ok = 1;

    scl_status_t s = scl_hex_to_bytes("CCAB90", buf, sizeof(buf), &written);
    if (s != SCL_OK) ok = 0;
    if (written != 3) ok = 0;
    if (buf[0] != 0xCC || buf[1] != 0xAB || buf[2] != 0x90) ok = 0;

    test_result("Hex→Bytes 标准 / Hex→Bytes standard", ok);
}

/**
 * @brief 测试带空格的十六进制字符串 / Test hex string with spaces
 */
static void test_hex_to_bytes_spaces(void)
{
    unsigned char buf[16];
    size_t written = 0;
    int ok = 1;

    scl_status_t s = scl_hex_to_bytes("CC AB 90 FF", buf, sizeof(buf), &written);
    if (s != SCL_OK) ok = 0;
    if (written != 4) ok = 0;
    if (buf[0] != 0xCC || buf[1] != 0xAB || buf[3] != 0xFF) ok = 0;

    test_result("Hex→Bytes 含空格 / Hex→Bytes with spaces", ok);
}

/**
 * @brief 测试非法十六进制字符 / Test invalid hex characters
 */
static void test_hex_to_bytes_invalid(void)
{
    unsigned char buf[16];
    size_t written = 0;

    scl_status_t s = scl_hex_to_bytes("XYZ", buf, sizeof(buf), &written);
    test_result("Hex→Bytes 非法字符 / Hex→Bytes invalid chars",
                s == SCL_ERR_INVALID_ARG);
}

/**
 * @brief 测试奇数长度 / Test odd-length hex string
 */
static void test_hex_to_bytes_odd(void)
{
    unsigned char buf[16];
    size_t written = 0;

    scl_status_t s = scl_hex_to_bytes("CCA", buf, sizeof(buf), &written);
    test_result("Hex→Bytes 奇数长度 / Hex→Bytes odd length",
                s == SCL_ERR_INVALID_ARG);
}

/**
 * @brief 测试字节数组转十六进制字符串 / Test bytes to hex string
 */
static void test_bytes_to_hex(void)
{
    unsigned char data[] = { 0xCC, 0xAB, 0x90, 0x00, 0xFF };
    char buf[32];
    int ok = 1;

    scl_status_t s = scl_bytes_to_hex(data, sizeof(data), buf, sizeof(buf));
    if (s != SCL_OK) ok = 0;

    /* 将预期和实际都转为大写再比较 / Uppercase for comparison */
    for (size_t i = 0; buf[i]; i++) buf[i] = (char)toupper((unsigned char)buf[i]);
    if (strcmp(buf, "CCAB9000FF") != 0) ok = 0;

    test_result("Bytes→Hex 标准 / Bytes→Hex standard", ok);
}

/**
 * @brief 测试过小的缓冲区 / Test buffer too small
 */
static void test_bytes_to_hex_small_buf(void)
{
    unsigned char data[] = { 0xCC, 0xAB };
    char buf[4]; /* 需要5字节(4 hex + NUL), 4不够 / Needs 5, buf=4 too small */

    scl_status_t s = scl_bytes_to_hex(data, sizeof(data), buf, sizeof(buf));
    test_result("Bytes→Hex 缓冲区太小 / Bytes→Hex buffer too small",
                s == SCL_ERR_BUFFER_TOO_SMALL);
}

/**
 * @brief 测试 NULL 输入 / Test NULL input
 */
static void test_null_input(void)
{
    unsigned char buf[16];
    size_t written = 0;
    int ok = 1;

    if (scl_hex_to_bytes(NULL, buf, sizeof(buf), &written) != SCL_ERR_INVALID_ARG) ok = 0;
    if (scl_bytes_to_hex(NULL, 0, (char *)buf, sizeof(buf)) != SCL_ERR_INVALID_ARG) ok = 0;

    test_result("NULL 输入 / NULL input", ok);
}

int main(void)
{
    INIT_CONSOLE();
    printf("╔══════════════════════════════════════════╗\n");
    printf("║   工具函数单元测试 / Utility Unit Tests   ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");

    test_hex_to_bytes_standard();
    test_hex_to_bytes_spaces();
    test_hex_to_bytes_invalid();
    test_hex_to_bytes_odd();
    test_bytes_to_hex();
    test_bytes_to_hex_small_buf();
    test_null_input();

    printf("\n═══════════════════════════════════════════\n");
    printf("  合计 / Total: %d  通过 / Passed: %d  失败 / Failed: %d\n",
           tests_passed + tests_failed, tests_passed, tests_failed);
    printf("═══════════════════════════════════════════\n");

    return tests_failed > 0 ? 1 : 0;
}