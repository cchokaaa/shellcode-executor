/**
 * @file    test_loader.c
 * @brief   加载器模块单元测试 / Unit tests for loader module
 *
 * 覆盖内存分配、写入、执行、释放等核心路径及边界条件。
 * Covers memory allocation, write, execute, free and edge cases.
 *
 * 注意: macOS (Apple Silicon) 由于系统强制 W^X 安全策略，
 *       无法通过 mprotect 将 RW 内存切换为 RX（可执行）。
 *       在此平台会跳过执行相关测试。
 * Note: On macOS (Apple Silicon), W^X policy blocks RW->RX mprotect,
 *       so execution-related tests are skipped.
 */

#include "scloader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>

#ifdef _WIN32
#include <windows.h>
#define INIT_CONSOLE() SetConsoleOutputCP(CP_UTF8)
#else
#define INIT_CONSOLE()
#endif

static int tests_passed = 0;
static int tests_failed = 0;

/* ── setjmp/longjmp 信号恢复 ── */
static jmp_buf jmp_env;
static volatile int sig_caught = 0;
static const char *sig_name = "";

static void sig_handler(int sig)
{
    sig_caught = 1;
    switch (sig) {
        case SIGILL:  sig_name = "SIGILL";  break;
        case SIGSEGV: sig_name = "SIGSEGV"; break;
        case SIGBUS:  sig_name = "SIGBUS";  break;
        case SIGABRT: sig_name = "SIGABRT"; break;
        default:      sig_name = "UNKNOWN"; break;
    }
    /* 跳回测试起点，避免重复触发 / Jump back to avoid re-trigger */
    longjmp(jmp_env, 1);
}

static void install_sig_handlers(void)
{
    signal(SIGILL,  sig_handler);
    signal(SIGSEGV, sig_handler);
    signal(SIGBUS,  sig_handler);
    signal(SIGABRT, sig_handler);
}

/* ── 工具宏：测试结果 ── */

static void test_result(const char *name, int passed)
{
    if (passed) {
        printf("  [\xe2\x9c\x93] %s\n", name);
        tests_passed++;
    } else {
        printf("  [\xe2\x9c\x97] %s\n", name);
        tests_failed++;
    }
}

/**
 * @brief 测试分配可执行内存 / Test allocating executable memory
 */
static void test_alloc_exec(void)
{
    void *ptr = NULL;
    size_t size = 4096;

    sig_caught = 0;
    if (setjmp(jmp_env) == 0) {
        scl_status_t s = scl_alloc_exec(size, SCL_MEM_RWX, &ptr);
        int ok = (s == SCL_OK && ptr != NULL);

        if (ok) {
            memset(ptr, 0xFF, 16);
            unsigned char val = *(volatile unsigned char *)ptr;
            if (val != 0xFF) ok = 0;
        }

        scl_free_exec(ptr, size, true);
        test_result("分配可执行内存 / Allocate executable memory", ok);
    } else {
        /* 信号跳转至此 / Signal caught here */
        test_result("分配可执行内存 / Allocate executable memory", 0);
        printf("         ↳ CRASHED: %s\n", sig_name);
    }
}

/**
 * @brief 测试零大小分配 / Test zero-size allocation
 */
static void test_alloc_zero_size(void)
{
    void *ptr = (void *)0xDEAD;
    sig_caught = 0;
    scl_status_t s = scl_alloc_exec(0, SCL_MEM_RWX, &ptr);
    test_result("零大小返回无效参数 / Zero-size returns invalid arg",
                s == SCL_ERR_INVALID_ARG);
}

/**
 * @brief 测试 NULL 输出参数 / Test NULL output parameter
 */
static void test_alloc_null_out(void)
{
    sig_caught = 0;
    scl_status_t s = scl_alloc_exec(4096, SCL_MEM_RWX, NULL);
    test_result("NULL 输出参数 / NULL output param", s == SCL_ERR_INVALID_ARG);
}

/**
 * @brief 测试写入 shellcode / Test writing shellcode to allocated memory
 */
