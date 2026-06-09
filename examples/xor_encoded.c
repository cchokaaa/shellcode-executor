/**
 * @file    xor_encoded.c
 * @brief   示例2: XOR 编码的 Shellcode 加载器 / Example 2: XOR-Encoded Loader
 *
 * 演示加载经 XOR 加密的 shellcode，以绕过静态特征检测。
 * Demonstrates loading XOR-encrypted shellcode to evade
 * static signature-based detection.
 *
 * 流程 / Flow: 磁盘上的编码数据 → 内存解码 → 执行
 *               Encoded on disk → Decode in memory → Execute
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

int main(void)
{
    INIT_CONSOLE();
    printf("═══ Shellcode Executor - XOR 编码示例 / XOR Encoded Example ═══\n\n");

    /* 原始 shellcode: INT3 + RET */
    unsigned char original[] = { 0xCC, 0xC3 };

    /* XOR 密钥 / XOR key */
    unsigned char key[] = { 0xAB, 0xCD };

    /* 编码 shellcode / Encode the shellcode */
    unsigned char *encoded = NULL;
    size_t encoded_size = 0;

    scl_status_t status = scl_encode(SCL_ENC_XOR_KEY, original, sizeof(original),
                                      key, sizeof(key),
                                      &encoded, &encoded_size);
    if (status != SCL_OK) {
        fprintf(stderr, "[!] 编码失败 / Encoding failed\n");
        return 1;
    }

    printf("[*] 原始数据 / Original: ");
    for (size_t i = 0; i < sizeof(original); i++) printf("%02X ", original[i]);
    printf("\n");

    printf("[*] 编码后 / Encoded:  ");
    for (size_t i = 0; i < encoded_size; i++) printf("%02X ", encoded[i]);
    printf("\n");

    printf("[*] 密钥 / Key:      0x%02X 0x%02X\n", key[0], key[1]);

    /* 在内存中解码 / Decode in memory */
    unsigned char *decoded = NULL;
    size_t decoded_size = 0;

    status = scl_decode(SCL_ENC_XOR_KEY, encoded, encoded_size,
                         key, sizeof(key),
                         &decoded, &decoded_size);
    if (status != SCL_OK) {
        fprintf(stderr, "[!] 解码失败 / Decoding failed\n");
        free(encoded);
        return 1;
    }

    printf("[*] 解码后 / Decoded:  ");
    for (size_t i = 0; i < decoded_size; i++) printf("%02X ", decoded[i]);
    printf("\n\n");

    /* 执行解码后的 shellcode / Execute the decoded shellcode */
    scl_loader_config_t config = SCL_LOADER_CONFIG_DEFAULT;
    config.clear_after = false;

    printf("[*] 正在执行解码后的 shellcode / Executing decoded shellcode...\n\n");
    status = scl_load_and_exec(decoded, decoded_size, &config);

    if (status == SCL_OK) {
        printf("\n[✓] XOR 编码 shellcode 执行成功 / Executed successfully!\n");
    } else {
        printf("\n[✗] 执行失败 / Execution failed, 状态码 / status: %d\n", status);
    }

    free(encoded);
    free(decoded);
    return (status == SCL_OK) ? 0 : 1;
}