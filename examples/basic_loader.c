/**
 * @file    basic_loader.c
 * @brief   示例1: 基础 Shellcode 加载器 / Example 1: Basic Shellcode Loader
 *
 * 演示最简单的用例：加载并执行一个最小 shellcode payload。
 * Demonstrates the simplest use case: loading and executing
 * a minimal shellcode payload.
 *
 * ⚠ 警告 / WARNING: 仅供教育用途 / For educational purposes only.
 *   仅在你有所有权的系统上执行你信任的 shellcode。
 *   Only execute shellcode you trust on systems you own.
 */

#include "scloader.h"
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#define INIT_CONSOLE() SetConsoleOutputCP(CP_UTF8)
#else
#define INIT_CONSOLE()
#endif

int main(void)
{
    INIT_CONSOLE();
    printf("═══ Shellcode Executor - 基础示例 / Basic Example ═══\n\n");

    /* 最小 shellcode: INT3 (断点) + RET (返回) */
    /* Minimal shellcode: INT3 (breakpoint) + RET */
    unsigned char shellcode[] = {
        0xCC,  /* INT3 - 触发调试断点 / Trigger debug breakpoint */
        0xC3   /* RET  - 返回到调用者 / Return to caller */
    };

    printf("[*] Shellcode 字节 / bytes: ");
    for (size_t i = 0; i < sizeof(shellcode); i++) {
        printf("%02X ", shellcode[i]);
    }
    printf("\n");
    printf("[*] 大小 / Size: %zu bytes\n", sizeof(shellcode));

    /* 使用默认配置 / Use default configuration */
    scl_loader_config_t config = SCL_LOADER_CONFIG_DEFAULT;
    config.clear_after = false; /* 保留内存以便调试 / Keep memory for debugging */

    printf("[*] 正在执行 shellcode / Executing shellcode...\n\n");

    scl_status_t status = scl_load_and_exec(shellcode, sizeof(shellcode), &config);

    if (status == SCL_OK) {
        printf("\n[✓] Shellcode 执行成功 / Executed successfully!\n");
    } else {
        printf("\n[✗] 执行失败 / Execution failed, 状态码 / status: %d\n", status);
        return 1;
    }

    return 0;
}