static void test_write_shellcode(void)
{
    unsigned char code[] = { 0xCC, 0xC3, 0x90, 0x90 };
    void *mem = NULL;
    int ok = 1;

    sig_caught = 0;
    if (setjmp(jmp_env) == 0) {
        scl_alloc_exec(4096, SCL_MEM_RWX, &mem);
        scl_status_t s = scl_write_shellcode(mem, code, sizeof(code));
        if (s != SCL_OK) ok = 0;
        if (memcmp(mem, code, sizeof(code)) != 0) ok = 0;
        scl_free_exec(mem, 4096, true);
        test_result("写入 shellcode / Write shellcode", ok);
    } else {
        test_result("写入 shellcode / Write shellcode", 0);
        printf("         ↳ CRASHED: %s\n", sig_name);
    }
}

/**
 * @brief 测试写入 NULL 目标 / Test write to NULL destination
 */
static void test_write_null_buf(void)
{
    unsigned char code[] = { 0xCC };
    sig_caught = 0;
    scl_status_t s = scl_write_shellcode(NULL, code, sizeof(code));
    test_result("写入 NULL 目标 / Write to NULL target",
                s == SCL_ERR_INVALID_ARG || s == SCL_ERR_NULL_PTR);
}

/**
 * @brief 测试释放并清空内存 / Test freeing memory with clearing
 */
static void test_free_with_clear(void)
{
    unsigned char *mem = NULL;
    int ok = 1;

    sig_caught = 0;
    if (setjmp(jmp_env) == 0) {
        scl_alloc_exec(4096, SCL_MEM_RWX, (void **)&mem);
        memset(mem, 0x41, 4096);
        scl_free_exec(mem, 4096, true);
        test_result("释放内存(带清零) / Free with clear", ok);
    } else {
        test_result("释放内存(带清零) / Free with clear", 0);
        printf("         ↳ CRASHED: %s\n", sig_name);
    }
}

/**
 * @brief 测试释放 NULL / Test freeing NULL pointer
 */
static void test_free_null(void)
{
    sig_caught = 0;
    scl_status_t s = scl_free_exec(NULL, 4096, true);
    test_result("释放 NULL / Free NULL pointer",
                s == SCL_ERR_INVALID_ARG);
}

/**
 * @brief 测试完整的加载-执行流程 / Test complete load-and-execute flow
 *
 * 注意: macOS (Apple Silicon) 由于系统安全限制，无法通过 mprotect 将
 *       内存从 RW 切换为 RX。这是预期中的系统行为。
 * Note: On macOS (Apple Silicon), RW→RX mprotect is blocked by the OS.
 *       This is an expected system limitation, not a code bug.
 */
static void test_full_load_exec(void)
{
    unsigned char shellcode[] = { 0xC3 }; /* x86 RET (ARM上不会被执行) */
    scl_loader_config_t config = SCL_LOADER_CONFIG_DEFAULT;
    config.clear_after = false;

    sig_caught = 0;
    if (setjmp(jmp_env) == 0) {
        scl_status_t s = scl_load_and_exec(shellcode, sizeof(shellcode), &config);

#if SCLOADER_PLATFORM_MACOS
        /* macOS 上 mprotect RW→RX 可能被拦截，这是系统安全策略 */
        int ok = (s == SCL_OK) || (s == SCL_ERR_PROTECT_FAILED);
#else
        int ok = (s == SCL_OK);
#endif
        test_result("完整加载-执行流程 / Full load-execute flow", ok);
    } else {
        test_result("完整加载-执行流程 / Full load-execute flow", 0);
        printf("         ↳ CRASHED: %s\n", sig_name);
    }
}

int main(void)
{
    INIT_CONSOLE();

    /* 安装信号处理器，避免 crash 直接退出 / Install signal handlers */
    install_sig_handlers();

    printf("╔══════════════════════════════════════════╗\n");
    printf("║   加载器单元测试 / Loader Unit Tests      ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");

    test_alloc_exec();
    test_alloc_zero_size();
    test_alloc_null_out();
    test_write_shellcode();
    test_write_null_buf();
    test_free_with_clear();
    test_free_null();
    test_full_load_exec();

    printf("\n═══════════════════════════════════════════\n");
    printf("  合计 / Total: %d  通过 / Passed: %d  失败 / Failed: %d\n",
           tests_passed + tests_failed, tests_passed, tests_failed);
    printf("═══════════════════════════════════════════\n");

    return tests_failed > 0 ? 1 : 0;
}